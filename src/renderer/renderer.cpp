#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "renderer/renderer.h"
#include "window/window.h"
#include "IO/file_manager.h"

Renderer::Renderer(ecs::Entity& light, ecs::Entity& camera)
    : m_light(light), m_camera(camera)
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
}

void Renderer::OnWindowResized(int w, int h) {
    m_proj = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(w) / static_cast<float>(h),
        0.01f,
        100.0f
    );
}

void Renderer::RenderLight() {
    m_view = glm::lookAt(
        m_camera.transform.pos,
        m_camera.transform.pos + m_camera.transform.rot,
        glm::vec3(0.f, 1.f, 0.f)
    );

    m_shader.setMat4("u_view", m_view);
    m_shader.setMat4("u_projection", m_proj);

    m_shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_shader.setVec3("lightPos", m_light.transform.pos);
    m_shader.setVec3("viewPos", m_camera.transform.pos);

    m_model = glm::mat4(1.f);
    m_model = glm::translate(m_model, m_light.transform.pos);

    m_shader.setMat4("u_model", m_model);

    m_light.mesh.object->Render(m_shader);
}

void Renderer::RenderEntity(const ecs::Entity& entity) {
    if (entity.mesh.object == nullptr) {
        std::cerr << "WARN: Entity doesn't have a mesh to render" << std::endl;
        return;
    }

    m_view = glm::lookAt(
        m_camera.transform.pos,
        m_camera.transform.pos + m_camera.transform.rot,
        glm::vec3(0.f, 1.f, 0.f)
    );

    m_model = glm::mat4(1.0f);

    // Apply translation
    m_model = glm::translate(m_model, entity.transform.pos);

    // Apply rotations (order matters!)
    m_model = glm::rotate(m_model, entity.transform.rot.x, glm::vec3(1, 0, 0)); // pitch
    m_model = glm::rotate(m_model, entity.transform.rot.y, glm::vec3(0, 1, 0)); // yaw
    m_model = glm::rotate(m_model, entity.transform.rot.z, glm::vec3(0, 0, 1)); // roll

    m_shader.setMat4("u_model", m_model);

    entity.mesh.object->Render(m_shader);
}