#ifndef COMPONENTS_TRANSFORM_H_
#define COMPONENTS_TRANSFORM_H_

#include <glm/glm.hpp>

struct transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

#endif // COMPONENTS_TRANSFORM_H_