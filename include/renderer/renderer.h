#ifndef RENDERER_H_
#define RENDERER_H_

#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>

#include "renderer/shader.h"

// TODO: make static or singleton
class Renderer {
public:
    Renderer();

    void Render(entt::registry& registry);

    void OnWindowResized(int w, int h);
private:
    Shader m_shader;

    glm::mat4 m_model;
    glm::mat4 m_proj;
    glm::mat4 m_view;
};

#endif // RENDERER_H_