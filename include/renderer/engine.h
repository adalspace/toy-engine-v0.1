#ifndef ENGINE_H_
#define ENGINE_H_

#include <memory>
#include <glm/glm.hpp>

#include "window/window.h"
#include "window/events/window.h"

class Engine {
private:
    std::unique_ptr<Window> m_window;
    bool m_isRunning;
private:
    glm::mat4 m_projection;
public:
    Engine();
    ~Engine();
private:
    void Stop();
    void Destroy() const;
    [[nodiscard]] bool Running() const;
private:
    void HandleWindowResized(const WindowResized& event);
public:
    void Run();
};


#endif // ENGINE_H_