#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <glm/glm.hpp>
#include <memory>

#include "engine/renderer/texture.h"

#ifndef DEFAULT_MATERIAL_NAME
    #define DEFAULT_MATERIAL_NAME "__default_material"
#endif

namespace Core {

typedef std::string MaterialID;

class Material {
public:
    Material(const std::string& name) : m_id(name) {}

    Material() : Material(DEFAULT_MATERIAL_NAME) {}

    // Material(const Material&) = default;
public:
    static Material Default() {
        Material material;
        material.SetAmbientColor(glm::vec3(0.52f, 0.52f, 0.52f));
        return material;
    }
public:
    inline const MaterialID GetID() const { return m_id; }

    inline const glm::vec3 GetAmbientColor() const { return m_ambient; }
    inline const glm::vec3 GetDiffuseColor() const { return m_diffuse; }
    inline const glm::vec3 GetSpecularColor() const { return m_specular; }
    inline const float GetSpecularWeight() const { return m_shininess; }
    inline const bool HasDiffuseTexture() const { return m_diffuse_tex != nullptr; }
    inline const Texture* GetDiffuseTexture() const { return m_diffuse_tex; }
    inline const float GetOpacity() const { return m_opacity; }
    inline const int GetIllumination() const { return m_illum; }
public:
    inline void SetAmbientColor(glm::vec3 ambient) { m_ambient = ambient; }
    inline void SetDiffuseColor(glm::vec3 diffuse) { m_diffuse = diffuse; }
    inline void SetSpecularColor(glm::vec3 specular) { m_specular = specular; }
    inline void SetSpecularWeight(float weight) { m_shininess = weight; }
    inline void SetDiffuseTexture(Texture* texture) { m_diffuse_tex = texture; }
    inline void SetOpacity(float opacity) { m_opacity = opacity; }
    inline void SetIllumination(float illum) { m_illum = illum; }
private:
    glm::vec3 m_ambient { 0.2f, 0.2f, 0.2f };
    glm::vec3 m_diffuse { 0.8f, 0.8f, 0.8f };
    glm::vec3 m_specular { 1.0f, 1.0f, 1.0f };
    float m_shininess { 32.0f };
    float m_opacity { 1.0f };
    int m_illum { 2 };

    MaterialID m_id { 0 };

    Texture* m_diffuse_tex { nullptr };
}; // class Material

} // namespace Core

#endif // MATERIAL_H_