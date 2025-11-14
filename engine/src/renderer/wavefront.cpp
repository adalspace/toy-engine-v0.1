#include <iostream>
#include <fstream>
#include <cstring>
#include <memory>
#include <filesystem>
#include <GL/glew.h>

#include "engine/renderer/wavefront.h"

#include "engine/IO/parser.h"
#include "engine/3d/mesh.hpp"

namespace Core {

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

void Object::AddMaterial(MaterialID id, const Material& material) {
    m_materials.insert(std::make_pair(id, material));
}

void Object::LoadMTL(const std::filesystem::path& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << filename << std::endl;
        return;
    }

    std::string currentMaterialName;
    Material currentMaterial;
    bool hasCurrent = false;

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
            if (hasCurrent) {
                AddMaterial(currentMaterialName, currentMaterial);
            }

            char* materialName = p.TakeWord();
            if (materialName) {
                currentMaterialName = materialName;
                currentMaterial = Material(currentMaterialName);
                hasCurrent = true;
            }
            break;
        }

        case MtlElement::NS: // specular weight
        {
            float weight = p.TakeFloat();
            if (hasCurrent) currentMaterial.SetSpecularWeight(weight);
            break;
        }

        case MtlElement::KA: // ambient color
        {
            float r = p.TakeFloat();
            float g = p.TakeFloat();
            float b = p.TakeFloat();
            if (hasCurrent) currentMaterial.SetAmbientColor(glm::vec3(r, g, b));
            break;
        }

        case MtlElement::KS: // specular color
        {
            float r = p.TakeFloat();
            float g = p.TakeFloat();
            float b = p.TakeFloat();
            if (hasCurrent) currentMaterial.SetSpecularColor(glm::vec3(r, g, b));
            break;
        }

        case MtlElement::KD: // diffuse color
        {
            float r = p.TakeFloat();
            float g = p.TakeFloat();
            float b = p.TakeFloat();
            if (hasCurrent) currentMaterial.SetDiffuseColor(glm::vec3(r, g, b));
            break;
        }

        case MtlElement::D: // opacity
        {
            float d = p.TakeFloat();
            if (hasCurrent) currentMaterial.SetOpacity(d);
            break;
        }

        case MtlElement::ILLUM: // illumination model
        {
            int illum = p.TakeInt();
            if (hasCurrent) currentMaterial.SetIllumination(illum);
            break;
        }

        case MtlElement::MAP_KD: // diffuse texture map
        {
            // take rest of line as texture path (can contain spaces)
            char* texPath = p.TakeUntil('\0');
            if (texPath && hasCurrent) {
                // trim trailing spaces
                size_t len = std::strlen(texPath);
                while (len > 0 && (texPath[len - 1] == ' ' || texPath[len - 1] == '\t'))
                    texPath[--len] = '\0';

                std::filesystem::path texturePath = filename.parent_path() / texPath;

                currentMaterial.SetDiffuseTexture(Texture::LoadFile(texturePath.string()));
            }
            break;
        }

        case MtlElement::MAP_KA: // ambient texture map
        {
            char* texPath = p.TakeUntil('\0');
            if (texPath && hasCurrent) {
                size_t len = std::strlen(texPath);
                while (len > 0 && (texPath[len - 1] == ' ' || texPath[len - 1] == '\t'))
                    texPath[--len] = '\0';

                // optional: handle ambient texture
                // currentMaterial.SetAmbientTexture(Texture::LoadFile(texPath));
            }
            break;
        }

        default:
            // ignore unknown tokens
            break;
        }
    }

    // Commit last material if pending
    if (hasCurrent) {
        AddMaterial(currentMaterialName, currentMaterial);
    }

    file.close();
}

Material* Object::GetMaterial(const MaterialID& id)
{
    auto material = m_materials.find(id);
    if (material == m_materials.end()) return nullptr;
    return &material->second;
}

void Object::CreateNewMesh(const Material& material)
{
    EmplaceBack(material);
}

void Object::CreateNewMesh()
{
    EmplaceBack();
}

