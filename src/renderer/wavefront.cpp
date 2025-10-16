#include <iostream>
#include <fstream>
#include <cstring>
#include <memory>
#include <filesystem>
#include <GL/glew.h>

#include "IO/parser.h"
#include "renderer/mesh.h"
#include "renderer/wavefront.h"

#define DEFAULT_MATERIAL_NAME "default"

// ObjElement toElement(const std::string &s) {
//     if (s == "#") return ObjElement::OHASH;
//     if (s == "mtllib") return ObjElement::MTLLIB;
//     if (s == "usemtl") return ObjElement::USEMTL;
//     if (s == "o") return ObjElement::O;
//     if (s == "v") return ObjElement::V;
//     if (s == "vn") return ObjElement::VN;
//     if (s == "vt") return ObjElement::VT;
//     if (s == "f") return ObjElement::F;
//     return ObjElement::OUNKNOWN;
// }

inline ObjElement toElement(const char* s) {
    switch (s[0]) {
        case '#': return ObjElement::OHASH;
        case 'm': if (strcmp(s, "mtllib") == 0) return ObjElement::MTLLIB; break;
        case 'u': if (strcmp(s, "usemtl") == 0) return ObjElement::USEMTL; break;
        case 'o': if (s[1] == '\0') return ObjElement::O; break;
        case 'v':
            if (s[1] == '\0') return ObjElement::V;
            if (s[1] == 'n' && s[2] == '\0') return ObjElement::VN;
            if (s[1] == 't' && s[2] == '\0') return ObjElement::VT;
            break;
        case 'f': if (s[1] == '\0') return ObjElement::F; break;
    }
    return ObjElement::OUNKNOWN;
}

// MtlElement toMtlElement(const std::string &s) {
//     if (s == "#") return MtlElement::MHASH;
//     if (s == "newmtl") return MtlElement::NEWMTL;
//     if (s == "Ns") return MtlElement::NS;
//     if (s == "Ka") return MtlElement::KA;
//     if (s == "Ks") return MtlElement::KS;
//     if (s == "Kd") return MtlElement::KD;
//     if (s == "Ni") return MtlElement::NI;
//     if (s == "d") return MtlElement::D;
//     if (s == "illum") return MtlElement::ILLUM;
//     if (s == "map_Kd") return MtlElement::MAP_KD;
//     if (s == "map_Ka") return MtlElement::MAP_KA;
//     // if (s == "map_Ke") return MtlElement::MAP_KE;
//     return MtlElement::MUNKNOWN;
// }

inline MtlElement toMtlElement(const char* s) {
    switch (s[0]) {
        case '#': return MtlElement::MHASH;
        case 'n':
            if (strcmp(s, "newmtl") == 0) return MtlElement::NEWMTL;
            break;
        case 'N':
            if (s[1] == 's' && s[2] == '\0') return MtlElement::NS;
            if (s[1] == 'i' && s[2] == '\0') return MtlElement::NI;
            break;
        case 'K':
            if (s[1] == 'a' && s[2] == '\0') return MtlElement::KA;
            if (s[1] == 's' && s[2] == '\0') return MtlElement::KS;
            if (s[1] == 'd' && s[2] == '\0') return MtlElement::KD;
            break;
        case 'd':
            if (s[1] == '\0') return MtlElement::D;
            break;
        case 'i':
            if (strcmp(s, "illum") == 0) return MtlElement::ILLUM;
            break;
        case 'm':
            if (strcmp(s, "map_Kd") == 0) return MtlElement::MAP_KD;
            if (strcmp(s, "map_Ka") == 0) return MtlElement::MAP_KA;
            // if (strcmp(s, "map_Ke") == 0) return MtlElement::MAP_KE;
            break;
    }
    return MtlElement::MUNKNOWN;
}

