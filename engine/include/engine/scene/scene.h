#pragma once

#include <entt/entt.hpp>
#include "engine/export.h"

namespace Engine {

class Entity;

class ENGINE_API Scene {
    friend class Entity;
public:
    Scene() = default;

    Entity CreateEntity();
private:
    entt::registry m_registry;
    friend class Renderer;
};

class ENGINE_API Entity {
public:
    Entity() = default;
    Entity(entt::entity entity, Scene* scene) : m_entity(entity), m_scene(scene) {}
    Entity(const Entity& other) = default;

    template<typename Type, typename... Args>
    auto AddComponent(Args &&...args) {
        assert(this->m_entity != entt::null && "Entity is empty");
        assert(this->m_scene != nullptr && "Scene has not been assigned to the entity");
        return m_scene->m_registry.emplace<Type>(m_entity, std::forward<Args>(args)...);
    }

    template<typename Type>
    [[nodiscard]] auto GetComponent() const {
        assert(this->m_entity != entt::null && "Entity is empty");
        assert(this->m_scene != nullptr && "Scene has not been assigned to the entity");
        return m_scene->m_registry.get<Type>(m_entity);
    }

    template<typename Type>
    [[nodiscard]] bool HasComponent() const {
        return m_scene->m_registry.all_of<Type>(m_entity);
    }
private:
    entt::entity m_entity { entt::null };
    Scene *m_scene = nullptr;
};

} // namespace Engine
