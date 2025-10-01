#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <filesystem>
#include <GL/glew.h>

#include "model.h"

ObjElement toElement(const std::string &s) {
    if (s == "#") return ObjElement::OHASH;
    if (s == "mtllib") return ObjElement::MTLLIB;
    if (s == "usemtl") return ObjElement::USEMTL;
    if (s == "o") return ObjElement::O;
    if (s == "v") return ObjElement::V;
    if (s == "vn") return ObjElement::VN;
    if (s == "vt") return ObjElement::VT;
    if (s == "f") return ObjElement::F;
    return ObjElement::OUNKNOWN;
}

MtlElement toMtlElement(const std::string &s) {
    if (s == "#") return MtlElement::MHASH;
    if (s == "newmtl") return MtlElement::NEWMTL;
    if (s == "Ns") return MtlElement::NS;
    if (s == "Ka") return MtlElement::KA;
    if (s == "Ks") return MtlElement::KS;
    if (s == "Kd") return MtlElement::KD;
    if (s == "Ni") return MtlElement::NI;
    if (s == "d") return MtlElement::D;
    if (s == "illum") return MtlElement::ILLUM;
    if (s == "map_Kd") return MtlElement::MAP_KD;
    if (s == "map_Ka") return MtlElement::MAP_KA;
    // if (s == "map_Ke") return MtlElement::MAP_KE;
    return MtlElement::MUNKNOWN;
}

void Vertex::DefineAttrib()
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, m_position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, m_normal)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, m_texCoord)));
    glEnableVertexAttribArray(2);
}

void Face::PushItem(const FaceItem& item)
{
    m_items.push_back(item);
}

inline int Object::NormalizeIndex(const std::string &s, int baseCount) {
    if (s.empty()) return -1;
    int idx = std::stoi(s);
    if (idx > 0) return idx - 1;
    return baseCount + idx;
}

Mesh::Mesh() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // VBO (vertex buffer)
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    // EBO (index buffer)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    Vertex::DefineAttrib();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Object::Object() {
    m_vertices = std::vector<glm::vec3>();
    m_normals = std::vector<glm::vec3>();
    m_texCoords = std::vector<glm::vec2>();
}

void Object::LoadMaterials(const std::filesystem::path& filename) {
    std::ifstream file(filename);

    std::string currentMaterialName;
    std::shared_ptr<Material> currentMaterial;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        switch(toMtlElement(prefix)) {
        case MtlElement::MHASH:
        {
            std::cout << "comment: " << line << std::endl;
            continue;
        }
        case MtlElement::NEWMTL:
        {
            if (currentMaterial) {
                m_materials.insert(std::make_pair(currentMaterialName, std::move(currentMaterial)));
                currentMaterial = nullptr;
            }
            std::string materialName;
            iss >> materialName;
            currentMaterialName = materialName;
            currentMaterial = std::make_shared<Material>();
            break;
        }
        case MtlElement::NS:
        {
            float weight;
            iss >> weight;
            currentMaterial->SetSpecularWeight(weight);
        }
        case MtlElement::KA:
        {
            float r, g, b;
            iss >> r >> g >> b;
            currentMaterial->SetAmbientColor(glm::vec3(r, g, b));
            break;
        }
        case MtlElement::KS:
        {
            float r, g, b;
            iss >> r >> g >> b;
            currentMaterial->SetSpecularColor(glm::vec3(r, g, b));
            break;
        }
        case MtlElement::KD:
        {
            float r, g, b;
            iss >> r >> g >> b;
            currentMaterial->SetDiffuseColor(glm::vec3(r, g, b));
            break;
        }
        case MtlElement::MAP_KD:
        {
            std::string texturePath;
            std::string part;
            while (iss >> part) {
                texturePath += part + " ";
            }
            texturePath = texturePath.substr(0, texturePath.size() - 1);
            currentMaterial->SetDiffuseTexture(Texture::LoadFile(texturePath));
        }
        }
    }

    if (currentMaterial) {
        // m_materials.insert(std::make_pair(currentMaterialName, std::move(currentMaterial)));
        AddMaterial(currentMaterialName, std::move(currentMaterial));
    }
}

void Object::AddMaterial(std::string name, std::shared_ptr<Material> material)
{
    m_materials.insert(std::make_pair(std::move(name), std::move(material)));
}

std::shared_ptr<Material> Object::GetMaterial(std::string name)
{
    auto material = m_materials.find(name);
    if (material == m_materials.end()) return nullptr;
    return material->second;
}

void Object::CreateNewMesh(const std::string& materialName)
{
    Mesh mesh;
    mesh.materialName = materialName;
    m_meshes.push_back(mesh);
}

