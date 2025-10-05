#include <memory>

#include "renderer/engine.h"
#include "window/event.h"

#include "IO/file_manager.h"
#include "renderer/shader.h"
#include "renderer/wavefront.h"

std::unique_ptr<IApplication> Engine::s_app = nullptr;
std::shared_ptr<Window> Engine::s_window = nullptr;
bool Engine::s_running = false;

void Engine::Run(std::unique_ptr<IApplication> app) {
    s_app = std::move(app);
    s_window = Window::GetInstance();
    s_running = true;

    s_app->OnInit();

    s_window->Subscribe<WindowCloseRequested>([](const WindowCloseRequested& e) {
        Engine::s_running = false;
    });

    s_window->Subscribe<WindowResized>([](const WindowResized& e) {
        Engine::s_app->OnWindowResized(e);
    });

    while (s_running) {
        s_window->ProcessEvents();

        s_app->OnUpdate();
        
        glClearColor(0x18/255.0f, 0x18/255.0f, 0x18/255.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        s_app->OnRender();

        s_window->SwapBuffers();
    }

    s_app->OnShutdown();

    s_window->Destroy();
    s_app.reset();
}

