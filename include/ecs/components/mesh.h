#ifndef ECS_COMP_MESH_H_
#define ECS_COMP_MESH_H_

#include "renderer/wavefront.h"
#include "ecs/component.h"

namespace ecs {
    class Mesh : Component {
    public:
        Object* object;

        Mesh(Object* model) : object(model) {}
    };
}

#endif // ECS_COMP_MESH_H_