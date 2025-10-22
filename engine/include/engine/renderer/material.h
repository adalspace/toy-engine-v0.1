#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <glm/glm.hpp>
#include <memory>

#include "engine/renderer/texture.h"

namespace Engine {

class Material {
private:
    glm::vec3 m_ambient { 0.2f, 0.2f, 0.2f };
    glm::vec3 m_diffuse { 0.8f, 0.8f, 0.8f };
    glm::vec3 m_specular { 1.0f, 1.0f, 1.0f };
    float m_shininess { 32.0f };
    float m_opacity { 1.0f };
    int m_illum { 2 };

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
    inline const float GetOpacity() const { return m_opacity; }
    inline const int GetIllumination() const { return m_illum; }
public:
    inline void SetAmbientColor(glm::vec3 ambient) { m_ambient = ambient; }
    inline void SetDiffuseColor(glm::vec3 diffuse) { m_diffuse = diffuse; }
    inline void SetSpecularColor(glm::vec3 specular) { m_specular = specular; }
    inline void SetSpecularWeight(float weight) { m_shininess = weight; }
    inline void SetDiffuseTexture(std::unique_ptr<Texture>&& texture) { m_diffuse_tex = std::move(texture); }
    inline void SetOpacity(float opacity) { m_opacity = opacity; }
    inline void SetIllumination(float illum) { m_illum = illum; }
};

}

#endif // MATERIAL_H_