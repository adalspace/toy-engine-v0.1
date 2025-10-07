#ifndef ECS_COMP_TRANSFORM_H_
#define ECS_COMP_TRANSFORM_H_

#include <glm/glm.hpp>
#include "ecs/component.h"

namespace ecs {
    class Transform : Component {
    public:
        Transform() = default;
    public:
        glm::vec3 pos;
        glm::vec3 rot;
        glm::vec3 scale;
    };
}

#endif // ECS_COMP_TRANSFORM_H_