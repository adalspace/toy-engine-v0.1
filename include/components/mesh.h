#ifndef COMPONENTS_MESH_H_
#define COMPONENTS_MESH_H_

#include <memory>
#include "renderer/wavefront.h"

struct mesh {
    std::unique_ptr<Object> object;
};

#endif // COMPONENTS_MESH_H_