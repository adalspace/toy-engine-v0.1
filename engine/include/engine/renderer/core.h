#ifndef ENGINE_H_
#define ENGINE_H_

#include <memory>
#include <glm/glm.hpp>

#include "engine/window/window.h"
#include "engine/window/events/window_events.h"

#include "engine/renderer/renderer.h"
#include "engine/scene/scene.h"

#include "engine/app/app.h"
#include "engine/export.h"

namespace Engine {

class ENGINE_API Engine : public EventHandler {
public:
    static Engine* GetInstance();    

    void Run(std::unique_ptr<IApplication> app);
private:
    Engine() {}
    static Engine* s_instance;

    void OnEvent(const Event& event) override;
private:
    std::unique_ptr<IApplication> m_app;
    std::shared_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::shared_ptr<Scene> m_scene;
    bool m_running;
};

}

#endif // ENGINE_H_