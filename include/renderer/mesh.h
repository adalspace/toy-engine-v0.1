#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include <string>
#include <GL/glew.h>

#include "renderer/basics.h"

class Mesh {
public: // TODO: abstract away
    unsigned int m_vao, m_vbo, m_ebo;
    std::vector<Vertex> m_vertexBuffer;
    std::vector<unsigned int> m_indexBuffer;
public: // TODO: abstract away
    void Bind() { glBindVertexArray(m_vao); }
    void Unbind() { glBindVertexArray(0); }
    void Upload();
public:
    std::string materialName;
public:
    Mesh();
public:
    void Render();
};

#endif // MESH_H_