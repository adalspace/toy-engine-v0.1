#ifndef MODEL_H_
#define MODEL_H_
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>

#include "shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

enum ObjElement { OHASH, MTLLIB, USEMTL, O, V, VN, VT, F, OUNKNOWN };
enum MtlElement { MHASH, NEWMTL, NS, KA, KS, KD, NI, D, ILLUM, MAP_KD, MAP_KA, MUNKNOWN };

class Object {
private:
    static inline int NormalizeIndex(int idx, int baseCount);

private:
    Object();
public:
    ~Object() = default;
public:
    static Object* LoadFile(const std::string& filename);

private:
    void LoadMaterials(const std::filesystem::path& filename);
private:
    void AddMaterial(std::string name, std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial(std::string name);
private:
    Mesh& GetLastMesh();
    void CreateNewMesh(const std::string& materialName);
public:
    void Render(Shader& shader);
private:
    std::string m_name;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;

    std::vector<Mesh> m_meshes;

    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};

#endif // MODEL_H_