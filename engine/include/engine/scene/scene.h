#ifndef ENGINE_SCENE_H_
#define ENGINE_SCENE_H_

#include <entt/entt.hpp>
#include <memory>

namespace Engine {

class Scene;

class Entity {
    friend class Scene;
private:
    Entity(entt::entity entity, Scene* scene) : m_entity(entity), m_scene(scene) {}
public:
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
    entt::entity m_entity;
    Scene *m_scene;
};

class Scene {
private:
    friend class Entity;
public:
    Scene();

    std::unique_ptr<Entity> CreateEntity();
private:
    entt::registry m_registry;
    friend class Renderer;
};

} // namespace Engine

#endif // ENGINE_SCENE_H_