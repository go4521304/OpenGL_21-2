#version 330 core

in vec2 Pos;
in vec3 Color;
out vec3 passColor;

void main()
{
    gl_Position = vec4(Pos.x, Pos.y, 0.0, 1.0);
    passColor = Color;
}