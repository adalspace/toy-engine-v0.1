#version 460 core

// Input vertex attributes
layout (location = 0) in vec3 position;  // Vertex position in local space (model space)
layout (location = 1) in vec3 normal;    // vertex normal
layout (location = 2) in vec2 texCoord;     // Vertex texture uv
layout (location = 3) in mat4 instanceModel;     // Vertex texture uv

// Output to fragment shader
out vec3 vertexPos;
out vec3 vertexNormal;
out vec2 TexCoords;
out vec4 fragPosLightSpace;

// Uniforms for transformation matrices
uniform mat4 u_model;       // Model matrix: transforms from local space to world space
uniform mat4 u_view;        // View matrix: transforms from world space to camera space (view space)
uniform mat4 u_projection;  // Projection matrix: transforms from camera space to clip space
uniform bool u_isInstanced;

void main()
{
    mat4 model = u_isInstanced ? instanceModel : u_model;

    vertexPos = vec3(model * vec4(position, 1.0));

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vertexNormal = normalMatrix * normal;
    // vertexNormal = normal;

    TexCoords = texCoord;

    // fragPosLightSpace = u_lightSpace * vec4(vertexPos, 1.0);

    gl_Position = u_projection * u_view * vec4(vertexPos, 1.0);
}