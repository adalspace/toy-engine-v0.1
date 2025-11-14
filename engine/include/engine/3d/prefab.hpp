#ifndef CORE_PREFAB_H_
#define CORE_PREFAB_H_

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#ifdef _WIN32
#include <corecrt_math_defines.h>
#endif
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "engine/opengl/buffers.h"
#include "engine/renderer/renderable.hpp"
#include "engine/scene/scene.h"
#include "engine/3d/array.hpp"
#include "engine/3d/mesh.hpp"

#include "engine/components/transform.h"
#include "engine/components/batch.h"

namespace Core {

class Prefab : public Renderable {
public:
    Prefab(MeshGroup&& mesh) : m_mesh(std::move(mesh)), m_id(++LastID) {}

    const unsigned int GetID() const { return m_id; }

public:
    void Prepare() override {
        std::cout << "[PREFAB] Prepare called" << std::endl;
        
        EnsureResources();

        m_mesh.Prepare();

        for (auto it = m_mesh.Begin(); it != m_mesh.End(); ++it) {
            it->Bind();

            std::cout << "[PREFAB] Configuring instance buffer for mesh" << std::endl;
            m_instanceBuffer->StartConfigure();
                std::size_t vec4Size = sizeof(glm::vec4);
                for (int i = 0; i < 4; ++i) {
                    glEnableVertexAttribArray(3 + i); // use locations 3,4,5,6 for instance matrix
                    glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
                                        sizeof(glm::mat4), (void*)(i * vec4Size));
                    glVertexAttribDivisor(3 + i, 1); // IMPORTANT: one per instance, not per vertex
                }
            m_instanceBuffer->EndConfigure();
            std::cout << "[PREFAB] Finished configuring" << std::endl;
            
            it->Unbind();
        }
    }

    void Render(Shader& shader, Scene& scene, unsigned int count) override {
        std::cout << "[PREFAB] Render called" << std::endl;
        Array<entt::entity> batches;

        for (auto [entt, item] : scene.m_registry.view<batch::item>().each()) {
            if (item.batchId == m_id) {
                batches.PushBack(entt);
            }
        }

        std::cout << "[PREFAB] Collected " << batches.GetSize() << " batch items" << std::endl;

        std::vector<glm::mat4> models;
        models.reserve(batches.GetSize());

        std::cout << "[PREFAB] Starting collecting models..." << std::endl;
        
        for (auto it = batches.Begin(); it != batches.End(); ++it) {
            auto &t = scene.m_registry.get<Transform>(*it);
            glm::mat4 rotation = glm::yawPitchRoll(t.rotation.y, t.rotation.x, t.rotation.z);
            auto itemModel = glm::translate(glm::mat4(1.f), t.position) * rotation;
            models.push_back(itemModel);
        }

        std::cout << "[PREFAB] Collected " << models.size() << " models" << std::endl;

        UploadInstances(models.data(), models.size());
        
        shader.setBool("u_isInstanced", true);
        shader.setBool("isLight", false);
        shader.setVec3("currentLightColor", glm::vec3(0.f));
        
        std::cout << "[PREFAB] Rendering mesh in instanced mode with size = " << models.size() << std::endl;
        m_mesh.Render(shader, scene, models.size());

        shader.setBool("u_isInstanced", false);
    }

private:
    void EnsureResources(unsigned int count = 0) {
        if (!m_instanceBuffer) {
            std::cout << "[PREFAB] Instance buffer init..." << std::endl;
            m_instanceBuffer = new OpenGL::InstanceBuffer(GL_DYNAMIC_DRAW);
            OpenGL::Buffer::Bind(m_instanceBuffer);
            OpenGL::Buffer::Data(m_instanceBuffer, nullptr, sizeof(glm::mat4) * count);
            OpenGL::Buffer::Unbind(m_instanceBuffer);
            m_instance_count = count;
        }
    }

    void UploadInstances(glm::mat4 *instances, unsigned int count) {
        std::cout << "[PREFAB] UploadInstances called" << std::endl;

        EnsureResources(count);

        if (count > m_instance_count) {
            std::cout << "[PREFAB] Reallocate buffer. Current = " << m_instance_count << " ; required = " << count << std::endl;
            // Optional: reallocate only if you *really* have more instances than before
            // FIXME: what the hell is m_instance_vbo
            // glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
            OpenGL::Buffer::Bind(m_instanceBuffer);
            OpenGL::Buffer::Data(m_instanceBuffer, nullptr, sizeof(glm::mat4) * count);
            OpenGL::Buffer::Unbind(m_instanceBuffer);
            m_instance_count = count;
        }

        // Just update the data region — much cheaper
        std::cout << "[PREFAB] Updating data of instance buffer..." << std::endl;
        // std::cout << "[PREFAB] buffer target " << (m_instanceBuffer->GetTarget() == GL_ARRAY_BUFFER ? "array buffer" : "other") << std::endl;
        // std::cout << "[PREFAB] buffer id " << m_instanceBuffer->GetID() << std::endl;
        std::cout << "count = " << count << std::endl;
        std::cout << "instances = " << instances << std::endl;
        OpenGL::Buffer::Bind(m_instanceBuffer);
        OpenGL::Buffer::SubData(m_instanceBuffer, instances, sizeof(glm::mat4) * count, 0);
    }

protected:
    static unsigned int LastID;

private:
    unsigned int m_id;
    MeshGroup m_mesh;
    OpenGL::InstanceBuffer* m_instanceBuffer = nullptr;
    unsigned int m_instance_count = 0;
    unsigned int m_instance_vbo = 0;
};

unsigned int Prefab::LastID = 0;

}

#endif // CORE_PREFAB_H_