Mesh& Object::GetLastMesh()
{
    if (Empty()) {
        CreateNewMesh(Material::Default());
    }
    return Back();
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
                obj->LoadMTL(mtlPath);
            }
            break;
        }

        case ObjElement::USEMTL:
        {
            char* materialName = p.TakeWord();
            if (materialName) {
                auto material = obj->GetMaterial(materialName);
                if (!material) {
                    // Not defined material being used.
                    std::cerr << "[WARN] WavefrontError: use of undefined material '"
                              << materialName << "'" << std::endl;
                    material = new Material();
                }
                auto mesh = obj->FindMeshByMaterial(material);
                if (mesh == obj->End()) {
                    obj->CreateNewMesh(*material);
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

            std::vector<uint32_t> faceIndices;
            faceIndices.reserve(8);

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

                Vertex v;
                v.position = obj->m_vertices[vi];
                v.normal = (ni >= 0) ? obj->m_normals[ni] : glm::vec3(0.0f);
                v.uv = (ti >= 0) ? obj->m_texCoords[ti] : glm::vec2(0.0f);

                uint32_t idx = mesh.PushVertex(v);
                faceIndices.push_back(idx);
                // mesh.m_vertexBuffer.emplace_back(vert, norm, texCoord);
                // mesh.m_indexBuffer.push_back(mesh.m_vertexBuffer.size() - 1);
            }

            // [0, 1, 2]
            //     ^

            // triangulate polygon (fan)
            if (faceIndices.size() >= 3) {
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                    mesh.PushTriangle(faceIndices[0], faceIndices[i], faceIndices[i+1]);
                }
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
    std::cout << "Meshes count: " << obj->GetSize() << std::endl;
    std::cout << "Materials count: " << obj->m_materials.size() << std::endl;

    file.close();

    unsigned int i = 0;
    for (auto it = obj->Begin(); it != obj->End(); ++it, ++i) {
        std::cout << "Mesh #" << i << " primitives count: " << it->GetIndicesCount() / 3 << std::endl;
    }

    return obj;
}

void Object::EnableBatch(const OpenGL::InstanceBuffer* instanceBuffer) {
    // FIXME:

    // for (auto &mesh : m_meshes) {
    //     mesh.Bind();

    //     instanceBuffer->StartConfigure();
    //         std::size_t vec4Size = sizeof(glm::vec4);
    //         for (int i = 0; i < 4; ++i) {
    //             glEnableVertexAttribArray(3 + i); // use locations 3,4,5,6 for instance matrix
    //             glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
    //                                 sizeof(glm::mat4), (void*)(i * vec4Size));
    //             glVertexAttribDivisor(3 + i, 1); // IMPORTANT: one per instance, not per vertex
    //         }
    //     instanceBuffer->EndConfigure();
        
    //     mesh.Unbind();
    // }
}

void Object::Render(Shader& shader, unsigned int count)
{
    // FIXME:

    // for (auto &mesh : m_meshes)
    // {
    //     auto material = GetMaterial(mesh.GetMaterialName());

    //     // --- Basic material properties ---
    //     shader.setFloat("opacity", material->GetOpacity());

    //     // Albedo (base color)
    //     shader.setVec3("albedo", material->GetDiffuseColor());

    //     // Metallic and roughness (defaults)
    //     shader.setFloat("metallic", 0.8f);
    //     shader.setFloat("roughness", 0.5f);
    //     shader.setFloat("ao", 1.0f); // default ambient occlusion if none

    //     // --- Optional textures ---
    //     int texUnit = 0;

    //     // Albedo texture
    //     if (material->HasDiffuseTexture()) {
    //         shader.setBool("useAlbedoMap", true);
    //         glActiveTexture(GL_TEXTURE0 + texUnit);
    //         glBindTexture(GL_TEXTURE_2D, material->GetDiffuseTexture()->GetID());
    //         shader.setInt("albedoTex", texUnit++);
    //     } else {
    //         shader.setBool("useAlbedoMap", false);
    //     }

    //     // Metallic texture
    //     // if (material->HasMetallicTexture()) {
    //     if (false) {
    //         shader.setBool("useMetallicMap", true);
    //         glActiveTexture(GL_TEXTURE0 + texUnit);
    //         // glBindTexture(GL_TEXTURE_2D, material->GetMetallicTexture()->GetID());
    //         shader.setInt("metallicTex", texUnit++);
    //     } else {
    //         shader.setBool("useMetallicMap", false);
    //     }

    //     // Roughness texture
    //     // if (material->HasRoughnessTexture()) {
    //     if (false) {
    //         shader.setBool("useRoughnessMap", true);
    //         glActiveTexture(GL_TEXTURE0 + texUnit);
    //         // glBindTexture(GL_TEXTURE_2D, material->GetRoughnessTexture()->GetID());
    //         shader.setInt("roughnessTex", texUnit++);
    //     } else {
    //         shader.setBool("useRoughnessMap", false);
    //     }

    //     // AO texture
    //     // if (material->HasAoTexture()) {
    //     if (false) {
    //         shader.setBool("useAoMap", true);
    //         glActiveTexture(GL_TEXTURE0 + texUnit);
    //         // glBindTexture(GL_TEXTURE_2D, material->GetAoTexture()->GetID());
    //         shader.setInt("aoTex", texUnit++);
    //     } else {
    //         shader.setBool("useAoMap", false);
    //     }

    //     // --- Render mesh ---
    //     mesh.Render(count);
    // }
}

}
