#ifndef COMPONENTS_MESH_H_
#define COMPONENTS_MESH_H_

#include <memory>
#include "engine/renderer/renderable.h"

#include "engine/export.h"

namespace Core {
struct ENGINE_API mesh {
    std::shared_ptr<Renderable> mesh;
};
}

#endif // COMPONENTS_MESH_H_