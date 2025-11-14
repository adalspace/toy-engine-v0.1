#pragma once

#include <entt/entt.hpp>
#include "engine/export.h"

namespace Core {

class Entity;
class Renderer;
class Prefab;

class ENGINE_API Scene {
public:
    Scene() = default;

    Entity CreateEntity();
private:
    entt::registry m_registry;
    friend class Core::Renderer;
    friend class Core::Entity;
    friend class Core::Prefab;
};

class ENGINE_API Entity {
public:
    Entity() = default;
    Entity(entt::entity entity, Scene* scene) : m_entity(entity), m_scene(scene) {}
    Entity(const Entity& other) = default;

    template<typename Type, typename... Args>
    decltype(auto) AddComponent(Args&&... args) {
        assert(m_entity != entt::null && "Entity is empty");
        assert(m_scene && "Scene has not been assigned to the entity");

        if constexpr (std::is_void_v<decltype(m_scene->m_registry.emplace<Type>(m_entity, std::forward<Args>(args)...))>) {
            m_scene->m_registry.emplace<Type>(m_entity, std::forward<Args>(args)...);
        } else {
            return m_scene->m_registry.emplace<Type>(m_entity, std::forward<Args>(args)...);
        }
    }

    template<typename Type>
    [[nodiscard]] Type& GetComponent() {
        assert(m_entity != entt::null && "Entity is empty");
        assert(m_scene && "Scene has not been assigned to the entity");
        return m_scene->m_registry.get<Type>(m_entity);
    }

    template<typename Type>
    [[nodiscard]] const Type& GetComponent() const {
        assert(m_entity != entt::null && "Entity is empty");
        assert(m_scene && "Scene has not been assigned to the entity");
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

} // namespace Core
