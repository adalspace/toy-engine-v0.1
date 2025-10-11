#version 410 core
out vec4 FragColor;

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 TexCoords;
in vec4 fragPosLightSpace;

uniform vec3 viewPos;

// Lights
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};
#define MAX_LIGHTS 10
uniform int lightsCount;
uniform Light lights[MAX_LIGHTS];

uniform bool isLight;
uniform vec3 currentLightColor;

// Material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D albedoTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D aoTex;
uniform bool useAlbedoMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useAoMap;

// Shadows
uniform sampler2D shadowMap;

uniform float opacity;
// uniform int currentLight;

#define PI 3.14159265359
#define LIGHT_COLOR vec3(1.0, 1.0, 1.0)

float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    // transform to [0,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // if outside light's orthographic frustum => not in shadow
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    // get depth from shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // bias to prevent self-shadowing (depend on slope)
    float bias = max(0.001 * (1.0 - dot(N, L)), 0.0005);

    // PCF (3x3)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth ? 1.0 : 0.0);
        }
    }
    shadow /= 9.0;

    return shadow;
}

// ----------------------------------------------------------------------------
// Helper functions (GGX, Fresnel, Geometry)
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
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float ggx1 = GeometrySchlickGGX(max(dot(N,L),0.0), roughness);
    float ggx2 = GeometrySchlickGGX(max(dot(N,V),0.0), roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
// Main
void main()
{
    if (isLight) {
        vec3 emissive = currentLightColor * 10.0;
        FragColor = vec4(emissive, 1.0);
        return;
    }

    vec3 N = normalize(vertexNormal);
    vec3 V = normalize(viewPos - vertexPos);

    vec3 baseColor = useAlbedoMap ? texture(albedoTex, TexCoords).rgb : albedo;
    float metal   = useMetallicMap  ? texture(metallicTex, TexCoords).r : metallic;
    float rough   = useRoughnessMap ? texture(roughnessTex, TexCoords).r : roughness;
    float aoValue = useAoMap        ? texture(aoTex, TexCoords).r        : ao;

    vec3 F0 = mix(vec3(0.04), baseColor, metal);

    vec3 Lo = vec3(0.0);
    // FragColor = vec4(1.0 - shadow, 1.0 - shadow, 1.0 - shadow, 1.0);
    // return;

    float shadow = 0.0;

    // Loop over all lights
    for (int i = 0; i < lightsCount; i++)
    {
        vec3 L = normalize(lights[i].position - vertexPos);
        vec3 H = normalize(V + L);

        float NDF = DistributionGGX(N, H, rough);
        float G   = GeometrySmith(N, V, L, rough);
        vec3  F   = fresnelSchlick(max(dot(H,V),0.0), F0);

        shadow = ShadowCalculation(fragPosLightSpace, N, L);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metal;

        float NdotL = max(dot(N,L), 0.0);

        vec3 radiance = lights[i].color * lights[i].intensity;
        Lo += (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    // Ambient
    vec3 ambient = vec3(0.03) * baseColor * aoValue;

    // TODO: apply shadow
    vec3 color = ambient + (1.0 - shadow) * Lo;

    // HDR tonemapping + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, opacity);
}
