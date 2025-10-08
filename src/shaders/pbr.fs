#version 410 core

out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 TexCoords;

// Lighting inputs
uniform vec3 lightPos;
uniform vec3 viewPos;

// Material parameters
uniform vec3 albedo;           // Base color (replaces diffuseColor)
uniform float metallic;        // 0 = dielectric, 1 = metal
uniform float roughness;       // 0 = smooth mirror, 1 = rough
uniform float ao;              // Ambient occlusion

// Textures
uniform sampler2D albedoTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D aoTex;
uniform bool useAlbedoMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useAoMap;

uniform float opacity;

// Used for emissive light sources
uniform bool isLight;

#define PI 3.14159265359
#define LIGHT_COLOR vec3(1.0, 1.0, 1.0)

// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
void main()
{
    if (isLight) {
        vec3 emissive = LIGHT_COLOR * 10.0; // bright light source
        FragColor = vec4(emissive, 1.0);
        return;
    }

    // Inputs
    vec3 N = normalize(vertexNormal);
    vec3 V = normalize(viewPos - vertexPos);
    vec3 L = normalize(lightPos - vertexPos);
    vec3 H = normalize(V + L);

    // Base color (albedo)
    vec3 baseColor = useAlbedoMap ? texture(albedoTex, TexCoords).rgb : albedo;

    float metal     = useMetallicMap  ? texture(metallicTex, TexCoords).r  : metallic;
    float rough     = useRoughnessMap ? texture(roughnessTex, TexCoords).r : roughness;
    float aoValue   = useAoMap        ? texture(aoTex, TexCoords).r        : ao;

    // Reflectance at normal incidence (F0)
    vec3 F0 = vec3(0.04); // typical dielectric reflectance
    F0 = mix(F0, baseColor, metal); // metals use albedo as F0

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, rough);
    float G   = GeometrySmith(N, V, L, rough);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular     = numerator / denominator;

    // kS is specular reflection, kD is diffuse reflection (energy conservation)
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;

    float NdotL = max(dot(N, L), 0.0);

    vec3 radiance = LIGHT_COLOR; // single light source color/intensity

    vec3 Lo = (kD * baseColor / PI + specular) * radiance * NdotL;

    // Ambient (IBL approximation using ao)
    vec3 ambient = vec3(0.03) * baseColor * aoValue;

    vec3 color = ambient + Lo;

    // HDR tonemapping and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, opacity);
}
