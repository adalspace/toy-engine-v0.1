#version 410 core

// Output color of the fragment (pixel)
out vec4 FragColor;  // RGBA color for the fragment, where A is the alpha (opacity)

in vec3 vertexPos;
in vec3 vertexNormal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;

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

#define LIGHT_COLOR vec3(1.0, 1.0, 1.0)

void main()
{
    // Lighting vectors
    vec3 lightDir = normalize(lightPos - vertexPos);
    vec3 norm = normalize(vertexNormal);
    vec3 viewDir = normalize(viewPos - vertexPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Phong components
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), clamp(shininess, 2, 256));
    // vec3 specular = (useSpecular) ? specularStrength * spec * specularColor : vec3(0.0);

    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), clamp(shininess, 2.0, 256.0));
    vec3 specular = (useSpecular) ? specularStrength * spec * specularColor : vec3(0.0);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor;

    vec3 ambient = ambientStrength * ambientColor;

    float distance    = length(lightPos - vertexPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    // tweak 0.09 and 0.032 for range/falloff

    // diffuse  *= attenuation;
    // specular *= attenuation;
    // ambient  *= attenuation; // optional, sometimes left constant

    vec3 texColor = (useTexture)
        ? texture(diffuseTex, TexCoords).rgb
        : diffuseColor;

    vec3 result = (ambient + diffuse + specular) * texColor;

    FragColor = vec4(result, opacity);
}