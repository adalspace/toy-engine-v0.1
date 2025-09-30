#ifndef MODEL_H_
#define MODEL_H_
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>

#include "shader.h"
#include "texture.h"

enum ObjElement { OHASH, MTLLIB, USEMTL, O, V, VN, VT, F, OUNKNOWN };
enum MtlElement { MHASH, NEWMTL, NS, KA, KS, KD, NI, D, ILLUM, MAP_KD, MAP_KA, MUNKNOWN };

class Vertex {
private:
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec2 m_texCoord;
public:
    Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoord)
        : m_position(position), m_normal(normal), m_texCoord(texCoord) {}
public:
    static void DefineAttrib();
};

class FaceItem {
private:
    unsigned int m_vIndex;
    unsigned int m_nIndex;
    unsigned int m_tIndex;
public:
    FaceItem()
        : m_vIndex(0), m_nIndex(0), m_tIndex(0) {}
    FaceItem(unsigned int vI, unsigned int nI, unsigned int tI)
        : m_vIndex(vI), m_nIndex(nI), m_tIndex(tI) {}
public:
    inline const unsigned int GetVertex() const { return m_vIndex; }
    inline const unsigned int GetNormal() const { return m_nIndex; }
    inline const unsigned int GetTex() const { return m_tIndex; }
public:
    inline void SetVertex(unsigned int vIndex) { m_vIndex = vIndex; }
    inline void SetNorm(unsigned int nIndex) { m_nIndex = nIndex; }
    inline void SetTex(unsigned int tIndex) { m_tIndex = tIndex; }
};

class Face {
private:
    std::vector<FaceItem> m_items;
public:
    Face()
        : m_items(std::vector<FaceItem>()) {}
public:
    void PushItem(const FaceItem& item);
public:
    inline const unsigned int GetSize() const { return m_items.size(); }
    inline const FaceItem& GetItem(unsigned int index) const { return m_items[index]; }
};

class Material {
private:
    glm::vec3 m_ambient { 0.2f, 0.2f, 0.2f };
    glm::vec3 m_diffuse { 0.8f, 0.8f, 0.8f };
    glm::vec3 m_specular { 1.0f, 1.0f, 1.0f };
    float m_shininess { 32.0f };

    std::unique_ptr<Texture> m_diffuse_tex { nullptr };
public:
    Material() = default;
    Material(const Material& other) = default; // copy constructor
    Material& operator=(const Material& other) = default;
public:
    inline const glm::vec3 GetAmbientColor() const { return m_ambient; }
    inline const glm::vec3 GetDiffuseColor() const { return m_diffuse; }
    inline const glm::vec3 GetSpecularColor() const { return m_specular; }
    inline const float GetSpecularWeight() const { return m_shininess; }
    inline const bool HasDiffuseTexture() const { return m_diffuse_tex != nullptr; }
    inline const Texture* GetDiffuseTexture() const { return m_diffuse_tex.get(); }
public:
    inline void SetAmbientColor(glm::vec3 ambient) { m_ambient = ambient; }
    inline void SetDiffuseColor(glm::vec3 diffuse) { m_diffuse = diffuse; }
    inline void SetSpecularColor(glm::vec3 specular) { m_specular = specular; }
    inline void SetSpecularWeight(float weight) { m_shininess = weight; }
    inline void SetDiffuseTexture(std::unique_ptr<Texture>&& texture) { m_diffuse_tex = std::move(texture); }
};

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
    void LoadMaterials(const std::string& filename);
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