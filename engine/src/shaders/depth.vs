#version 460 core

// Input vertex attributes
layout (location = 0) in vec3 position;  // Vertex position in local space (model space)
layout (location = 3) in mat4 instanceModel;     // Vertex texture uv

// layout (std140, binding = 1) uniform Matrices
// {
//     mat4 projection;
//     mat4 view;
// };

// Uniforms for transformation matrices
uniform mat4 u_model;       // Model matrix: transforms from local space to world space
uniform mat4 u_lightSpace;
uniform bool u_isInstanced;

void main()
{
    mat4 model = u_isInstanced ? instanceModel : u_model;

    gl_Position = u_lightSpace * model * vec4(position, 1.0);
}