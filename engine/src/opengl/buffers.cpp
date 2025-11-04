#include <iostream>
#include <cassert>
#include "engine/opengl/buffers.h"

namespace Core {

namespace OpenGL {

    BufferID Buffer::m_bound = 0;

    Buffer::Buffer(BufferTarget target, BufferUsage usage)
        : m_target(target), m_usage(usage)
    {
        glGenBuffers(1, &m_buffer);
    }

    Buffer::Buffer(BufferTarget target)
        : Buffer(target, GL_STATIC_DRAW) {}
    
    Buffer::~Buffer() {
        glDeleteBuffers(1, &m_buffer);
    }

    void Buffer::Data(const Buffer* buffer, const void *data, size_t size) {
        if (!IsBound(buffer)) Bind(buffer);
        glBufferData(buffer->m_target, size, data, buffer->m_usage);
    }

    void Buffer::SubData(const Buffer* buffer, const void *data, size_t size, size_t offset) {
        if (!IsBound(buffer)) Bind(buffer);
        glBufferSubData(buffer->m_target, offset, size, data);
    }

    void Buffer::BindBuffer(unsigned int index) const {
        Buffer::Bind(this);
        glBindBufferBase(m_target, index, m_buffer);
        Buffer::Unbind(this);
    }

    void Buffer::BindBufferRanged(unsigned int index, size_t offset, size_t size) const {
        Buffer::Bind(this);
        glBindBufferRange(m_target, index, m_buffer, offset, size);
        Buffer::Unbind(this);
    }

    unsigned int UniformBuffer::s_bufferNextId = 1;

    UniformBuffer::UniformBuffer(size_t size, unsigned int index)
        : Buffer(GL_UNIFORM_BUFFER, GL_STATIC_DRAW), m_uniformBinding(s_bufferNextId++)
    {
        Buffer::Bind(this);
        Data(this, nullptr, size);
        Buffer::Unbind(this);

        BindBuffer(m_uniformBinding);
    }

    void UniformBuffer::ConfigureShader(Shader& shader, const char* uniformName) const {
        auto uniformIndex = glGetUniformBlockIndex(shader.GetID(), uniformName);
        glUniformBlockBinding(shader.GetID(), uniformIndex, m_uniformBinding);
    }

    ArrayBuffer::ArrayBuffer(BufferUsage usage)
        : Buffer(GL_ARRAY_BUFFER, usage) {}
    
    InstanceBuffer::InstanceBuffer(BufferUsage usage)
        : ArrayBuffer(usage) {}

    VertexArray::VertexArray() : m_id(0) {
        std::cout << "Vertex Array init" << std::endl;
        glGenVertexArrays(1, &m_id);
        std::cout << "m_id: " << m_id << std::endl;
    }

    VertexArray::~VertexArray() {
        // if (m_vbo) {
        //     delete m_vbo;
        // }

        glDeleteVertexArrays(1, &m_id);
    }

    void VertexArray::Bind() {
        std::cout << "Binding VAO" << std::endl;
        assert(m_id != 0 && "Vertex Array wasn't initialized.");

        glBindVertexArray(m_id);
    }

    void VertexArray::Unbind() {
        assert(m_id != 0 && "Vertex Array wasn't initialized.");
        
        // TODO: Add EBO as well
        if (Buffer::IsBound(m_vbo)) {
            Buffer::Unbind(m_vbo);
        }

        glBindVertexArray(0);
    }

    void VertexArray::SetupVertexBuffer(BufferUsage usage) {
        if (m_vbo) {
            delete m_vbo;
        }

        m_vbo = new ArrayBuffer(usage);
        Buffer::Bind(m_vbo);
        Buffer::Data(m_vbo, nullptr, 0);
    }

    void VertexArray::VertexBufferData(size_t size, const void* data) {
        assert(m_vbo != nullptr && "Trying to upload vertex buffer data to nullptr");

        Buffer::Bind(m_vbo);
        Buffer::Data(m_vbo, data, size);
    }

} // namespace OpenGL

} // namespace Core