#ifndef ECS_ENTITY_CAMERA_H_
#define ECS_ENTITY_CAMERA_H

#include "ecs/entity.h"

namespace ecs { 
    class Camera : Entity {
    public:
        Camera() = default;
        ~Camera() = default;
    };
}

#endif // ECS_ENTITY_CAMERA_H_