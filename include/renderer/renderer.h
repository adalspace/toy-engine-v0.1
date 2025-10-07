#ifndef RENDERER_H_
#define RENDERER_H_

#include <glm/glm.hpp>
#include "renderer/shader.h"
#include "ecs/entity.h"

// TODO: make static or singleton
class Renderer {
public:
    Renderer(ecs::Entity& light, ecs::Entity& camera);

    void RenderLight();
    void RenderEntity(const ecs::Entity& entity);

    void OnWindowResized(int w, int h);
private:
    Shader m_shader;

    glm::mat4 m_model;
    glm::mat4 m_proj;
    glm::mat4 m_view;

    ecs::Entity& m_light;
    ecs::Entity& m_camera;
};

#endif // RENDERER_H_