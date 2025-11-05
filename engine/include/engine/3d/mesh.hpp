#pragma once

#include "engine/3d/array.hpp"
#include "engine/3d/vertex.hpp"
#include "engine/3d/material.hpp"
#include "engine/opengl/buffers.h"
#include "engine/renderer/renderable.hpp"

#include "engine/export.h"

namespace Core {

class MeshGroup;

class ENGINE_API Mesh : public Renderable, public OpenGL::VertexArray {
    friend class Core::MeshGroup;
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

    const auto GetVerticesCount() const { return m_vertices.GetSize(); }
    const auto GetIndicesCount() const { return m_indices.GetSize(); }
public:
    inline const Material& GetMaterial() const { return m_material; }
private:
    void Prepare() override {
        // ---------- INIT   -----------
        m_ebo = 0;
    
        // glGenVertexArrays(1, &m_vao);
        // glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        Bind();

        SetupVertexBuffer(GL_DYNAMIC_DRAW);

        // EBO (index buffer)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        // attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, normal)));
        glEnableVertexAttribArray(1);
        
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, uv)));
        glEnableVertexAttribArray(2);

        // TODO: delete after ebo moved in VertexArray
        // glBindBuffer(GL_DYNAMIC_DRAW, 0);
        Unbind();

        // ---------- UPLOAD -----------
        Bind();

        VertexBufferData(m_vertices.GetSize() * sizeof(Vertex), m_vertices.Begin());

        // Upload indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.GetSize() * sizeof(unsigned int), m_indices.Begin(), GL_DYNAMIC_DRAW);

        // TODO: delete after ebo moved in VertexArray
        // glBindBuffer(GL_DYNAMIC_DRAW, 0);
        Unbind();
    }

    void Render(Shader& shader) override {
        // --- Basic material properties ---
        shader.setFloat("opacity", m_material.GetOpacity());

        // Albedo (base color)
        shader.setVec3("albedo", m_material.GetDiffuseColor());

        // Metallic and roughness (defaults)
        shader.setFloat("metallic", 0.8f);
        shader.setFloat("roughness", 0.5f);
        shader.setFloat("ao", 1.0f); // default ambient occlusion if none

        // --- Optional textures ---
        int texUnit = 0;

        // Albedo texture
        if (m_material.HasDiffuseTexture()) {
            shader.setBool("useAlbedoMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            glBindTexture(GL_TEXTURE_2D, m_material.GetDiffuseTexture()->GetID());
            shader.setInt("albedoTex", texUnit++);
        } else {
            shader.setBool("useAlbedoMap", false);
        }

        // Metallic texture
        // if (m_material.HasMetallicTexture()) {
        if (false) {
            shader.setBool("useMetallicMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            // glBindTexture(GL_TEXTURE_2D, m_material.GetMetallicTexture()->GetID());
            shader.setInt("metallicTex", texUnit++);
        } else {
            shader.setBool("useMetallicMap", false);
        }

        // Roughness texture
        // if (m_material.HasRoughnessTexture()) {
        if (false) {
            shader.setBool("useRoughnessMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            // glBindTexture(GL_TEXTURE_2D, m_material.GetRoughnessTexture()->GetID());
            shader.setInt("roughnessTex", texUnit++);
        } else {
            shader.setBool("useRoughnessMap", false);
        }

        // AO texture
        // if (m_material.HasAoTexture()) {
        if (false) {
            shader.setBool("useAoMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            // glBindTexture(GL_TEXTURE_2D, m_material.GetAoTexture()->GetID());
            shader.setInt("aoTex", texUnit++);
        } else {
            shader.setBool("useAoMap", false);
        }

        // --- Render mesh ---
        Bind();
        // TODO: support batch render
        // if (count > 1) {
        //     glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_INT, 0, count);
        // } else {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.GetSize()), GL_UNSIGNED_INT, 0);
        // }
        Unbind();
    }
private:
    Array<Vertex> m_vertices { VERTICES_INITIAL_CAPACITY };
    Array<uint32_t> m_indices { INDICES_INITIAL_CAPACITY };

    // TODO: move out
    // ------- RENDERING ---------
    unsigned int m_ebo;
    // ------- RENDERING ---------

    Material m_material;

    std::unordered_map<Vertex, uint32_t, VertexHash> m_vti;
}; // class Mesh

// Right now it's just a list of meshes connected to each other.
// In future we might want to add support for global material
// that can affect all sub materials, aka this class
// will act like Parent Mesh that contains Child Meshes
class ENGINE_API MeshGroup : public Array<Mesh>, public Renderable {
public:
    MeshGroup() {}

    void Prepare() override {
        for (auto it = Begin(); it != End(); ++it) {
            it->Prepare();
        }
    }

    void Render(Shader& shader) {
        for (auto it = Begin(); it != End(); ++it) {
            it->Render(shader);
        }
    }

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