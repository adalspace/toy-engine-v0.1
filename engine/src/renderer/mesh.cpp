#include <cstddef>

#include "engine/renderer/mesh.h"

namespace Engine {

Mesh::Mesh() {
    m_vao = 0;
    m_vbo = 0;
    m_ebo = 0;
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // VBO (vertex buffer)
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // EBO (index buffer)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, m_position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, m_normal)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, m_texCoord)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::Upload() const {
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size() * sizeof(Vertex), m_vertexBuffer.data(), GL_DYNAMIC_DRAW);

    // Upload indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer.size() * sizeof(unsigned int), m_indexBuffer.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::Render(unsigned int count)
{
    Bind();
    if (count > 1) {
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_INT, 0, count);
    } else {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_INT, 0);
    }
    Unbind();
}

}
