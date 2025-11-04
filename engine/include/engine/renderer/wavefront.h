#ifndef MODEL_H_
#define MODEL_H_
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>

#include "engine/renderer/shader.h"
#include "engine/renderer/renderer.h"
#include "engine/3d/mesh.hpp"
#include "engine/opengl/buffers.h"

#include "engine/export.h"

namespace Core {

enum ObjElement { OHASH, MTLLIB, USEMTL, O, V, VN, VT, F, OUNKNOWN };
enum MtlElement { MHASH, NEWMTL, NS, KA, KS, KD, NI, D, ILLUM, MAP_KD, MAP_KA, MUNKNOWN };

class ENGINE_API Object : public MeshGroup {
    friend class Renderer;
private:
    static inline int NormalizeIndex(int idx, int baseCount);

    Object();

public:
    ~Object() = default;

public:
    static Object* LoadFile(const std::string& filename);

private:
    void LoadMTL(const std::filesystem::path& filename);

private:
    void CreateNewMesh();
    void CreateNewMesh(const Material& material);
    Mesh& GetLastMesh();

    void AddMaterial(MaterialID id, const Material& material);
    Material* GetMaterial(const MaterialID& id);

public:
    void Render(Shader& shader, unsigned int count);
    [[nodiscard]] inline const std::string Name() const { return m_name; }

protected:
    void EnableBatch(const OpenGL::InstanceBuffer* instanceBuffer);

private:
    std::string m_name;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;

    std::unordered_map<MaterialID, Material> m_materials;
};

}

#endif // MODEL_H_