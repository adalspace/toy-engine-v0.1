#pragma once

#include "engine/3d/array.hpp"
#include "engine/3d/vertex.h"
#include "engine/3d/material.hpp"

#include "engine/export.h"

namespace Core {

class ENGINE_API Mesh {
public:
    Mesh() = default;
    Mesh(const Material& material) : m_material(material) {}

    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&) = default;
public:
    template<typename... Args>
    uint32_t EmplaceVertex(Args&&... args) {
        Vertex v(std::forward<Args>(args)...);
        auto it = m_vti.find(v);
        if (it != m_vti.end()) return it->second;
        uint32_t newIndex = static_cast<uint32_t>(m_vertices.GetSize());
        m_vertices.PushBack(std::move(v));
        m_vti.emplace(m_vertices.Back(), newIndex);
        return newIndex;
    }

    uint32_t PushVertex(const Vertex& vertex) {
        auto it = m_vti.find(vertex);
        if (it != m_vti.end()) return it->second;
        uint32_t newIndex = static_cast<uint32_t>(m_vertices.GetSize());
        m_vertices.PushBack(vertex);
        m_vti.emplace(m_vertices.Back(), newIndex);
        return newIndex;
    }

    void PushTriangle(uint32_t a, uint32_t b, uint32_t c) {
        m_indices.PushBack(a);
        m_indices.PushBack(b);
        m_indices.PushBack(c);
    }
public:
    inline const Material& GetMaterial() const { return m_material; }
private:
    Array<Vertex> m_vertices { VERTICES_INITIAL_CAPACITY };
    Array<uint32_t> m_indices { INDICES_INITIAL_CAPACITY };

    Material m_material;

    std::unordered_map<Vertex, uint32_t, VertexHash> m_vti;
}; // class Mesh

// Right now it's just a list of meshes connected to each other.
// In future we might want to add support for global material
// that can affect all sub materials, aka this class
// will act like Parent Mesh that contains Child Meshes
class ENGINE_API MeshGroup : public Array<Mesh> {
public:
    MeshGroup() {}

public:
    inline Mesh* FindMeshByMaterial(const Material& material) {
        for (auto it = Begin(); it != End(); ++it) {
            if (it->GetMaterial().GetID() == material.GetID()) {
                return it;
            }
        }
        
        return End();
    }

    inline Mesh* FindMeshByMaterial(const Material* material) {
        for (auto it = Begin(); it != End(); ++it) {
            if (it->GetMaterial().GetID() == material->GetID()) {
                return it;
            }
        }
        
        return End();
    }
};

} // namespace Core