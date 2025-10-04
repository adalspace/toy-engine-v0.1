#include <SDL3/SDL.h>

#include "window/window.h"
#include "window/events/window.h"

#include <iostream>
#include <GL/glew.h>

#include "renderer/debug.h"

Window::Window() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    std::cout << "Setting gl context version " << ENGINE_GL_MAJOR_VERSION << "." << ENGINE_GL_MINOR_VERSION << std::endl;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, ENGINE_GL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, ENGINE_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    std::cout << "Setting gl context multisample " << ENGINE_GL_MULTISAMPLE_BUFFERS
              << "buffers " << ENGINE_GL_MULTISAMPLE_SAMPLES << " samples" << std::endl;
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, ENGINE_GL_MULTISAMPLE_BUFFERS);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, ENGINE_GL_MULTISAMPLE_SAMPLES);

    m_width = DEFAULT_WIDTH;
    m_height = DEFAULT_HEIGHT;

    std::cout << "Width: " << m_width << std::endl;
    std::cout << "Height: " << m_height << std::endl;

    m_window = SDL_CreateWindow("OpenGL Test", m_width, m_height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALWAYS_ON_TOP|SDL_WINDOW_RESIZABLE);

    if (!m_window) {
        std::cerr << "Failed to create window" << std::endl;
        std::exit(1);
    }

    SDL_SetWindowRelativeMouseMode(m_window, true);

    m_context = SDL_GL_CreateContext(m_window);

    if (!SDL_GL_MakeCurrent(m_window, m_context)) {
        std::cerr << "SDL_GL_MakeCurrent failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(m_window);
        std::exit(1);
    }

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cerr << "Could not initialize GLEW!" << std::endl;
        SDL_GL_DestroyContext(m_context);
        SDL_DestroyWindow(m_window);
        std::exit(1);
    }

    std::cout << "GL_VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GL_VERSION:  " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glDebugMessageCallback(MessageCallback, nullptr);

    glViewport(0, 0, m_width, m_height);

    m_is_open = true;
}

void Window::ProcessEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_QUIT:
                std::cout << "Close requested" << std::endl;
                publish(WindowCloseRequested());
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    std::cout << "Close requested" << std::endl;
                    publish(WindowCloseRequested());
                }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                int width, height;
                if (SDL_GetWindowSizeInPixels(m_window, &width, &height)) {
                    std::cout << "Window resized: " << width << ", " << height << std::endl;
                    m_width = width;
                    m_height = height;
                    glViewport(
                        0,
                        0,
                        width,
                        height);
                    publish(WindowResized{ m_width, m_height });
                }
                break;
            default: break;
        };
    }
}

void Window::SwapBuffers() const {
    SDL_GL_SwapWindow(m_window);
}

Window::~Window() {
    Destroy();
}

void Window::Destroy() const {
    if (m_context)
        SDL_GL_DestroyContext(m_context);
    if (m_window)
        SDL_DestroyWindow(m_window);
}

