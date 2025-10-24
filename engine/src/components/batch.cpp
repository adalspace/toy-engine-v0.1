#include <GL/glew.h>

#include "engine/components/batch.h"

namespace Core {

unsigned int batch::LastID = 0;

batch::batch() {
    m_id = ++LastID;
}

void batch::prepare(glm::mat4 *instances, unsigned int count) {
    if (!m_instanceBuffer) {
        m_instanceBuffer = std::make_unique<OpenGL::InstanceBuffer>(GL_DYNAMIC_DRAW);
        m_instanceBuffer->Data(nullptr, sizeof(glm::mat4) * count);
        m_instance_count = count;
    } else if (count > m_instance_count) {
        // Optional: reallocate only if you *really* have more instances than before
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        m_instanceBuffer->Data(nullptr, sizeof(glm::mat4) * count);
        m_instance_count = count;
    }

    // Just update the data region — much cheaper
    m_instanceBuffer->SubData(instances, sizeof(glm::mat4) * count, 0);
}

}
