#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#ifdef WIN32
#include <corecrt_math_defines.h>
#endif
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "engine/renderer/renderer.h"
#include "engine/window/window.h"
#include "engine/IO/file_manager.h"

#include "engine/components/transform.h"
#include "engine/components/camera.h"
#include "engine/components/light.h"
#include "engine/components/mesh.h"
#include "engine/components/batch.h"

Renderer::Renderer(entt::registry& registry) : m_registry(registry)
{
    m_proj = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(Window::GetWidth()) / static_cast<float>(Window::GetHeight()),
        0.01f,
        100.0f
    );

    m_shader.init(
        FileManager::read("./src/shaders/main.vs"),
        FileManager::read("./src/shaders/pbr.fs")
    );

    m_depthShader.init(
        FileManager::read("./src/shaders/depth.vs"),
        FileManager::read("./src/shaders/depth.fs")
    );

    m_model = glm::mat4(1.f);

    m_shader.use();
    m_shader.setMat4("u_projection", m_proj);
}

void Renderer::Init() {
    // auto view = m_registry.view<batch, mesh>();
    // for (auto [_, b, m] : m_registry.view<batch, mesh>().each()) {
    //     unsigned int items = 0;
    //     for (auto [entt, item] : m_registry.view<batch::item>().each()) {
    //         if (item.batchId == b.id()) ++items;
    //     }
    //     b.prepare()
    //     m.object->EnableBatch(b.m_instance_vbo);
    // }
}

void Renderer::OnWindowResized(int w, int h) {
    m_proj = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(w) / static_cast<float>(h),
        0.01f,
        100.0f
    );
}

void Renderer::ApplyLights(Shader &shader) {
    auto lights = m_registry.view<light>();
    // TODO: Pass Lights Data to depth shader as well
    shader.setInt("lightsCount", static_cast<int>(lights.size()));
    size_t lightIndex = 0;
    for (auto entity : lights) {
        auto &l = m_registry.get<light>(entity);
        auto &transf = m_registry.get<transform>(entity);
        
        shader.setInt("lights[" + std::to_string(lightIndex) + "].type", static_cast<int>(l.type));
        shader.setVec3("lights[" + std::to_string(lightIndex) + "].position", transf.position);
        shader.setVec3("lights[" + std::to_string(lightIndex) + "].color", l.color);
        shader.setFloat("lights[" + std::to_string(lightIndex) + "].intensity", l.intensity);
        shader.setMat4("lights[" + std::to_string(lightIndex) + "].lightSpace", l.lightSpace);
        shader.setInt("lights[" + std::to_string(lightIndex) + "].shadowMap", 10 + lightIndex);
        glActiveTexture(GL_TEXTURE10 + lightIndex);
        glBindTexture(GL_TEXTURE_2D, l.shadowMap);

        ++lightIndex;
    }
}

void Renderer::UpdateView() {
    auto cam = m_registry.view<transform, camera>().back();
    auto camTransform = m_registry.get<transform>(cam);

    m_view = glm::lookAt(
        camTransform.position,
        camTransform.position + camTransform.rotation,
        glm::vec3(0.f, 1.f, 0.f)
    );
    m_shader.setMat4("u_view", m_view);
    m_shader.setMat4("u_projection", m_proj);

    m_shader.setVec3("viewPos", camTransform.position);
}

void Renderer::RenderScene(Shader &shader) {
    std::unordered_map<unsigned int, std::vector<entt::entity>> batches;

    for (auto [entt, item] : m_registry.view<batch::item>().each()) {
        if (batches.find(item.batchId) == batches.end())
            batches.insert(std::make_pair(item.batchId, std::vector<entt::entity>()));

        batches[item.batchId].push_back(entt);
    }

    shader.setBool("u_isInstanced", true);
    shader.setBool("isLight", false);
    shader.setVec3("currentLightColor", glm::vec3(0.f));
    for (auto [entt, b, m] : m_registry.view<batch, mesh>().each()) {
        // check if have items for batch render
        if (batches.find(b.id()) == batches.end()) continue;

        auto &batchItems = batches[b.id()];

        std::vector<glm::mat4> models;
        models.reserve(batchItems.size());

        for (auto item : batchItems) {
            auto &t = m_registry.get<transform>(item);
            glm::mat4 rotation = glm::yawPitchRoll(t.rotation.y, t.rotation.x, t.rotation.z);
            auto itemModel = glm::translate(glm::mat4(1.f), t.position) * rotation;
            models.push_back(itemModel);
        }

        auto prevInstanceVBO = b.m_instance_vbo;
        b.prepare(models.data(), models.size());
        if (prevInstanceVBO <= 0) {
            std::cout << "[DEBUG] enabling batch"<<std::endl;
            m.object->EnableBatch(b.m_instance_vbo);
        }
        m.object->Render(shader, batchItems.size());
    }
    shader.setBool("u_isInstanced", false);

    for (auto [entity, transf, mesh] : m_registry.view<transform, mesh>(entt::exclude<batch, batch::item>).each()) {
        if (mesh.object == nullptr) {
            std::cerr << "WARN: Entity doesn't have a mesh to render" << std::endl;
            return;
        }

        if (m_registry.all_of<light>(entity)) {
            auto &l = m_registry.get<light>(entity);
            shader.setBool("isLight", true);
            shader.setVec3("currentLightColor", l.color);
        } else {
            shader.setBool("isLight", false);
            shader.setVec3("currentLightColor", glm::vec3(0.f));
        }

        glm::mat4 rotation = glm::yawPitchRoll(transf.rotation.y, transf.rotation.x, transf.rotation.z);
        m_model = glm::translate(glm::mat4(1.f), transf.position) * rotation;

        shader.setMat4("u_model", m_model);

        mesh.object->Render(shader, 1);
    }
}

void Renderer::GenerateShadowMaps() {
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    m_depthShader.use();

    auto lights = m_registry.view<light>();

    for (auto [lEntt, l] : lights.each()) {
        // TODO: support other light types when ready
        if (l.type != light::LightType::DIRECTIONAL) return;

        glGenFramebuffers(1, &l.fbo);
        glGenTextures(1, &l.shadowMap);
        glBindTexture(GL_TEXTURE_2D, l.shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                    SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, l.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, l.shadowMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer::Render() {
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    m_depthShader.use();

    auto lights = m_registry.view<light, transform>();

    for (auto [lEntt, l, t] : lights.each()) {
        // TODO: support other light types when ready
        if (l.type != light::LightType::DIRECTIONAL) return;

        glClearColor(0x18/255.0f, 0x18/255.0f, 0x18/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float near_plane = 0.1f, far_plane = 50.0f;
        glm::vec3 target   = glm::vec3(0.0f, 0.5f, 0.0f);
        glm::mat4 lightView = glm::lookAt(t.position, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightProjection = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, 1.0f, 20.0f);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        m_depthShader.setMat4("u_lightSpace", lightSpaceMatrix);
        l.lightSpace = lightSpaceMatrix;

        glCullFace(GL_FRONT);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, l.fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            RenderScene(m_depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);
    }

    // actual rendering

    glViewport(0, 0, Window::GetWidth(), Window::GetHeight());

    glClearColor(0x18/255.0f, 0x18/255.0f, 0x18/255.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_shader.use();

    ApplyLights(m_shader);
    UpdateView();

    RenderScene(m_shader);
}