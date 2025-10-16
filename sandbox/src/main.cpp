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

#include "engine/renderer/shader.h"
#include "engine/renderer/wavefront.h"
#include "engine/renderer/core.h"
#include "engine/renderer/renderer.h"

#include "engine/IO/file_manager.h"

#include "engine/components/transform.h"
#include "engine/components/light.h"
#include "engine/components/camera.h"
#include "engine/components/mesh.h"
#include "engine/components/rotate.h"
#include "engine/components/batch.h"

class Game : public IApplication {
public:
    Game() : m_renderer(m_registry) {
        Object* lightObj = Object::LoadFile("./assets/sphere.obj");
        const auto lght = m_registry.create();
        m_registry.emplace<transform>(lght, glm::vec3(5.f, 5.f, 5.f), glm::vec3(0.f));
        m_registry.emplace<light>(lght, light::LightType::DIRECTIONAL, glm::vec3(1.f, 1.f, 1.f), 1.5f);
        m_registry.emplace<mesh>(lght, std::shared_ptr<Object>(lightObj));

        const auto cameraEntity = m_registry.create();
        m_registry.emplace<transform>(cameraEntity, glm::vec3(0.f, 2.f, 2.f));
        m_registry.emplace<camera>(cameraEntity);

        Object* targetObj = Object::LoadFile("./assets/wizard/wizard.obj");
        const auto targetEntity = m_registry.create();
        m_registry.emplace<transform>(targetEntity, glm::vec3(0.f, 0.0f, 0.f));
        m_registry.emplace<mesh>(targetEntity, std::shared_ptr<Object>(targetObj));

        Object* grass = Object::LoadFile("./assets/grass_block/grass_block.obj");
        const auto cubeEntity = m_registry.create();
        m_registry.emplace<transform>(cubeEntity, glm::vec3(-1.5f, 0.4f, 0.f));
        m_registry.emplace<mesh>(cubeEntity, std::shared_ptr<Object>(grass));

        // Cube template (use shared object to avoid reloading 1000 times)
        std::shared_ptr<Object> cubeObj = std::shared_ptr<Object>(Object::LoadFile("./assets/grass_block/grass_block.obj"));
        const auto batchEntt = m_registry.create();
        m_registry.emplace<batch>(batchEntt);
        m_registry.emplace<mesh>(batchEntt, cubeObj);
        auto cubeBatch = m_registry.get<batch>(batchEntt);
        // Generate 1000 random cubes
        for (int i = 0; i < 1000; ++i) {
            const auto cubeEntity = m_registry.create();

            float x = static_cast<float>(rand()) / RAND_MAX * 200.f - 100.f; // range [-100, 100]
            float y = static_cast<float>(rand()) / RAND_MAX * 10.f;          // range [0, 10]
            float z = static_cast<float>(rand()) / RAND_MAX * 200.f - 100.f; // range [-100, 100]

            m_registry.emplace<transform>(cubeEntity, glm::vec3(x, y, z));
            m_registry.emplace<rotate>(cubeEntity);
            m_registry.emplace<batch::item>(cubeEntity, cubeBatch.id());
        }

        Object* floorObj = Object::LoadFile("./assets/plane.obj");
        const auto floorEntt = m_registry.create();
        m_registry.emplace<transform>(floorEntt, glm::vec3(0.f));
        m_registry.emplace<mesh>(floorEntt, std::shared_ptr<Object>(floorObj));
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

        m_renderer.Init();
        m_renderer.GenerateShadowMaps();
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

        // ---- Day-night simulation ----
        m_dayTime += deltaTime;
        if (m_dayTime > m_dayLength)
            m_dayTime -= m_dayLength; // loop every "day"

        float dayProgress = m_dayTime / m_dayLength; // 0.0 -> 1.0
        float sunAngle = dayProgress * glm::two_pi<float>(); // radians through the sky

        // Compute sun direction (rotating around X axis)
        // At t=0.0 sun at east horizon, at π/2 overhead, at π west horizon
        glm::vec3 sunDir = glm::normalize(glm::vec3(0.0f, sin(sunAngle), cos(sunAngle)));

        // Compute intensity: bright at noon, dim at dusk/dawn, dark at night
        float intensity = glm::max(sin(sunAngle), (double)0.0f); // 0 at night, 1 at noon
        intensity = glm::mix(0.05f, 1.5f, intensity);    // keep some ambient even at night

        // Optional: tint color (warm at sunrise/sunset)
        glm::vec3 dayColor   = glm::vec3(1.0f, 0.95f, 0.9f);
        glm::vec3 sunsetColor= glm::vec3(1.0f, 0.6f, 0.3f);
        float sunsetFactor = glm::clamp(1.0f - abs(sin(sunAngle)) * 2.0f, 0.0f, 1.0f);
        glm::vec3 sunColor = glm::mix(dayColor, sunsetColor, sunsetFactor);

        // Update the directional light in the registry
        auto lightsView = m_registry.view<light, transform>();
        for (auto [entity, l, t] : lightsView.each()) {
            if (l.type == light::LightType::DIRECTIONAL) {
                // "position" for directional light often stores direction vector
                // If your system instead uses transform.rotation, adjust accordingly
                t.position = sunDir * 15.f;       // use this as light direction
                l.color = sunColor;
                l.intensity = intensity;
            }
        }

        auto rotateEntts = m_registry.view<transform, rotate>();
        for (auto [entity, t] : rotateEntts.each()) {
            // auto targetTransform = rotateEntts.get<transform>(entity);
            if (!m_registry.all_of<light>(entity)) {
                t.rotation.y = m_angle;
            }
        }
    }

    void OnRender() override {
        m_renderer.Render();

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

    float m_dayTime = 0.0f;       // accumulates time for day-night cycle
    float m_dayLength = 60.0f;    // seconds per full day cycle

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