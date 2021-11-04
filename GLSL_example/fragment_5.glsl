#version 330 core

uniform vec4 vColor; //--- 응용 프로그램에서 변수 값 설정
out vec4 FragColor; //--- 출력할 객체의 색상

void main()
{
    FragColor = vColor;
}
