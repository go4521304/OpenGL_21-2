#version 330 core

layout (location = 0) in vec3 aPos;   // 위치 변수는 attribute position 0을 가집니다.
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * model * vec4(aPos, 1.0);

    FragPos = vec3(model * vec4(aPos, 1.0));

    Normal = vec3(model * vec4(aNormal, 1.0));
}  
