#pragma once

#include <entt/entt.hpp>
#include <memory>

namespace Engine {

class Entity;

class Scene {
private:
    friend class Entity;
public:
    Scene() = default;

    Entity CreateEntity();
private:
    entt::registry m_registry;
    friend class Renderer;
};

class Entity {
public:
    Entity() = default;
    Entity(entt::entity entity, Scene* scene) : m_entity(entity), m_scene(scene) {}
    Entity(const Entity& other) = default;

    template<typename Type, typename... Args>
    inline auto AddComponent(Args &&...args) {
        assert(this->m_scene != nullptr && "Scene has not been assigned to the entity");
        return m_scene->m_registry.emplace<Type>(m_entity, std::forward<Args>(args)...);
    }

    template<typename Type>
    [[nodiscard]] inline auto GetComponent() {
        assert(this->m_scene != nullptr && "Scene has not been assigned to the entity");
        return m_scene->m_registry.get<Type>(m_entity);
    }
private:
    entt::entity m_entity { 0 };
    Scene *m_scene = nullptr;
};

} // namespace Engine
