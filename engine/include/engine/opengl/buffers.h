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
        Buffer(BufferTarget target);
        ~Buffer();

        inline const BufferID GetID() const { return m_buffer; }
        inline const BufferTarget GetTarget() const { return m_target; }
    public:
		static bool IsBound(const Buffer& buffer) noexcept
		{
			return m_bound == buffer.GetID();
		}

		static bool IsBound(const Buffer* buffer) noexcept
		{
			return buffer != nullptr && m_bound == buffer->GetID();
		}

		static void Bind(const Buffer* buffer)
		{
			assert(buffer != nullptr && "Trying to bind a null Buffer");

			glBindBuffer(buffer->GetTarget(), buffer->GetID());
			m_bound = buffer->GetID();
		}

		static void Unbind(const Buffer* buffer)
		{
			assert(buffer != nullptr && "Trying to unbind a null Buffer");

			glBindBuffer(buffer->GetTarget(), 0);

			if (m_bound == buffer->GetID())
				m_bound = 0;
		}

        static void Data(const Buffer* buffer, const void* data, size_t size);
        static void SubData(const Buffer* buffer, const void *data, size_t size, size_t offset);
    protected:
        void BindBuffer(unsigned int index) const;
        void BindBufferRanged(unsigned int index, size_t offset, size_t size) const;
    private:
        BufferID m_buffer;
        BufferTarget m_target;
        BufferUsage m_usage;
    private:
		inline static BufferID m_bound = 0;
    };

    // TODO: Implement custom fields structuring via ordered_map?
    class ENGINE_API UniformBuffer : public Buffer {
    public:
        UniformBuffer(size_t size, unsigned int index);

        void ConfigureShader(Shader& shader, const char* uniformName) const;

        template<typename T, typename S = size_t>
        void UpdateUniform(void* data, S offset) {
            SubData(this, data, sizeof(T), offset);
        }
    private:
        unsigned int m_uniformBinding;
    private:
        static unsigned int s_bufferNextId;
    };

    class ENGINE_API ArrayBuffer : public Buffer {
    public:
        ArrayBuffer(BufferUsage usage);
    };

    class ENGINE_API InstanceBuffer : public ArrayBuffer {
    public:
        InstanceBuffer(BufferUsage usage);

        inline void StartConfigure() const { Buffer::Bind(this); }
        inline void EndConfigure() const { Buffer::Unbind(this); }
    };

    class ENGINE_API VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void Bind();
        void Unbind();

        void SetupVertexBuffer(BufferUsage usage);
    public:
        void VertexBufferData(size_t size, const void* data);
    private:
        unsigned int m_id { 0 };
        ArrayBuffer* m_vbo { nullptr };
    };
} // namespace OpenGL

} // namespace Core

#endif // OPENGL_BUFFERS_H_