inline int Object::NormalizeIndex(int idx, int baseCount) {
    // idx is the raw value returned by parser:
    //  0 -> means "not present" or invalid in our convention
    // >0 -> 1-based index -> convert to 0-based
    // <0 -> negative index -> relative to baseCount: baseCount + idx
    if (idx == 0) return -1;          // absent / invalid
    if (idx > 0) return idx - 1;      // 1-based -> 0-based
    return baseCount + idx;           // negative -> count from end
}

Object::Object() {
    m_vertices = std::vector<glm::vec3>();
    m_normals = std::vector<glm::vec3>();
    m_texCoords = std::vector<glm::vec2>();
}

void Object::LoadMaterials(const std::filesystem::path& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << filename << std::endl;
        return;
    }

    std::string currentMaterialName;
    std::shared_ptr<Material> currentMaterial;

    char line[1024]; // buffer per line

    while (file.getline(line, sizeof(line))) {
        Parser p(line);
        char* prefix = p.TakeWord();
        if (!prefix) continue;

        switch (toMtlElement(prefix)) {
        case MtlElement::MHASH: // comment
            continue;

        case MtlElement::NEWMTL:
        {
            // If a material was being built, commit it first
            if (currentMaterial) {
                AddMaterial(currentMaterialName, std::move(currentMaterial));
                currentMaterial = nullptr;
            }

            char* materialName = p.TakeWord();
            if (materialName) {
                currentMaterialName = materialName;
                currentMaterial = std::make_shared<Material>();
            }
            break;
        }

        case MtlElement::NS: // specular weight
        {
            float weight = p.TakeFloat();
            if (currentMaterial) currentMaterial->SetSpecularWeight(weight);
            break;
        }

        case MtlElement::KA: // ambient color
        {
            float r = p.TakeFloat();
            float g = p.TakeFloat();
            float b = p.TakeFloat();
            if (currentMaterial) currentMaterial->SetAmbientColor(glm::vec3(r, g, b));
            break;
        }

        case MtlElement::KS: // specular color
        {
            float r = p.TakeFloat();
            float g = p.TakeFloat();
            float b = p.TakeFloat();
            if (currentMaterial) currentMaterial->SetSpecularColor(glm::vec3(r, g, b));
            break;
        }

        case MtlElement::KD: // diffuse color
        {
            float r = p.TakeFloat();
            float g = p.TakeFloat();
            float b = p.TakeFloat();
            if (currentMaterial) currentMaterial->SetDiffuseColor(glm::vec3(r, g, b));
            break;
        }

        case MtlElement::D: // opacity
        {
            float d = p.TakeFloat();
            if (currentMaterial) currentMaterial->SetOpacity(d);
            break;
        }

        case MtlElement::ILLUM: // illumination model
        {
            int illum = p.TakeInt();
            if (currentMaterial) currentMaterial->SetIllumination(illum);
            break;
        }

        case MtlElement::MAP_KD: // diffuse texture map
        {
            // take rest of line as texture path (can contain spaces)
            char* texPath = p.TakeUntil('\0');
            if (texPath && currentMaterial) {
                // trim trailing spaces
                size_t len = std::strlen(texPath);
                while (len > 0 && (texPath[len - 1] == ' ' || texPath[len - 1] == '\t'))
                    texPath[--len] = '\0';

                currentMaterial->SetDiffuseTexture(Texture::LoadFile(texPath));
            }
            break;
        }

        case MtlElement::MAP_KA: // ambient texture map
        {
            char* texPath = p.TakeUntil('\0');
            if (texPath && currentMaterial) {
                size_t len = std::strlen(texPath);
                while (len > 0 && (texPath[len - 1] == ' ' || texPath[len - 1] == '\t'))
                    texPath[--len] = '\0';

                // optional: handle ambient texture
                // currentMaterial->SetAmbientTexture(Texture::LoadFile(texPath));
            }
            break;
        }

        default:
            // ignore unknown tokens
            break;
        }
    }

    // Commit last material if pending
    if (currentMaterial) {
        AddMaterial(currentMaterialName, std::move(currentMaterial));
    }

    file.close();
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
        AddMaterial(DEFAULT_MATERIAL_NAME, std::move(material));
        CreateNewMesh(DEFAULT_MATERIAL_NAME);
    }
    return m_meshes.back();
}