Mesh& Object::GetLastMesh()
{
    if (m_meshes.empty()) {
        auto material = std::make_shared<Material>();
        material->SetAmbientColor(glm::vec3(0.52f, 0.52f, 0.52f));
        // TODO: come up with name for a default material
        AddMaterial("", std::move(material));
        CreateNewMesh("");
    }
    return m_meshes.back();
}

Object Object::LoadFile(const std::string& filename) {
    std::ifstream file(filename);

    Object obj;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        switch(toElement(prefix)) {
        // comment
        case ObjElement::OHASH:
        {
            std::cout << "comment: " << line << std::endl;
            continue;
        }
        case ObjElement::MTLLIB:
        {
            std::string mtlFile;
            iss >> mtlFile;
            std::filesystem::path fullPath = filename;
            std::filesystem::path mtlPath = fullPath.replace_filename(mtlFile);
            obj.LoadMaterials(mtlPath);
            std::cout << "loaded mtl at '" << mtlPath << "' with "
                << obj.m_materials.size() << " materials" << std::endl;
            break;
        }
        case ObjElement::USEMTL:
        {
            std::string materialName;
            iss >> materialName;
            auto& mesh = obj.GetLastMesh();
            if (mesh.materialName != materialName) {
                Mesh mesh;
                mesh.materialName = materialName;
                obj.m_meshes.push_back(mesh);
            }
            break;
        }
        // object name I suppose
        case ObjElement::O:
        {
            obj.m_name = line.substr(2);
            break;
        }
        // vertex with its position
        case ObjElement::V:
        {
            float x, y, z, w;
            w = 1.0f;
            iss >> x >> y >> z;
            if (iss >> w) {
                x /= w;
                y /= w;
                z /= w;
            }
            obj.m_vertices.emplace_back(x, y, z);
            break;
        }
        case ObjElement::VN:
        {
            float x, y, z;
            iss >> x >> y >> z;
            obj.m_normals.emplace_back(x, y, z);
            break;
        }
        case ObjElement::VT:
        {
            float u, v;
            iss >> u >> v;
            obj.m_texCoords.emplace_back(u, 1.0f - v);
            break;
        }
        case ObjElement::F:
        {
            auto& mesh = obj.GetLastMesh();
            std::string token;
            
            Face fv;

            while (iss >> token) {
                std::string a, b, c;
                std::istringstream ref(token);

                std::getline(ref, a, '/');
                std::getline(ref, b, '/');
                std::getline(ref, c, '/');

                int vi = Object::NormalizeIndex(a, (int)obj.m_vertices.size());
                int ti = Object::NormalizeIndex(b, (int)obj.m_texCoords.size());
                int ni = Object::NormalizeIndex(c, (int)obj.m_normals.size());

                glm::vec3 vert, norm;
                glm::vec2 texCoord;
                vert = obj.m_vertices[vi];
                if (ni >= 0) norm = obj.m_normals[ni];
                if (ti >= 0) texCoord = obj.m_texCoords[ti];

                mesh.m_vertexBuffer.emplace_back(vert, norm, texCoord);
                mesh.m_indexBuffer.push_back(mesh.m_vertexBuffer.size() - 1);
            }
            
            break;
        }
        }
    }

    std::cout << "Object name: " << obj.m_name << std::endl;
    std::cout << "Vertices count: " << obj.m_vertices.size() << std::endl;
    std::cout << "Normals count: " << obj.m_normals.size() << std::endl;
    std::cout << "TexCoords count: " << obj.m_texCoords.size() << std::endl;
    std::cout << "Meshes count: " << obj.m_meshes.size() << std::endl;
    std::cout << "Materials count: " << obj.m_materials.size() << std::endl;
    
    // std::cout << "Vertex Buffer size: " << obj.m_vertexBuffer.size() << std::endl;
    // std::cout << "Index Buffer size: " << obj.m_indexBuffer.size() << std::endl;

    file.close();

    for (auto &mesh : obj.m_meshes) {
        mesh.Upload();
    }

    return obj;
}

void Object::Render(Shader& shader)
{
    for (auto &mesh : m_meshes) {
        auto material = GetMaterial(mesh.materialName);
        
        shader.setFloat("ambientStrength", 0.2f);
        shader.setFloat("specularStrength", material->GetSpecularWeight());
        shader.setVec3("ambientColor", material->GetAmbientColor());
        shader.setVec3("diffuseColor", material->GetDiffuseColor());
        shader.setVec3("specularColor", material->GetSpecularColor());

        if (material->HasDiffuseTexture()) {
            shader.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material->GetDiffuseTexture()->GetID());
            shader.setInt("diffuseTex", 0);
        } else {
            shader.setBool("useTexture", false);
        }

        mesh.Render();
    }
}

void Mesh::Upload()
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size() * sizeof(Vertex), m_vertexBuffer.data(), GL_STATIC_DRAW);

    // Upload indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer.size() * sizeof(unsigned int), m_indexBuffer.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::Render()
{
    Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_INT, 0);
    Unbind();
}
