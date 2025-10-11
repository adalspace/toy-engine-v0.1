#version 410 core

// Output color of the fragment (pixel)
out vec4 FragColor;  // RGBA color for the fragment, where A is the alpha (opacity)

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 TexCoords;

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

// From Object Renderer

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

uniform float ambientStrength;

uniform float specularStrength;
uniform float shininess;
uniform bool useSpecular;

uniform float opacity;

uniform sampler2D diffuseTex;
uniform bool useTexture;

uniform bool isLight;

void main()
{
    // Lighting vectors
    vec3 norm = normalize(vertexNormal);
    vec3 viewDir = normalize(viewPos - vertexPos);
    // vec3 viewDir = normalize(-vertexPos);

    vec3 ambient = ambientStrength * ambientColor;
    vec3 texColor = (useTexture)
        ? texture(diffuseTex, TexCoords).rgb
        : diffuseColor;

    vec3 result = ambient;

    for (int i = 0; i < lightsCount; i++) {
        vec3 lightDir = normalize(lights[i].position - vertexPos);
        vec3 halfDir = normalize(lightDir + viewDir);
        
        // Blinn Phong
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * diffuseColor * lights[i].color * lights[i].intensity;

        float spec = pow(max(dot(norm, halfDir), 0.0), clamp(shininess, 2.0, 256.0));
        vec3 specular = (useSpecular) ?
            specularStrength * spec * specularColor * lights[i].color * lights[i].intensity
            : vec3(0.0);

        result += (diffuse + specular);
    }

    result *= texColor;

    if (isLight) {
        vec3 emissive = vec3(1.0, 1.0, 1.0) * 10.0; // big intensity
        FragColor = vec4(emissive, 1.0);
        return;
    }

    FragColor = vec4(result, opacity);
}