Object* Object::LoadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filename << std::endl;
        return {};
    }

    Object* obj = new Object();
    char line[1024]; // static buffer for each line (enough for OBJ lines)

    while (file.getline(line, sizeof(line))) {
        Parser p(line);
        char* prefix = p.TakeWord();
        if (!prefix) continue;

        switch (toElement(prefix)) {
        case ObjElement::OHASH: // comment
            continue;

        case ObjElement::MTLLIB:
        {
            char* mtlFile = p.TakeWord();
            if (mtlFile) {
                std::filesystem::path fullPath = filename;
                std::filesystem::path mtlPath = fullPath.replace_filename(mtlFile);
                obj->LoadMaterials(mtlPath);
            }
            break;
        }

        case ObjElement::USEMTL:
        {
            char* materialName = p.TakeWord();
            if (materialName) {
                auto& mesh = obj->GetLastMesh();
                if (mesh.materialName != materialName) {
                    Mesh newMesh;
                    newMesh.materialName = materialName;
                    obj->m_meshes.push_back(newMesh);
                }
            }
            break;
        }

        case ObjElement::O: // object name
        {
            char* name = p.TakeWord();
            if (name) obj->m_name = name;
            break;
        }

        case ObjElement::V: // vertex
        {
            float x = p.TakeFloat();
            float y = p.TakeFloat();
            float z = p.TakeFloat();
            float w = p.TakeFloat();

            if (w != 0.0f && w != 1.0f) {
                x /= w; y /= w; z /= w;
            }
            obj->m_vertices.emplace_back(x, y, z);
            break;
        }

        case ObjElement::VN: // normal
        {
            float x = p.TakeFloat();
            float y = p.TakeFloat();
            float z = p.TakeFloat();
            obj->m_normals.emplace_back(x, y, z);
            break;
        }

        case ObjElement::VT: // texcoord
        {
            float u = p.TakeFloat();
            float v = p.TakeFloat();
            obj->m_texCoords.emplace_back(u, 1.0f - v);
            break;
        }

        case ObjElement::F: // face
        {
            auto& mesh = obj->GetLastMesh();
            int raw_vi, raw_ti, raw_ni;

            while (p.TakeFaceIndices(raw_vi, raw_ti, raw_ni)) {
                // Convert raw OBJ indices to 0-based / -1 sentinel
                int vi = Object::NormalizeIndex(raw_vi, (int)obj->m_vertices.size());
                int ti = Object::NormalizeIndex(raw_ti, (int)obj->m_texCoords.size());
                int ni = Object::NormalizeIndex(raw_ni, (int)obj->m_normals.size());

                if (vi < 0) {
                    // malformed token (no vertex) — skip
                    continue;
                }

                glm::vec3 vert = obj->m_vertices[vi];
                glm::vec3 norm(0.0f);
                glm::vec2 texCoord(0.0f);

                if (ni >= 0) norm = obj->m_normals[ni];
                if (ti >= 0) texCoord = obj->m_texCoords[ti];

                mesh.m_vertexBuffer.emplace_back(vert, norm, texCoord);
                mesh.m_indexBuffer.push_back(mesh.m_vertexBuffer.size() - 1);
            }
            break;
        }

        default:
            // ignore unknown tokens
            break;
        }
    }

    std::cout << "Object name: " << obj->m_name << std::endl;
    std::cout << "Vertices count: " << obj->m_vertices.size() << std::endl;
    std::cout << "Normals count: " << obj->m_normals.size() << std::endl;
    std::cout << "TexCoords count: " << obj->m_texCoords.size() << std::endl;
    std::cout << "Meshes count: " << obj->m_meshes.size() << std::endl;
    std::cout << "Materials count: " << obj->m_materials.size() << std::endl;

    file.close();

    for (auto &mesh : obj->m_meshes) {
        mesh.Upload();
    }

    return obj;
}

