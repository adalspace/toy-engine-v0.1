#ifndef COMPONENT_BATCH_H_
#define COMPONENT_BATCH_H_

#include <glm/mat4x4.hpp>
#include <memory>

#include "engine/opengl/buffers.h"

#include "engine/export.h"

namespace Core {
// requires mesh component
struct ENGINE_API batch {
public:
    // requires Transform component
    struct item {
        unsigned int batchId;
    };

    batch();

    inline const unsigned int id() const { return m_id; }
    inline const bool Initialized() const { return m_instanceBuffer != nullptr; }
protected:
    static unsigned int LastID;
private:
    unsigned int m_id;
    unsigned int m_instance_vbo { 0 };
    unsigned int m_instance_count { 0 };

    // TODO: use static draw when possible
    std::unique_ptr<OpenGL::InstanceBuffer> m_instanceBuffer = nullptr;
private:
    friend class Renderer;
    void prepare(glm::mat4 *instances, unsigned int count);
};
}

#endif // COMPONENT_BATCH_H_