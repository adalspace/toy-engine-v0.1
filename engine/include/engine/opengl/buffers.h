#ifndef OPENGL_BUFFERS_H_
#define OPENGL_BUFFERS_H_

#include <GL/glew.h>

#include "engine/renderer/shader.h"
#include "engine/export.h"

namespace Core {

namespace OpenGL {
    using BufferTarget = GLenum;
    using BufferUsage = GLenum;
    using BufferID = unsigned int;

    class ENGINE_API Buffer {
    public:
        Buffer(BufferTarget target, BufferUsage usage);

    protected:
        void Data(void* data, size_t size);
        void SubData(void *data, size_t size, size_t offset);
        
        void BindBuffer(unsigned int index);
        void BindBufferRanged(unsigned int index, size_t offset, size_t size);
    protected:
        void Bind();
        void Unbind();
    private:
        BufferID m_buffer;
        BufferTarget m_target;
        BufferUsage m_usage;
    };

    class ENGINE_API UniformBuffer : public Buffer {
    public:
        UniformBuffer(size_t size, unsigned int index);

        void ConfigureShader(Shader& shader, const char* uniformName);

        template<typename T, typename S = size_t>
        void UpdateUniform(void* data, S offset) {
            SubData(data, sizeof(T), offset);
        }
    private:
        unsigned int m_uniformBinding;
    private:
        static unsigned int s_bufferNextId;
    };
} // namespace OpenGL

} // namespace Core

#endif // OPENGL_BUFFERS_H_
