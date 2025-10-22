#include "engine/scene/scene.h"

namespace Engine {

Entity Scene::CreateEntity() {
    return { m_registry.create(), this };
}

}
