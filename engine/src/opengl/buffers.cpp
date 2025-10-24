#include "engine/opengl/buffers.h"

namespace Core {

namespace OpenGL {

    Buffer::Buffer(BufferTarget target, BufferUsage usage)
        : m_target(target), m_usage(usage)
    {
        glGenBuffers(1, &m_buffer);
        Bind();
        Data(nullptr, 0);
        Unbind();
    }

    void Buffer::Bind() {
        glBindBuffer(m_target, m_buffer);
    }

    void Buffer::Unbind() {
        glBindBuffer(m_target, 0);
    }

    void Buffer::Data(void *data, size_t size) {
        Bind();
        glBufferData(m_target, size, data, m_usage);
        Unbind();
    }

    void Buffer::SubData(void *data, size_t size, size_t offset) {
        Bind();
        glBufferSubData(m_target, offset, size, data);
        Unbind();
    }

    void Buffer::BindBuffer(unsigned int index) {
        Bind();
        glBindBufferBase(m_target, index, m_buffer);
        Unbind();
    }

    void Buffer::BindBufferRanged(unsigned int index, size_t offset, size_t size) {
        Bind();
        glBindBufferRange(m_target, index, m_buffer, offset, size);
        Unbind();
    }

    unsigned int UniformBuffer::s_bufferNextId = 1;

    UniformBuffer::UniformBuffer(size_t size, unsigned int index)
        : Buffer(GL_UNIFORM_BUFFER, GL_STATIC_DRAW), m_uniformBinding(s_bufferNextId++)
    {
        Data(nullptr, size);

        BindBuffer(m_uniformBinding);
    }

    void UniformBuffer::ConfigureShader(Shader& shader, const char* uniformName) {
        auto uniformIndex = glGetUniformBlockIndex(shader.GetID(), uniformName);
        glUniformBlockBinding(shader.GetID(), uniformIndex, m_uniformBinding);
    }

} // namespace OpenGL

} // namespace Core