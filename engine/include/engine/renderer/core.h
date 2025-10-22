#ifndef ENGINE_H_
#define ENGINE_H_

#include <memory>
#include <glm/glm.hpp>

#include "engine/window/window.h"
#include "engine/window/events/window.h"

#include "engine/renderer/renderer.h"
#include "engine/scene/scene.h"

#include "engine/app/app.h"
#include "engine/export.h"

class ENGINE_API Engine {
public:
    static void Run(std::unique_ptr<IApplication> app);
private:
    static std::unique_ptr<IApplication> s_app;
    static std::shared_ptr<Window> s_window;
    static std::unique_ptr<Renderer> s_renderer;
    static std::shared_ptr<Scene> s_scene;
    static bool s_running;
};


#endif // ENGINE_H_