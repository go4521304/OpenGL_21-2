#version 330 core

in vec3 Pos;
in vec3 Color;
out vec3 passColor;

uniform mat4 modelTransform;

void main()
{
    gl_Position = modelTransform * vec4(Pos, 1.0);
    passColor = Color;
}