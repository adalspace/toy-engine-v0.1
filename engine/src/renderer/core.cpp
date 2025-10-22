#include <memory>

#include "engine/renderer/core.h"

#include "engine/window/event.h"
#include "engine/renderer/wavefront.h"

std::unique_ptr<IApplication> Engine::s_app = nullptr;
std::shared_ptr<Window> Engine::s_window = nullptr;
std::unique_ptr<Renderer> Engine::s_renderer = nullptr;
std::shared_ptr<Scene> Engine::s_scene = nullptr;
bool Engine::s_running = false;

void Engine::Run(std::unique_ptr<IApplication> app) {
    s_scene = std::make_shared<Scene>();
    s_renderer = std::make_unique<Renderer>(s_scene);
    s_window = Window::GetInstance();
    s_app = std::move(app);
    s_running = true;

    s_app->OnInit(s_scene);
    s_renderer->Init();

    s_window->Subscribe<WindowCloseEvent>([&](const WindowCloseEvent& e) {
        s_running = false;
        s_app->OnEvent(e);
    });

    s_window->Subscribe<WindowResizeEvent>([&](const WindowResizeEvent& e) {
        s_renderer->OnWindowResized(e.GetWidth(), e.GetHeight());
        s_app->OnEvent(e);
    });

    while (s_running) {
        s_window->ProcessEvents();

        s_app->OnUpdate();

        s_renderer->Render();

        s_window->SwapBuffers();
    }

    s_app->OnShutdown();

    s_window->Destroy();
    s_app.reset();
}

