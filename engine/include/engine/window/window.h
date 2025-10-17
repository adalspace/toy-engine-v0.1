#ifndef WINDOW_H_
#define WINDOW_H_
#include <SDL3/SDL.h>
#include <memory>

#include "engine/window/event.h"

#define ENGINE_GL_MAJOR_VERSION 4
#define ENGINE_GL_MINOR_VERSION 6
#define ENGINE_GL_MULTISAMPLE_BUFFERS 1
#define ENGINE_GL_MULTISAMPLE_SAMPLES 8

#define DEFAULT_WIDTH 1024
#define DEFAULT_HEIGHT 768

class Window : public EventDispatcher {
    friend class Engine;
private:
    Window();
    Window(const char* title, int width, int height);
    ~Window();

    struct WindowDeleter {
        void operator()(Window* w) const { delete w; };
    };
public:
    static std::shared_ptr<Window> GetInstance();

    Window(Window&& window) noexcept;
    Window& operator=(Window&& window) noexcept;

    Window(const Window& window) noexcept = delete;
    Window& operator=(const Window& window) noexcept = delete;
public:
    [[nodiscard]] static inline int GetWidth() { return Window::GetInstance()->m_width; }
    [[nodiscard]] static inline int GetHeight() { return Window::GetInstance()->m_height; }
private:
    void ProcessEvents();
    void SwapBuffers() const;
    void Destroy() const;
private:
    static std::shared_ptr<Window> s_instance;
    
    SDL_Window *m_handle;
    SDL_GLContext m_context;

    int m_width;
    int m_height;
};

#endif //WINDOW_H_