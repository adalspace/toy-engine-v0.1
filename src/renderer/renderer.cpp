#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "renderer/renderer.h"
#include "window/window.h"
#include "IO/file_manager.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/light.h"
#include "components/mesh.h"

Renderer::Renderer()
{
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

    m_model = glm::mat4(1.f);

    m_shader.use();

    m_shader.setMat4("u_projection", m_proj);
}

void Renderer::OnWindowResized(int w, int h) {
    m_proj = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(w) / static_cast<float>(h),
        0.01f,
        100.0f
    );
    m_shader.setMat4("u_projection", m_proj);
}

void Renderer::Render(entt::registry& registry) {
    auto view = registry.view<transform, mesh>();

    auto cam = registry.view<transform, camera>().back();
    auto camTransform = registry.get<transform>(cam);

    auto lightEntt = registry.view<transform, light>().back();
    auto lightTransform = registry.get<transform>(lightEntt);

    m_view = glm::lookAt(
        camTransform.position,
        camTransform.position + camTransform.rotation,
        glm::vec3(0.f, 1.f, 0.f)
    );
    m_shader.setMat4("u_view", m_view);

    m_shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_shader.setVec3("lightPos", lightTransform.position);
    m_shader.setVec3("viewPos", camTransform.position);

    // std::cout << "cam pos: " << "vec(" << camTransform.position.x << ", " << camTransform.position.y << ", " << camTransform.position.z << ")" << std::endl;
    // std::cout << "cam rot: " << "vec(" << camTransform.rotation.x << ", " << camTransform.rotation.y << ", " << camTransform.rotation.z << ")" << std::endl;

    // std::cout << "light pos: " << "vec(" << lightTransform.position.x << ", " << lightTransform.position.y << ", " << lightTransform.position.z << ")" << std::endl;
    // std::cout << "light rot: " << "vec(" << lightTransform.rotation.x << ", " << lightTransform.rotation.y << ", " << lightTransform.rotation.z << ")" << std::endl;

    for (auto [entity, transf, mesh] : view.each()) {
        if (mesh.object == nullptr) {
            std::cerr << "WARN: Entity doesn't have a mesh to render" << std::endl;
            return;
        }

        m_model = glm::mat4(1.0f);

        // Apply translation
        m_model = glm::translate(m_model, transf.position);

        // Apply rotations (order matters!)
        m_model = glm::rotate(m_model, transf.rotation.x, glm::vec3(1, 0, 0)); // pitch
        m_model = glm::rotate(m_model, transf.rotation.y, glm::vec3(0, 1, 0)); // yaw
        m_model = glm::rotate(m_model, transf.rotation.z, glm::vec3(0, 0, 1)); // roll

        m_shader.setMat4("u_model", m_model);

        mesh.object->Render(m_shader);
    }
}