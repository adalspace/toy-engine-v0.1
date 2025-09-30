#ifndef VERTEX_H_
#define VERTEX_H_
#include <vector>
#include <glm/glm.hpp>

#include "block.h"

class Point {
private:
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec4 m_color;
public:
    Point(glm::vec3 position, glm::vec3 normal, glm::vec4 color);
};

class Vertices {
private:
    std::vector<Point> m_items;
    std::vector<unsigned int> m_indices;

    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_ebo;
public:
    Vertices();
public: // GPU
    void Bind();
    void Unbind();
    void Draw();
    void Upload();
public:
    void PushVertex(const Point& point);
    void PushIndex(unsigned int index);
};

#endif // VERTEX_H_
