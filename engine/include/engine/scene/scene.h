#ifndef ENGINE_SCENE_H_
#define ENGINE_SCENE_H_

#include <entt/entt.hpp>

class Scene {
public:
    Scene();
private:
    entt::registry m_registry;
    friend class Renderer;
    friend class Game;
};

#endif // ENGINE_SCENE_H_