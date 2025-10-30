#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#ifdef _WIN32
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/extended_min_max.hpp>
#endif

#include "engine/renderer/wavefront.h"

#include "engine/app/app.h"

#include "engine/components/transform.h"
#include "engine/components/light.h"
#include "engine/components/camera.h"
#include "engine/components/mesh.h"
#include "engine/components/rotate.h"
#include "engine/components/batch.h"

#include "engine/scene/scene.h"
#include "engine/input/input.h"

#include "engine/api.h"

using namespace Core;

class Game : public IApplication {
public:
    Game() = default;
    ~Game() override {}

    void OnInit(std::shared_ptr<Scene> scene) override {
        m_scene = scene;

        Object* lightObj = Object::LoadFile("./assets/common/sphere/sphere.obj");
        lightEntity = scene->CreateEntity();
        lightEntity.AddComponent<Transform>(glm::vec3(5.f, 5.f, 5.f), glm::vec3(0.f));
        lightEntity.AddComponent<light>(light::LightType::DIRECTIONAL, glm::vec3(1.f, 1.f, 1.f), 1.5f);
        lightEntity.AddComponent<mesh>(std::shared_ptr<Object>(lightObj));
        assert(lightEntity.HasComponent<mesh>() && "light doesn't have any mesh!");

        cameraEntity = scene->CreateEntity();
        cameraEntity.AddComponent<camera>();
        cameraEntity.AddComponent<Transform>(glm::vec3(0.f, 2.f, 2.f));
        assert(cameraEntity.HasComponent<camera>() && "Camera doesn't have required 'camera' component");
        assert(cameraEntity.HasComponent<Transform>() && "Camera doesn't have 'transform' component");

        Object* targetObj = Object::LoadFile("./assets/wizard/wizard.obj");
        modelEntity = scene->CreateEntity();
        modelEntity.AddComponent<Transform>(glm::vec3(0.f, 0.0f, 0.f));
        modelEntity.AddComponent<mesh>(std::shared_ptr<Object>(targetObj));
        modelEntity.AddComponent<rotate>();
        assert(modelEntity.HasComponent<mesh>() && "model doesn't have any mesh!");

        // Cube template (use shared object to avoid reloading 1000 times)
        std::shared_ptr<Object> cubeObj = std::shared_ptr<Object>(Object::LoadFile("./assets/grass_block/grass_block.obj"));
        auto batchEntt = scene->CreateEntity();
        auto& cubeBatch = batchEntt.AddComponent<batch>();
        // auto& cubeBatch = batchEntt.GetComponent<batch>();
        batchEntt.AddComponent<mesh>(cubeObj);
        assert(batchEntt.HasComponent<batch>() && "batch doesn't have any batch component!");
        assert(batchEntt.HasComponent<mesh>() && "batch doesn't have any mesh component!");
        // Generate 1000 random cubes
        for (int i = 0; i < 100; ++i) {
            auto cubeEntity = scene->CreateEntity();

            float x = static_cast<float>(rand()) / RAND_MAX * 200.f - 100.f; // range [-100, 100]
            float y = static_cast<float>(rand()) / RAND_MAX * 10.f;          // range [0, 10]
            float z = static_cast<float>(rand()) / RAND_MAX * 200.f - 100.f; // range [-100, 100]

            cubeEntity.AddComponent<Transform>(glm::vec3(x, y, z));
            cubeEntity.AddComponent<rotate>();
            cubeEntity.AddComponent<batch::item>(cubeBatch.id());
        }

        Object* floorObj = Object::LoadFile("./assets/common/plane/plane.obj");
        auto floorEntt = scene->CreateEntity();
        floorEntt.AddComponent<Transform>(glm::vec3(0.f));
        floorEntt.AddComponent<mesh>(std::shared_ptr<Object>(floorObj));
        assert(floorEntt.HasComponent<mesh>() && "floor doesn't have any mesh component!");

        std::cout << "Game initialized" << std::endl;

        m_angle = 3.45f;

        m_yaw   = -90.0f; // looking along -Z initially
        m_pitch = 0.0f;   // no vertical tilt
    }

