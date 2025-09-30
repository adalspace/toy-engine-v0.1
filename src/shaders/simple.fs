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
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * specularColor; 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor;

    vec3 ambient = ambientStrength * ambientColor;

    vec3 texColor = (useTexture)
        ? texture(diffuseTex, TexCoords).rgb
        : diffuseColor;

    vec3 result = (ambient + diffuse + specular) * texColor;
    FragColor = vec4(result, 1.0);
}