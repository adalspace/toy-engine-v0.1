#ifndef MODEL_H_
#define MODEL_H_
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>

#include "shader.h"
#include "texture.h"
#include "renderer/material.h"
#include "renderer/basics.h"

enum ObjElement { OHASH, MTLLIB, USEMTL, O, V, VN, VT, F, OUNKNOWN };
enum MtlElement { MHASH, NEWMTL, NS, KA, KS, KD, NI, D, ILLUM, MAP_KD, MAP_KA, MUNKNOWN };

class Mesh {
public: // TODO: abstract away
    unsigned int m_vao, m_vbo, m_ebo;
    std::vector<Vertex> m_vertexBuffer;
    std::vector<unsigned int> m_indexBuffer;
public: // TODO: abstract away
    void Bind() { glBindVertexArray(m_vao); }
    void Unbind() { glBindVertexArray(0); }
    void Upload();
public:
    std::string materialName;
public:
    Mesh();
public:
    void Render();
};

class Object {
private:
    std::string m_name;
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;

    std::vector<Mesh> m_meshes;

    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
private:
    static inline int NormalizeIndex(const std::string &s, int baseCount);

private:
    Object();
public:
    static Object LoadFile(const std::string& filename);

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
};

#endif // MODEL_H_