    void OnUpdate(Timestep dt) override {
        glm::vec2 mouseRel = Input::GetRelativeMouse();

        float sensitivity = 0.1f; // tweak as needed
        m_yaw   += mouseRel.x * sensitivity;
        m_pitch -= mouseRel.y * sensitivity; // invert Y for typical FPS control

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

        glm::vec3 front = glm::normalize(glm::vec3(cameraViewDirection.x, 0.f, cameraViewDirection.z));
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.f, 1.f, 0.f)));

        if (Input::IsKeyPressed(SDL_SCANCODE_W)) velocity += front;
        if (Input::IsKeyPressed(SDL_SCANCODE_S)) velocity -= front;
        if (Input::IsKeyPressed(SDL_SCANCODE_A)) velocity -= right;
        if (Input::IsKeyPressed(SDL_SCANCODE_D)) velocity += right;
        if (Input::IsKeyPressed(SDL_SCANCODE_SPACE)) velocity.y += 1.f;
        if (Input::IsKeyPressed(SDL_SCANCODE_LSHIFT)) velocity.y -= 1.f;

        auto& camTransform = cameraEntity.GetComponent<Transform>();
        camTransform.position += velocity * (float)dt * 2.5f; // speed is e.g. 2.5f
        camTransform.rotation = cameraViewDirection;

        // update rotation
        m_angle += glm::radians(45.0f) * dt; // 72° per second
        if (m_angle > glm::two_pi<float>()) {
            m_angle -= glm::two_pi<float>(); // keep value small
        }

        // ---- Day-night simulation ----
        m_dayTime += dt;
        if (m_dayTime > m_dayLength)
            m_dayTime -= m_dayLength; // loop every "day"

        float dayProgress = m_dayTime / m_dayLength; // 0.0 -> 1.0
        float sunAngle = dayProgress * glm::two_pi<float>(); // radians through the sky

        // Compute sun direction (rotating around X axis)
        // At t=0.0 sun at east horizon, at π/2 overhead, at π west horizon
        glm::vec3 sunDir = glm::normalize(glm::vec3(0.0f, sin(sunAngle), cos(sunAngle)));

        // Compute intensity: bright at noon, dim at dusk/dawn, dark at night
        float intensity = glm::max(static_cast<double>(sin(sunAngle)), static_cast<double>(0.0f)); // 0 at night, 1 at noon
        intensity = glm::mix(0.05f, 1.5f, intensity);    // keep some ambient even at night

        // Optional: tint color (warm at sunrise/sunset)
        glm::vec3 dayColor   = glm::vec3(1.0f, 0.95f, 0.9f);
        glm::vec3 sunsetColor= glm::vec3(1.0f, 0.6f, 0.3f);
        float sunsetFactor = glm::clamp(1.0f - abs(sin(sunAngle)) * 2.0f, 0.0f, 1.0f);
        glm::vec3 sunColor = glm::mix(dayColor, sunsetColor, sunsetFactor);

        // Update the directional light in the registry
        auto& l = lightEntity.GetComponent<light>();
        auto& t = lightEntity.GetComponent<Transform>();
        if (l.type == light::LightType::DIRECTIONAL) {
            // "position" for directional light often stores direction vector
            // If your system instead uses transform.rotation, adjust accordingly
            t.position = sunDir * 15.f;       // use this as light direction
            l.color = sunColor;
            l.intensity = intensity;
        }

        m_elapsed += dt.GetMilliseconds();

        if (m_elapsed >= 1000) { // one second passed
            m_elapsed = 0;
            double fps = 1 / dt;
            std::cout << "FPS: " << fps << std::endl;
        }
    }

    void OnEvent(const Event& event) override {
        if (event.GetType() == EventType::WINDOW_RESIZE) {
            auto resizeEvent = static_cast<const WindowResizeEvent&>(event);
            std::cout << "[DEBUG] <EVENT> Window resized to " << resizeEvent.GetWidth() << "x" << resizeEvent.GetHeight() << std::endl;
        }
        else if (event.GetType() == EventType::WINDOW_CLOSE) {
            std::cout << "[DEBUG] <EVENT> Window closing" << std::endl;
        }
    }
private:
    // for internal 1-second timer
    int m_elapsed;

    std::shared_ptr<Scene> m_scene;

    Entity lightEntity;
    Entity cameraEntity;
    Entity modelEntity;

    float m_angle;

    float m_dayTime = 0.0f;       // accumulates time for day-night cycle
    float m_dayLength = 60.0f;    // seconds per full day cycle

    float m_yaw   = -90.0f; // looking along -Z initially
    float m_pitch = 0.0f;   // no vertical tilt
};

IApplication* CreateApplication() {
    return new Game();
}
