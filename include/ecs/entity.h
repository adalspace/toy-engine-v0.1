#ifndef ECS_ENTITY_H_
#define ECS_ENTITY_H_

#include "ecs/components/transform.h"
#include "ecs/components/mesh.h"

namespace ecs {
    class Entity {
    public:
        Transform transform;
        Mesh mesh;

        Entity(Mesh m) : mesh(m) {}
        ~Entity() = default;
    };
}

#endif // ECS_ENTITY_H_