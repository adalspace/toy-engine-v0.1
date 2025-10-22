#include <GL/glew.h>

#include "engine/components/batch.h"

namespace Engine {

unsigned int batch::LastID = 0;

batch::batch() {
    m_id = ++LastID;
}

void batch::prepare(glm::mat4 *instances, unsigned int count) {
    if (m_instance_vbo == 0) {
        glGenBuffers(1, &m_instance_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        // Allocate *once*, no data yet — just reserve space
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * count, nullptr, GL_DYNAMIC_DRAW);
        m_instance_count = count;
    } else if (count > m_instance_count) {
        // Optional: reallocate only if you *really* have more instances than before
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * count, nullptr, GL_DYNAMIC_DRAW);
        m_instance_count = count;
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
    }

    // Just update the data region — much cheaper
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * count, instances);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}