void Object::EnableBatch(unsigned int instanceVBO) {
    for (auto &mesh : m_meshes) {
        mesh.Bind();

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        std::size_t vec4Size = sizeof(glm::vec4);
        for (int i = 0; i < 4; ++i) {
            glEnableVertexAttribArray(3 + i); // use locations 3,4,5,6 for instance matrix
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
                                sizeof(glm::mat4), (void*)(i * vec4Size));
            glVertexAttribDivisor(3 + i, 1); // IMPORTANT: one per instance, not per vertex
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        mesh.Unbind();
    }
}


// void Object::Render(Shader& shader)
// {
//     for (auto &mesh : m_meshes) {
//         auto material = GetMaterial(mesh.materialName);
        
//         shader.setFloat("ambientStrength", 0.2f);
//         shader.setFloat("shininess", material->GetSpecularWeight());
//         shader.setFloat("opacity", material->GetOpacity());
//         shader.setBool("useSpecular", material->GetIllumination() >= 2);
//         shader.setFloat("specularStrength", 1.0f);
//         shader.setVec3("ambientColor", material->GetAmbientColor());
//         shader.setVec3("diffuseColor", material->GetDiffuseColor());
//         shader.setVec3("specularColor", material->GetSpecularColor());

//         if (material->HasDiffuseTexture()) {
//             shader.setBool("useTexture", true);
//             glActiveTexture(GL_TEXTURE0);
//             glBindTexture(GL_TEXTURE_2D, material->GetDiffuseTexture()->GetID());
//             shader.setInt("diffuseTex", 0);
//         } else {
//             shader.setBool("useTexture", false);
//         }

//         mesh.Render();
//     }
// }

void Object::Render(Shader& shader, unsigned int count)
{
    for (auto &mesh : m_meshes)
    {
        auto material = GetMaterial(mesh.materialName);

        // --- Basic material properties ---
        shader.setFloat("opacity", material->GetOpacity());

        // Albedo (base color)
        shader.setVec3("albedo", material->GetDiffuseColor());

        // Metallic and roughness (defaults)
        shader.setFloat("metallic", 0.8f);
        shader.setFloat("roughness", 0.5f);
        shader.setFloat("ao", 1.0f); // default ambient occlusion if none

        // --- Optional textures ---
        int texUnit = 0;

        // Albedo texture
        if (material->HasDiffuseTexture()) {
            shader.setBool("useAlbedoMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            glBindTexture(GL_TEXTURE_2D, material->GetDiffuseTexture()->GetID());
            shader.setInt("albedoTex", texUnit++);
        } else {
            shader.setBool("useAlbedoMap", false);
        }

        // Metallic texture
        // if (material->HasMetallicTexture()) {
        if (false) {
            shader.setBool("useMetallicMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            // glBindTexture(GL_TEXTURE_2D, material->GetMetallicTexture()->GetID());
            shader.setInt("metallicTex", texUnit++);
        } else {
            shader.setBool("useMetallicMap", false);
        }

        // Roughness texture
        // if (material->HasRoughnessTexture()) {
        if (false) {
            shader.setBool("useRoughnessMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            // glBindTexture(GL_TEXTURE_2D, material->GetRoughnessTexture()->GetID());
            shader.setInt("roughnessTex", texUnit++);
        } else {
            shader.setBool("useRoughnessMap", false);
        }

        // AO texture
        // if (material->HasAoTexture()) {
        if (false) {
            shader.setBool("useAoMap", true);
            glActiveTexture(GL_TEXTURE0 + texUnit);
            // glBindTexture(GL_TEXTURE_2D, material->GetAoTexture()->GetID());
            shader.setInt("aoTex", texUnit++);
        } else {
            shader.setBool("useAoMap", false);
        }

        // --- Render mesh ---
        mesh.Render(count);
    }
}


