#ifndef RENDERER_BASICS_H
#define RENDERER_BASICS_H

#include <glm/glm.hpp>

#include "renderer/mesh.h"

class Vertex {
    friend class Mesh;
private:
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec2 m_texCoord;
public:
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord)
        : m_position(position), m_normal(normal), m_texCoord(texCoord) {}
public:
    static void DefineAttrib();
};

#endif // RENDERER_BASICS_H