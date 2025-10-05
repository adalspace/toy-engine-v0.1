#ifndef ENGINE_H_
#define ENGINE_H_

#include <memory>
#include <glm/glm.hpp>

#include "window/window.h"
#include "window/events/window.h"

#include "app/app.h"

class Engine {
public:
    static void Run(std::unique_ptr<IApplication> app);
private:
    void HandleWindowResized(const WindowResized& event);
private:
    static std::unique_ptr<IApplication> s_app;
    static std::shared_ptr<Window> s_window;
    static bool s_running;
};


#endif // ENGINE_H_