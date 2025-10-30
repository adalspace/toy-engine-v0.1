#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include <string>
#include <GL/glew.h>

#include "engine/renderer/basics.h"
#include "engine/opengl/buffers.h"

namespace Core {

class Mesh : public OpenGL::VertexArray {
public: // TODO: abstract away
    unsigned int m_vbo, m_ebo;
    std::vector<Vertex> m_vertexBuffer;
    std::vector<unsigned int> m_indexBuffer;
public:
    void Upload();
public:
    std::string materialName;
public:
    Mesh();
public:
    void Render(unsigned int count);
};

}

#endif // MESH_H_