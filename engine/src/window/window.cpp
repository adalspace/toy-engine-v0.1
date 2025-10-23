#include <SDL3/SDL.h>

#include "engine/window/window.h"
#include "engine/window/events/window_events.h"

#include <iostream>
#include <GL/glew.h>

#include "engine/renderer/debug.h"

namespace Core {

std::shared_ptr<Window> Window::s_instance = nullptr;

Window::Window(const char* title, int width, int height) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, ENGINE_GL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, ENGINE_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, ENGINE_GL_MULTISAMPLE_BUFFERS);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, ENGINE_GL_MULTISAMPLE_SAMPLES);

    m_width = width;
    m_height = height;

    m_handle = SDL_CreateWindow(title, m_width, m_height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALWAYS_ON_TOP|SDL_WINDOW_RESIZABLE);

    if (!m_handle) {
        std::cerr << "Failed to create window" << std::endl;
        std::exit(1);
    }

    SDL_SetWindowRelativeMouseMode(m_handle, true);
    SDL_Rect boundaries = {0, 0, m_width, m_height};
    SDL_SetWindowMouseRect(m_handle, &boundaries);

    m_context = SDL_GL_CreateContext(m_handle);

    if (!SDL_GL_MakeCurrent(m_handle, m_context)) {
        std::cerr << "SDL_GL_MakeCurrent failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(m_handle);
        std::exit(1);
    }

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cerr << "Could not initialize GLEW!" << std::endl;
        SDL_GL_DestroyContext(m_context);
        SDL_DestroyWindow(m_handle);
        std::exit(1);
    }

    std::cout << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDebugMessageCallback(MessageCallback, nullptr);

    glViewport(0, 0, m_width, m_height);
}

Window::Window() : Window("OpenGL Test", DEFAULT_WIDTH, DEFAULT_HEIGHT) {}

std::shared_ptr<Window> Window::GetInstance() {
    if (!s_instance) {
        s_instance = std::shared_ptr<Window>(new Window(), WindowDeleter{});
    }
    return s_instance;
}

Window::Window(Window&& window) noexcept
    : m_handle(window.m_handle), m_context(window.m_context), m_width(window.m_width), m_height(window.m_height)
{
    window.m_handle = nullptr;
    window.m_context = (SDL_GLContext)nullptr;
    window.m_width = 0;
    window.m_height = 0;
}

Window& Window::operator=(Window&& window) noexcept
{
    if (this == &window) return *this;

    // Destroy();

    this->m_handle = window.m_handle;
    this->m_context = window.m_context;
    this->m_width = window.m_width;
    this->m_height = window.m_height;

    return *this;
}

void Window::ProcessEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_QUIT:
                EmitEvent(WindowCloseEvent{});
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    EmitEvent(WindowCloseEvent{});
                }
                if (event.key.scancode == SDL_SCANCODE_F11) {
                    bool isFullscreen = SDL_GetWindowFlags(m_handle) & SDL_WINDOW_FULLSCREEN;
                    SDL_SetWindowFullscreen(m_handle, !isFullscreen);
                }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                int width, height;
                if (SDL_GetWindowSizeInPixels(m_handle, &width, &height)) {
                    m_width = width;
                    m_height = height;
                    glViewport(
                        0,
                        0,
                        width,
                        height);
                    auto event = WindowResizeEvent(static_cast<unsigned int>(m_width), static_cast<unsigned int>(m_height));
                    EmitEvent(event);
                    SDL_SetWindowRelativeMouseMode(m_handle, true);
                    SDL_Rect boundaries = {0, 0, m_width, m_height};
                    SDL_SetWindowMouseRect(m_handle, &boundaries);
                }
                break;
            default: break;
        };
    }
}

void Window::SwapBuffers() const {
    SDL_GL_SwapWindow(m_handle);
}

Window::~Window() {
    Destroy();
}

void Window::Destroy() const {
    if (m_context)
        SDL_GL_DestroyContext(m_context);
    if (m_handle)
        SDL_DestroyWindow(m_handle);
}

}
