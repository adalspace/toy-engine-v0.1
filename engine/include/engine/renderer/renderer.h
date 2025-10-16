#ifndef RENDERER_H_
#define RENDERER_H_

#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>

#include "engine/renderer/shader.h"

// TODO: make static or singleton
class Renderer {
public:
    Renderer(entt::registry& registry);

    void Render();
    void Init();
    void GenerateShadowMaps();

    void OnWindowResized(int w, int h);
private:
    void ApplyLights(Shader &shader);
    void UpdateView();
    void RenderScene(Shader &shader);
private:
    Shader m_shader;
    Shader m_depthShader;

    entt::registry& m_registry;

    // unsigned int m_depth_fbo;
    // unsigned int m_depthMap;

    glm::mat4 m_model;
    glm::mat4 m_proj;
    glm::mat4 m_view;
};

#endif // RENDERER_H_