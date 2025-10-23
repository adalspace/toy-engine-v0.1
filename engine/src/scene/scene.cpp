#include "engine/scene/scene.h"

#include <iostream>

namespace Core {

Entity Scene::CreateEntity() {
    Entity entity = { m_registry.create(), this };
    // std::cout << "Entities: " << (int)m_registry.view<entt::entity>().size() << std::endl;
    return entity;
}

}
