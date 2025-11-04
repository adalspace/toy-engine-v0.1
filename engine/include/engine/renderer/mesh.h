#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include <string>
#include <GL/glew.h>

#include "engine/3d/vertex.hpp"
#include "engine/opengl/buffers.h"

namespace Core {

class Mesh : public OpenGL::VertexArray {
public: // TODO: abstract away
    unsigned int m_ebo;
    std::vector<Vertex> m_vertexBuffer;
    std::vector<unsigned int> m_indexBuffer;
public:
    Mesh(const std::string& name);

    Mesh(const Mesh& other) = delete;
    Mesh(Mesh&& other) noexcept
        : m_ebo(other.m_ebo), m_vertexBuffer(std::move(other.m_vertexBuffer)), m_indexBuffer(std::move(other.m_indexBuffer)), m_materialName(std::move(other.m_materialName)) {}
public:
    inline const std::string& GetMaterialName() const { return m_materialName; }
    void Upload();
public:
    void Render(unsigned int count);
private:
    std::string m_materialName;
};

}

#endif // MESH_H_