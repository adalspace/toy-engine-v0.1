#version 410 core

// Output color
out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;

// From Object Renderer
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;   // used as F0 (base reflectance)

uniform float ambientStrength;
uniform float specularStrength; 
uniform float shininess;      // mapped to roughness
uniform bool useSpecular;

uniform float opacity;

uniform sampler2D diffuseTex;
uniform bool useTexture;

#define LIGHT_COLOR vec3(1.0, 1.0, 1.0)

// ----------------------------------------------------------------------------
// Helper functions for Cook-Torrance BRDF
// ----------------------------------------------------------------------------

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;

    return num / denom;
}

// Geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // remapped for direct lighting

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Smith's geometry function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel term (Schlick's approximation)
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
void main()
{
    vec3 N = normalize(vertexNormal);
    vec3 V = normalize(viewPos - vertexPos);
    vec3 L = normalize(lightPos - vertexPos);
    vec3 H = normalize(V + L);

    // Texture or uniform color
    vec3 albedo = (useTexture)
        ? texture(diffuseTex, TexCoords).rgb
        : diffuseColor;

    // Map shininess to roughness (inverse relationship)
    float roughness = clamp(1.0 - (shininess / 256.0), 0.05, 1.0);

    // Base reflectivity (F0)
    vec3 F0 = mix(vec3(0.04), specularColor, specularStrength);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular     = numerator / denominator;

    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (useSpecular ? 1.0 : 1.0); // keep diffuse always unless specular is off

    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = kD * albedo / 3.14159265;
    vec3 radiance = LIGHT_COLOR;

    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    // Ambient (simple, not IBL)
    vec3 ambient = ambientStrength * ambientColor * albedo;

    vec3 result = ambient + Lo;

    // Gamma correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, opacity);
}
