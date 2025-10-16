#ifndef COMPONENTS_MESH_H_
#define COMPONENTS_MESH_H_

#include <memory>
#include "engine/renderer/wavefront.h"

struct mesh {
    std::shared_ptr<Object> object;
};

#endif // COMPONENTS_MESH_H_