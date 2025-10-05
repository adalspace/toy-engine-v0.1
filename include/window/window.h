#ifndef WINDOW_H_
#define WINDOW_H_
#include <SDL3/SDL.h>

#include "event.hpp"

#define ENGINE_GL_MAJOR_VERSION 4
#define ENGINE_GL_MINOR_VERSION 6
#define ENGINE_GL_MULTISAMPLE_BUFFERS 1
#define ENGINE_GL_MULTISAMPLE_SAMPLES 8

#define DEFAULT_WIDTH 1024
#define DEFAULT_HEIGHT 768

class Window : public EventBus {
private:
    SDL_Window *m_handle;
    SDL_GLContext m_context;

    int m_width;
    int m_height;

    bool m_is_open;
public:
    Window();
    Window(const char* title, int width, int height);
    ~Window();

    Window(Window&& window) noexcept;
    Window& operator=(Window&& window) noexcept;

    Window(const Window& window) noexcept = delete;
    Window& operator=(const Window& window) noexcept = delete;
public:
    [[nodiscard]] inline int GetWidth() const { return m_width; }
    [[nodiscard]] inline int GetHeight() const { return m_height; }
    [[nodiscard]] inline bool IsOpen() const { return m_is_open; }
public:
    void ProcessEvents();
public:
    void SwapBuffers() const;
public:
    void Destroy() const;
};

#endif //WINDOW_H_