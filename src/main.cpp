#ifndef WIN32
#define GLEW_STATIC
#endif

#include <iostream>
#include <memory>

#ifdef WIN32
#include <corecrt_math_defines.h>
#endif
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "renderer/shader.h"
#include "renderer/wavefront.h"
#include "renderer/engine.h"

#include "IO/file_manager.h"

class Game : public IApplication {
public:
    Game() {}
    ~Game() override {}

    void OnInit() override {
        std::cout << "Game initialized" << std::endl;

        m_proj = glm::perspective(
            static_cast<float>(M_PI_2),
            static_cast<float>(Window::GetWidth()) / static_cast<float>(Window::GetHeight()),
            0.01f,
            100.0f
        );

        m_shader.init(
            FileManager::read("./src/shaders/simple.vs"),
            FileManager::read("./src/shaders/simple.fs")
        );

        m_camPos = glm::vec3(0.f, 0.f, 2.f);
        // glm::vec3 cameraViewDirection(0.f, 0.f, -1.f);
        // glm::vec3 lightPosition(1.f, 3.5f, -2.f);
        m_lightPos = glm::vec3(-5.f, 5.f, 5.f);

        m_model = glm::mat4(1.f);

        m_angle = 3.45f;
        m_lastTicks = SDL_GetTicks();

        // m_sun = Object::LoadFile("./assets/cube.obj");
        // m_target = Object::LoadFile("./assets/monkey.obj");

        m_sun = std::unique_ptr<Object>(Object::LoadFile("./assets/cube.obj"));
        m_target = std::unique_ptr<Object>(Object::LoadFile("./assets/monkey.obj"));

        m_paused = false;

        m_yaw   = -90.0f; // looking along -Z initially
        m_pitch = 0.0f;   // no vertical tilt

        // FPS tracking
        m_startTicks = SDL_GetTicks();
        m_frameCount = 0;
    }

    void OnWindowResized(const WindowResized& event) override {
        m_proj = glm::perspective(
            static_cast<float>(M_PI_2),
            static_cast<float>(event.w) / static_cast<float>(event.h),
            0.01f,
            100.0f
        );
    }

    void OnUpdate() override {
        m_currentTicks = SDL_GetTicks();
        float deltaTime = static_cast<float>(m_currentTicks - m_lastTicks) / 1000.0f; // seconds

        m_lastTicks = m_currentTicks;

        float mouseXRel, mouseYRel;
        SDL_GetRelativeMouseState(&mouseXRel, &mouseYRel);

        float sensitivity = 0.1f; // tweak as needed
        m_yaw   += mouseXRel * sensitivity;
        m_pitch -= mouseYRel * sensitivity; // invert Y for typical FPS control

        // clamp pitch to avoid flipping
        // if (pitch > 89.0f)  pitch = 89.0f;
        // if (pitch < -89.0f) pitch = -89.0f;
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

        // convert to direction vector
        glm::vec3 cameraViewDirection(0.f, 0.f, -1.f);
        cameraViewDirection.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        cameraViewDirection.y = sin(glm::radians(m_pitch));
        cameraViewDirection.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        cameraViewDirection = glm::normalize(cameraViewDirection);

        glm::vec3 velocity(0.f);

        const bool* state = SDL_GetKeyboardState(nullptr);

        if (state[SDL_SCANCODE_P]) m_paused = !m_paused;

        glm::vec3 front = glm::normalize(glm::vec3(cameraViewDirection.x, 0.f, cameraViewDirection.z));
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.f, 1.f, 0.f)));

        if (state[SDL_SCANCODE_W]) velocity += front;
        if (state[SDL_SCANCODE_S]) velocity -= front;
        if (state[SDL_SCANCODE_A]) velocity -= right;
        if (state[SDL_SCANCODE_D]) velocity += right;
        if (state[SDL_SCANCODE_SPACE]) velocity.y += 1.f;
        if (state[SDL_SCANCODE_LSHIFT]) velocity.y -= 1.f;

        m_camPos += velocity * deltaTime * 2.5f; // speed is e.g. 2.5f

        m_view = glm::lookAt(
            m_camPos,
            m_camPos + cameraViewDirection,
            glm::vec3(0.f, 1.f, 0.f)
        );

        // update rotation
        if (!m_paused) {
            m_angle += glm::radians(45.0f) * deltaTime; // 72° per second
            if (m_angle > glm::two_pi<float>()) {
                m_angle -= glm::two_pi<float>(); // keep value small
            }
        }
    }

    void OnRender() override {
        m_shader.use();

        m_shader.setMat4("u_view", m_view);
        m_shader.setMat4("u_projection", m_proj);

        m_shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setVec3("lightPos", m_lightPos);
        m_shader.setVec3("viewPos", m_camPos);

        m_model = glm::mat4(1.f);
        m_model = glm::translate(m_model, m_lightPos);

        m_shader.setMat4("u_model", m_model);

        m_sun->Render(m_shader);

        // lightPosition -= glm::vec3(0.05f, 0.f, 0.f) * deltaTime;

        m_model = glm::rotate(
            glm::mat4(1.f),
            m_angle,
            glm::vec3(0.f, -0.5f, 0.0f)
        ) * 0.5f;

        m_shader.setMat4("u_model", m_model);

        m_target->Render(m_shader);

        m_frameCount++;
        m_currentTicks = SDL_GetTicks();
        Uint64 elapsed = m_currentTicks - m_startTicks;

        if (elapsed >= 1000) { // one second passed
            double fps = static_cast<double>(m_frameCount) / (static_cast<double>(elapsed) / 1000.0);
            std::cout << "FPS: " << fps << std::endl;
            m_frameCount = 0;
            m_startTicks = m_currentTicks;
        }
    }
private:
    Shader m_shader;

    glm::mat4 m_model;
    glm::mat4 m_proj;
    glm::mat4 m_view;

    glm::vec3 m_camPos;
    glm::vec3 m_lightPos;

    float m_angle;
    Uint64 m_lastTicks;

    std::unique_ptr<Object> m_sun = nullptr;
    std::unique_ptr<Object> m_target = nullptr;

    bool m_paused = false;

    float m_yaw   = -90.0f; // looking along -Z initially
    float m_pitch = 0.0f;   // no vertical tilt

    // FPS tracking
    Uint64 m_startTicks;
    int m_frameCount;

    Uint64 m_currentTicks;
};

int main() {
    Engine::Run(std::make_unique<Game>());
    return 0;
}