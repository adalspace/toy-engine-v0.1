#ifndef CORE_RENDERABLE_H_
#define CORE_RENDERABLE_H_

#include "engine/renderer/shader.h"

#include "engine/export.h"

namespace Core {

class Renderer;

class ENGINE_API Renderable {
protected:
    Renderable();
    virtual ~Renderable() = default;

private:
    virtual void Prepare() = 0;
    virtual void Render(Shader& shader) = 0;

    friend class Core::Renderer;
};

} // namespace Core

#endif // CORE_RENDERABLE_H_