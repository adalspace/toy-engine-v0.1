#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "engine/scene/scene.h"
#include "engine/window/event.hpp"
#include "engine/export.h"

class ENGINE_API IApplication {
public:
    virtual ~IApplication() = default;

    virtual void OnInit(std::shared_ptr<Scene> scene) {};
    virtual void OnUpdate() {};
    virtual void OnShutdown() {};
    
    virtual void OnEvent(const Event& event) {};
};

#endif // APPLICATION_H_