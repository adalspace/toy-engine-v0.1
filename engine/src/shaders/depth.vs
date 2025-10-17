#version 410 core

// Input vertex attributes
layout (location = 0) in vec3 position;  // Vertex position in local space (model space)

// Uniforms for transformation matrices
uniform mat4 u_model;       // Model matrix: transforms from local space to world space
uniform mat4 u_lightSpace;

void main()
{
    gl_Position = u_lightSpace * u_model * vec4(position, 1.0);
}