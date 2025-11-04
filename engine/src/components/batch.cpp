#include <GL/glew.h>

#include "engine/components/batch.h"

namespace Core {

unsigned int batch::LastID = 0;

batch::batch() {
    m_id = ++LastID;
}

void batch::prepare(glm::mat4 *instances, unsigned int count) {
    if (!m_instanceBuffer) {
        m_instanceBuffer = new OpenGL::InstanceBuffer(GL_DYNAMIC_DRAW);
        OpenGL::Buffer::Bind(m_instanceBuffer);
        OpenGL::Buffer::Data(m_instanceBuffer, nullptr, sizeof(glm::mat4) * count);
        OpenGL::Buffer::Unbind(m_instanceBuffer);
        m_instance_count = count;
    } else if (count > m_instance_count) {
        // Optional: reallocate only if you *really* have more instances than before
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        OpenGL::Buffer::Bind(m_instanceBuffer);
        OpenGL::Buffer::Data(m_instanceBuffer, nullptr, sizeof(glm::mat4) * count);
        OpenGL::Buffer::Unbind(m_instanceBuffer);
        m_instance_count = count;
    }

    // Just update the data region — much cheaper
    OpenGL::Buffer::SubData(m_instanceBuffer, instances, sizeof(glm::mat4) * count, 0);
}

}
