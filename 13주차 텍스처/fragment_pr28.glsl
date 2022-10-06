#version 330 core

out vec4 FragColor;

in vec3 outNormal;
in vec2 outTexCoord;

uniform sampler2D outTexture;

void main()
{
    FragColor = texture(outTexture, outTexCoord);
}