#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 PVMmatrix;

out vec2  vTexCoord;   // texture coords
out float vTexU;       // U test

void main() {
    gl_Position  = PVMmatrix * vec4(position, 1.0);
    vTexCoord    = texCoord;
    vTexU         = texCoord.x;
}