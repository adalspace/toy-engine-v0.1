#include <GL/glew.h>

#include "components/batch.h"

unsigned int batch::LastID = 0;

batch::batch() {
    m_id = ++LastID;
}

void batch::prepare(glm::mat4 *instances, unsigned int count) {
    if (m_instance_vbo == 0) {
        glGenBuffers(1, &m_instance_vbo);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * count, reinterpret_cast<void*>(instances), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}