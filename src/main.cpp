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
#include "renderer/renderer.h"

#include "IO/file_manager.h"

#include "components/transform.h"
#include "components/light.h"
#include "components/camera.h"
#include "components/mesh.h"

class Game : public IApplication {
public:
    Game() {
        Object* lightObj = Object::LoadFile("./assets/sphere.obj");
        // const auto lightEntity = m_registry.create();
        // m_registry.emplace<transform>(lightEntity, glm::vec3(-5.f, 5.f, 5.f), glm::vec3(0.f));
        // m_registry.emplace<light>(lightEntity, glm::vec3(1.f, 0.f, 0.f), 1.f);
        // m_registry.emplace<mesh>(lightEntity, std::unique_ptr<Object>(lightObj));

        const auto lEntt2 = m_registry.create();
        m_registry.emplace<transform>(lEntt2, glm::vec3(5.f, 5.f, 5.f), glm::vec3(0.f));
        m_registry.emplace<light>(lEntt2, glm::vec3(1.f, 1.f, 1.f), 1.5f);
        m_registry.emplace<mesh>(lEntt2, std::unique_ptr<Object>(lightObj));

        const auto cameraEntity = m_registry.create();
        m_registry.emplace<transform>(cameraEntity, glm::vec3(0.f, 2.f, 2.f));
        m_registry.emplace<camera>(cameraEntity);

        Object* targetObj = Object::LoadFile("./assets/wizard/wizard.obj");
        const auto targetEntity = m_registry.create();
        m_registry.emplace<transform>(targetEntity, glm::vec3(0.f, 0.0f, 0.f));
        m_registry.emplace<mesh>(targetEntity, std::unique_ptr<Object>(targetObj));

        Object* cubeObj = Object::LoadFile("./assets/cube.obj");
        const auto cubeEntity = m_registry.create();
        m_registry.emplace<transform>(cubeEntity, glm::vec3(-1.5f, 0.4f, 0.f));
        m_registry.emplace<mesh>(cubeEntity, std::unique_ptr<Object>(cubeObj));

        Object* floorObj = Object::LoadFile("./assets/plane.obj");
        const auto floorEntt = m_registry.create();
        m_registry.emplace<transform>(floorEntt, glm::vec3(0.f));
        m_registry.emplace<mesh>(floorEntt, std::unique_ptr<Object>(floorObj));
    }
    ~Game() override {}

    void OnInit() override {
        std::cout << "Game initialized" << std::endl;

        m_angle = 3.45f;
        m_lastTicks = SDL_GetTicks();

        m_paused = false;

        m_yaw   = -90.0f; // looking along -Z initially
        m_pitch = 0.0f;   // no vertical tilt

        // FPS tracking
        m_startTicks = SDL_GetTicks();
        m_frameCount = 0;

        m_renderer.GenerateShadowMaps(m_registry);
    }

    void OnWindowResized(const WindowResized& event) override {
        m_renderer.OnWindowResized(event.w, event.h);
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

        auto view = m_registry.view<camera, transform>();
        for (auto [cam, camTransform] : view.each()) {
            camTransform.position += velocity * deltaTime * 2.5f; // speed is e.g. 2.5f
            camTransform.rotation = cameraViewDirection;
        }

        // update rotation
        if (!m_paused) {
            m_angle += glm::radians(45.0f) * deltaTime; // 72° per second
            if (m_angle > glm::two_pi<float>()) {
                m_angle -= glm::two_pi<float>(); // keep value small
            }
        }

        // auto rotateEntts = m_registry.view<transform, const mesh>();
        // for (auto [entity, transform, mesh] : rotateEntts.each()) {
        //     // auto targetTransform = rotateEntts.get<transform>(entity);
        //     if (!m_registry.all_of<light>(entity)) {
        //         transform.rotation.y = m_angle;
        //     }
        // }
    }

    void OnRender() override {
        m_renderer.Render(m_registry);

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
    Renderer m_renderer;
    entt::registry m_registry;

    float m_angle;
    Uint64 m_lastTicks;

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