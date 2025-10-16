#ifndef COMPONENT_BATCH_H_
#define COMPONENT_BATCH_H_

#include "engine/renderer/renderer.h"
#include "engine/export.h"

// requires mesh component
struct ENGINE_API batch {
    friend class Renderer;
public:
    // requires transform component
    struct item {
        unsigned int batchId;
    };

    batch();

    inline const unsigned int id() const { return m_id; }
protected:
    static unsigned int LastID;
private:
    unsigned int m_id;
    unsigned int m_instance_vbo { 0 };
private:
    void prepare(glm::mat4 *instances, unsigned int count);
};

#endif // COMPONENT_BATCH_H_