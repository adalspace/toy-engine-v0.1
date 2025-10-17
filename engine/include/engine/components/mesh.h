#ifndef COMPONENTS_MESH_H_
#define COMPONENTS_MESH_H_

#include <memory>

#include "engine/renderer/wavefront.h"
#include "engine/export.h"

struct ENGINE_API mesh {
    std::shared_ptr<Object> object;
};

#endif // COMPONENTS_MESH_H_