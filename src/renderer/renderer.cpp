#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#ifdef WIN32
#include <corecrt_math_defines.h>
#endif
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

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
        FileManager::read("./src/shaders/pbr.fs")
    );

    m_depthShader.init(
        FileManager::read("./src/shaders/depth.vs"),
        FileManager::read("./src/shaders/depth.fs")
    );

    m_model = glm::mat4(1.f);

    SwitchShader(&m_shader);

    m_shader.setMat4("u_projection", m_proj);
}

void Renderer::SwitchShader(Shader *newShader) {
    m_currentShader = newShader;
    m_currentShader->use();
}

void Renderer::OnWindowResized(int w, int h) {
    m_proj = glm::perspective(
        static_cast<float>(M_PI_2),
        static_cast<float>(w) / static_cast<float>(h),
        0.01f,
        100.0f
    );
    m_shader.setMat4("u_projection", m_proj);
    m_depthShader.setMat4("u_projection", m_proj);
}

void Renderer::ApplyLights(entt::registry& registry) {
    auto lights = registry.view<light>();
    // TODO: Pass Lights Data to depth shader as well
    m_shader.setInt("lightsCount", static_cast<int>(lights.size()));
    size_t lightIndex = 0;
    for (auto entity : lights) {
        auto &comp = registry.get<light>(entity);
        auto &transf = registry.get<transform>(entity);

        m_shader.setVec3("lights[" + std::to_string(lightIndex) + "].position", transf.position);
        m_shader.setVec3("lights[" + std::to_string(lightIndex) + "].color", comp.color);
        m_shader.setFloat("lights[" + std::to_string(lightIndex) + "].intensity", comp.intensity);

        ++lightIndex;
    }
}

void Renderer::UpdateView(entt::registry& registry) {
    auto cam = registry.view<transform, camera>().back();
    auto camTransform = registry.get<transform>(cam);

    m_view = glm::lookAt(
        camTransform.position,
        camTransform.position + camTransform.rotation,
        glm::vec3(0.f, 1.f, 0.f)
    );
    m_shader.setMat4("u_view", m_view);

    m_shader.setVec3("viewPos", camTransform.position);
}

void Renderer::RenderScene(entt::registry& registry) {
    auto view = registry.view<transform, mesh>();

    for (auto [entity, transf, mesh] : view.each()) {
        if (mesh.object == nullptr) {
            std::cerr << "WARN: Entity doesn't have a mesh to render" << std::endl;
            return;
        }

        if (registry.all_of<light>(entity)) {
            auto &comp = registry.get<light>(entity);
            m_currentShader->setBool("isLight", true);
            m_currentShader->setVec3("currentLightColor", comp.color);
        } else {
            m_currentShader->setBool("isLight", false);
            m_currentShader->setVec3("currentLightColor", glm::vec3(0.f));
        }

        glm::mat4 rotation = glm::yawPitchRoll(transf.rotation.y, transf.rotation.x, transf.rotation.z);
        m_model = glm::translate(glm::mat4(1.f), transf.position) * rotation;

        m_currentShader->setMat4("u_model", m_model);

        mesh.object->Render(*m_currentShader);
    }
}

void Renderer::GenerateShadowMaps(entt::registry& registry) {
    SwitchShader(&m_depthShader);

    ApplyLights(registry);
    UpdateView(registry);
    
    glGenFramebuffers(1, &m_depth_fbo);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    glGenTextures(1, &m_depthMap);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_depth_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_shader.setInt("shadowMap", 31);
}

void Renderer::Render(entt::registry& registry) {
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto shadowLight = registry.view<light, transform>().back();
    auto &comp = registry.get<transform>(shadowLight);

    float near_plane = 0.1f, far_plane = 50.0f; // pick bounds that cover your scene
    glm::vec3 lightPos = comp.position;
    glm::vec3 target   = glm::vec3(0.0f, 0.5f, 0.0f);
    glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjection = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, 1.0f, 20.0f);
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // lightView = glm::lookAt(/*eye*/ -lightDir * distance, /*center*/ vec3(0), up)

    // glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    SwitchShader(&m_depthShader);
    m_currentShader->setMat4("u_lightSpace", lightSpaceMatrix);

    // enable culling and render front faces to the shadow map
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);  // only for the depth pass
    // or use polygon offset:
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depth_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        RenderScene(registry);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // enable culling and render front faces to the shadow map
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // only for the depth pass
    // or use polygon offset:
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.f, 1.f);

    glViewport(0, 0, Window::GetWidth(), Window::GetHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    SwitchShader(&m_shader);

    ApplyLights(registry);

    UpdateView(registry);

    m_currentShader->setMat4("u_lightSpace", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE31);
    glBindTexture(GL_TEXTURE_2D, m_depthMap);

    RenderScene(registry);
}