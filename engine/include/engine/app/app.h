#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "engine/window/events/window.h"

class IApplication {
public:
    virtual ~IApplication() = default;

    virtual void OnInit() {};
    virtual void OnUpdate() {};
    virtual void OnRender() {};
    virtual void OnShutdown() {};
    
    virtual void OnEvent() {};
    virtual void OnWindowResized(const WindowResized& e) {};
};

#endif // APPLICATION_H_