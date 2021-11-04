#version 330 core

layout (location = 0) in vec3 vPos; //--- attribute로 설정된 위치 속성: 인덱스 0

void main()
{
    gl_Position = vec4 (vPos.x, vPos.y, vPos.z, 1.0);
}
