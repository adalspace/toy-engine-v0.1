#include <memory>

#include "engine/renderer/core.h"

#include "engine/window/event.hpp"
#include "engine/renderer/wavefront.h"

Engine* Engine::s_instance = nullptr;

void Engine::Run(std::unique_ptr<IApplication> app) {
    m_scene = std::make_shared<Scene>();
    m_renderer = std::make_unique<Renderer>(m_scene);
    m_window = Window::GetInstance();
    m_app = std::move(app);
    m_running = true;

    m_app->OnInit(m_scene);
    m_renderer->Init();

    // m_window->Subscribe<WindowCloseEvent>([&](const WindowCloseEvent& e) {
        
    //     m_app->OnEvent(e);
    // });

    // m_window->Subscribe<WindowResizeEvent>([&](const WindowResizeEvent& e) {
    //     m_renderer->OnWindowResized(e.GetWidth(), e.GetHeight());
    //     m_app->OnEvent(e);
    // });

    m_window->Subscribe2(this);

    while (m_running) {
        m_window->ProcessEvents();

        m_app->OnUpdate();

        m_renderer->Render();

        m_window->SwapBuffers();
    }

    m_app->OnShutdown();

    m_window->Destroy();
    m_app.reset();
}

void Engine::OnEvent(const Event& event) {
    m_app->OnEvent(event);
    if (event.GetCategory() == Event::EventCategory::WINDOW) {
        if (event.GetType() == EventType::WINDOW_RESIZE) {
            auto e = static_cast<const WindowResizeEvent&>(event);
            m_renderer->OnWindowResized(e.GetWidth(), e.GetHeight());
        }
        if (event.GetType() == EventType::WINDOW_CLOSE) {
            m_running = false;
        }
    }
}

Engine* Engine::GetInstance() {
    if (!s_instance) {
        s_instance = new Engine();
    }

    return s_instance;
}

