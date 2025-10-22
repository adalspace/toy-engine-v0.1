#ifndef COMPONENTS_LIGHT_H_
#define COMPONENTS_LIGHT_H_

#include <glm/glm.hpp>

#include "engine/renderer/renderer.h"
#include "engine/export.h"

namespace Engine {
struct ENGINE_API light {
    friend class Renderer;
public:
    enum LightType {
        DIRECTIONAL = 0,
    };
    LightType type;
    glm::vec3 color;
    float intensity;

    light(LightType t, const glm::vec3& c, float i)
        : type(t), color(c), intensity(i),
          shadowMap(0), fbo(0), lightSpace(1.0f) {}
private:
    unsigned int shadowMap;
    unsigned int fbo;
    glm::mat4 lightSpace;
    int shadowRes{1024};
};
}

#endif // COMPONENTS_LIGHT_H_