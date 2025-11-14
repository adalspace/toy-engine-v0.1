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

#include "engine/3d/prefab.hpp"

#include "engine/scene/scene.h"
#include "engine/input/input.h"

#include "engine/api.h"

using namespace Core;

class ModelViewer : public IApplication {
public:
    ModelViewer() = default;
    ~ModelViewer() override {}

    void OnInit(std::shared_ptr<Scene> scene) override {
        m_scene = scene;

        Object* lightObj = Object::LoadFile("./assets/common/sphere/sphere.obj");
        lightEntity = scene->CreateEntity();
        lightEntity.AddComponent<Transform>(glm::vec3(5.f, 5.f, 5.f), glm::vec3(0.f));
        lightEntity.AddComponent<light>(light::LightType::DIRECTIONAL, glm::vec3(1.f, 1.f, 1.f), 1.5f);
        lightEntity.AddComponent<mesh>(std::shared_ptr<Renderable>(lightObj));
        assert(lightEntity.HasComponent<mesh>() && "light doesn't have any mesh!");

        cameraEntity = scene->CreateEntity();
        cameraEntity.AddComponent<camera>();
        cameraEntity.AddComponent<Transform>(glm::vec3(0.f, 2.f, 2.f));
        assert(cameraEntity.HasComponent<camera>() && "Camera doesn't have required 'camera' component");
        assert(cameraEntity.HasComponent<Transform>() && "Camera doesn't have 'transform' component");

        Object* targetObj = Object::LoadFile("./assets/grass_block/grass_block.obj");
        modelEntity = scene->CreateEntity();
        modelEntity.AddComponent<Transform>(glm::vec3(0.f, 0.0f, 0.f));
        modelEntity.AddComponent<mesh>(std::shared_ptr<Renderable>(targetObj));
        assert(modelEntity.HasComponent<mesh>() && "model doesn't have any mesh!");

        std::cout << "ModelViewer initialized" << std::endl;
    }

    void OnUpdate(Timestep dt) override {
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
};

IApplication* CreateApplication() {
    return new ModelViewer();
}
