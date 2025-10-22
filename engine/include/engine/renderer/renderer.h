#ifndef RENDERER_H_
#define RENDERER_H_

#include <glm/glm.hpp>

#include "engine/scene/scene.h"
#include "engine/renderer/shader.h"
#include "engine/export.h"
#include "engine/components/light.h"

// TODO: make static or singleton
class ENGINE_API Renderer {
public:
    Renderer(std::shared_ptr<Scene> scene);

    void Render();
    void Init();

    void OnWindowResized(int w, int h);
private:
    void ApplyLights(Shader &shader);
    void UpdateView();
    void RenderScene(Shader &shader);
    void GenerateShadowMaps();
    void EnsureShadowResources(light& l);
private:
    Shader m_shader;
    Shader m_depthShader;

    std::shared_ptr<Scene> m_scene;

    // unsigned int m_depth_fbo;
    // unsigned int m_depthMap;

    glm::mat4 m_model;
    glm::mat4 m_proj;
    glm::mat4 m_view;
};

#endif // RENDERER_H_