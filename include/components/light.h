#ifndef COMPONENTS_LIGHT_H_
#define COMPONENTS_LIGHT_H_

#include <glm/glm.hpp>

struct light {
    glm::vec3 color;
    float intensity;
};

#endif // COMPONENTS_LIGHT_H_