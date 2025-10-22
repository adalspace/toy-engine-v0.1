#include "engine/scene/scene.h"

namespace Engine {

Scene::Scene() = default;

std::unique_ptr<Entity> Scene::CreateEntity() {
    return std::unique_ptr<Entity>(new Entity(m_registry.create(), this));
}

} // namespace Engine
