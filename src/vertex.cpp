#include <GL/glew.h>

#include "vertex.h"

#define BLOCK_SIZE 0.5f

Point::Point(glm::vec3 position, glm::vec3 normal, glm::vec4 color)
{
    m_position = position;
    m_normal = normal;
    m_color = color;
}

Vertices::Vertices()
{
    m_items = std::vector<Point>();

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

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangleIndices), triangleIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Call this after you finish adding vertices (or call it each time after PushBlock)
void Vertices::Upload()
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_items.size() * sizeof(Point), m_items.data(), GL_DYNAMIC_DRAW);

    // Upload indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// void Vertices::PushBlock(const Block& block)
// {
//     // 1 face
//     m_items.emplace_back(block.Position(), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, -BLOCK_SIZE, 0.f), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, 0.f), block.Color());

//     // 2 face
//     m_items.emplace_back(block.Position(), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, 0.f), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, 0.f), block.Color());

//     // 3 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, -BLOCK_SIZE, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, -BLOCK_SIZE), block.Color());

//     // 4 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, -BLOCK_SIZE), block.Color());

//     // 5 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position(), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, -BLOCK_SIZE, 0.f), block.Color());

//     // 6 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, -BLOCK_SIZE, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, -BLOCK_SIZE, 0.f), block.Color());

//     // 7 face
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, 0.f), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, 0.f), block.Color());

//     // 8 face
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, -BLOCK_SIZE, 0.f), block.Color());

//     // 9 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, 0.f), block.Color());
//     m_items.emplace_back(block.Position(), block.Color());

//     // 10 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, 0.f, 0.f), block.Color());

//     // 11 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, BLOCK_SIZE, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, BLOCK_SIZE, 0.f), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, BLOCK_SIZE, 0.f), block.Color());

//     // 12 face
//     m_items.emplace_back(block.Position() + glm::vec3(0.f, BLOCK_SIZE, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, BLOCK_SIZE, -BLOCK_SIZE), block.Color());
//     m_items.emplace_back(block.Position() + glm::vec3(BLOCK_SIZE, BLOCK_SIZE, 0.f), block.Color());
// }

void Vertices::PushVertex(const Point& point)
{
    m_items.push_back(point);
}

void Vertices::PushIndex(unsigned int index)
{
    m_indices.push_back(index);
}

void Vertices::Bind()
{
    glBindVertexArray(m_vao);
}

void Vertices::Unbind()
{
    glBindVertexArray(0);
}

void Vertices::Draw()
{
    Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    Unbind();
}