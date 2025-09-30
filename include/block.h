#ifndef BLOCK_H_
#define BLOCK_H_
#include <glm/glm.hpp>

class Block {
private:
    glm::vec3 m_position;
    glm::vec4 m_color;
public:
    Block(glm::vec3 position, glm::vec4 color);
public:
    inline glm::vec3 Position() const { return m_position; }
    inline glm::vec4 Color() const { return m_color; }
};

#endif