#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

out vec3 outNormal;
out vec2 outTexCoord;

uniform mat4 modelTransform;

void main()
{
    gl_Position = modelTransform * vec4(vPos, 1.0);
    outNormal = vNormal;
    outTexCoord = vTexCoord;
}