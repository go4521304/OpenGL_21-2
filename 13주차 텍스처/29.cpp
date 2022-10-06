// Reading Obj file
#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <random>
#include "gl/glew.h"
#include "gl/freeglut.h"
#include "gl/glm/glm.hpp"
#include "gl/glm/ext.hpp"
#include "gl/glm/gtc/matrix_transform.hpp"
#include "shader.h"

#include "objRead.cpp"

using namespace std;

char* filetobuf(const char* file)
{
	ifstream in{ file };

	if (!in.is_open())
		return NULL;

	string s(istreambuf_iterator<char>{in}, {});

	char* str = new char[s.size() + 1];
	copy(s.begin(), s.end(), str);
	str[s.size()] = '\0';

	return str;
}

// 0,1,2, 0,3,1, 0,4,3, 0,2,4, 2,1,3, 2,3,4
// 앞 왼 뒤 오 아래
glm::vec3 Pyramid[] = {
	{0.0f, 0.8f, 0.0f},
	{-0.5f, 0.0f, +0.5f},
	{+0.5f, 0.0f, +0.5f}
};
glm::vec3 PyramidNormal[] = {
	{0.0f, 0.0f, 1.0f}
};

// 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7
// 앞 왼 오 뒤 위 아래
glm::vec3 Cube[] = {
	{-0.25f, +0.25f, +0.25f},
	{+0.25f, +0.25f, +0.25f},
	{-0.25f, -0.25f, +0.25f},
	{+0.25f, -0.25f, +0.25f},
	{-0.25f, +0.25f, -0.25f},
	{+0.25f, +0.25f, -0.25f},
	{-0.25f, -0.25f, -0.25f},
	{+0.25f, -0.25f, -0.25f}
};
glm::vec3 CubeNormal[] = {
	{0.0f, 0.0f, 1.0f},
	{-1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, -1.0f, 0.0f}
};

GLuint g_window_w = 1000;
GLuint g_window_h = 800;

constexpr float angleVal = 1.0f;
constexpr float moveVal = 0.1f;

const int num_vertices = 3;
const int num_triangles = 1;

void Display();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void make_vertexShader();
void make_fragmentShader();
void InitShader();
void InitBuffer();
void InitBuffer_My();
void BlendCubeSet();


void makeTriangle(int curr, int level, glm::vec3 vert[3]);
void OrbitSet();
void SnowSet();
GLvoid Animate(int val);


//--- load obj related variabales
int loadObj(const char* filename);
int loadObj_normalize_center(const char* filename);
float* sphere_object;
int num_Triangle;
float sunSize;

GLuint s_program;
GLuint vertexShader, fragmentShader; //--- 세이더 객체

GLuint VAO;
GLuint VBO_position;
GLuint VBO_normal;

GLuint My_VAO;
GLuint My_VBO_position;
GLuint My_VBO_normal;

vector<glm::vec3> vPos;
vector<glm::vec3> vNormal;

float lightAngle = 0.0f, lightRadius = 1.0f, brightness = 1.0f;
bool lightRotate = false;
int triLevel = 0;

float camAngle = 0.0f;
bool isCamRotate = false;

struct Orbit
{
	float Angle, OrbitAngle, Radius, Scale, Speed;
	glm::vec3 Color;
};

Orbit planet[3];

struct Snow
{
	glm::vec3 pos;
	float speed;
};

vector<Snow> snow;
bool snowing = false;

glm::vec3 CubePosition[5];

int main(int argc, char** argv)
{
	// create window using freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(g_window_w, g_window_h);
	glutInitWindowPosition(0, 0);

	glutCreateWindow("Computer Graphics");

	//////////////////////////////////////////////////////////////////////////////////////
	//// initialize GLEW
	//////////////////////////////////////////////////////////////////////////////////////
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n\n";
		std::cout << "0~7: 프랙탈 삼각형 단계 조절\n";
		std::cout << "S: 눈 내리기\n";
		std::cout << "R: 조명 회전 \n\n";
		std::cout << "+/-: 조명의 밝기 조절 \n";
		std::cout << "i/o: 조명 거리 조절\n";


		std::cout << "\n\n\n";
	}

	OrbitSet();
	BlendCubeSet();
	//////////////////////////////////////////////////////////////////////////////////////
	//// Create shader program an register the shader
	//////////////////////////////////////////////////////////////////////////////////////
	InitShader();
	InitBuffer();
	InitBuffer_My();

	// callback functions
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(10, Animate, 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();

	return 0;
}


void Display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//*************************************************************************
	// Drawing circle

	glUseProgram(s_program);

	unsigned int pView = glGetUniformLocation(s_program, "view");
	unsigned int pProj = glGetUniformLocation(s_program, "proj");
	unsigned int pModel = glGetUniformLocation(s_program, "model");
	unsigned int pObjColor = glGetUniformLocation(s_program, "ObjectColor");
	unsigned int pLightPos = glGetUniformLocation(s_program, "lightPos");
	unsigned int pLightColor = glGetUniformLocation(s_program, "lightColor");
	unsigned int pViewPos = glGetUniformLocation(s_program, "viewPos");



	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);


	glm::vec3 cameraPos(5.0f * glm::sin(glm::radians(camAngle)), 3.0f, 5.0f * glm::cos(glm::radians(camAngle)));
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	m_view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
	m_proj = glm::perspective(glm::radians(45.0f), (float)g_window_w / (float)g_window_h, 0.1f, 100.0f);
	glUniformMatrix4fv(pView, 1, GL_FALSE, glm::value_ptr(m_view));
	glUniformMatrix4fv(pProj, 1, GL_FALSE, glm::value_ptr(m_proj));


	// 빛
	glm::vec3 lightPos(lightRadius * glm::cos(glm::radians(lightAngle)), lightRadius, lightRadius * glm::sin(glm::radians(lightAngle)));
	glUniform3f(pLightPos, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(pLightColor, brightness, brightness, brightness);

	glUniform3f(pViewPos, cameraPos.x, cameraPos.y, cameraPos.z);


	// Use My_VAO
	glBindVertexArray(My_VAO);

	// 조명 박스
	glm::mat4 mat_lightBox(1.0f);
	mat_lightBox = glm::translate(mat_lightBox, lightPos);
	mat_lightBox = glm::rotate(mat_lightBox, glm::radians(-lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	mat_lightBox = glm::rotate(mat_lightBox, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mat_lightBox = glm::scale(mat_lightBox, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(mat_lightBox));
	glUniform4f(pObjColor, 1.0f, 1.0f, 1.0f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 6, 36);


	// 바닥면
	glm::mat4 m_floor(1.0f);
	m_floor = glm::scale(m_floor, { 5.0f, 0.0f, 5.0f });
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_floor));
	glUniform4f(pObjColor, 0.5f, 0.5f, 0.5f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// 삼각형
	glUniform4f(pObjColor, 0.2f, 0.2f, 0.8f, 1.0f);
	glm::mat4 m_triangle1(1.0f);
	//m_triangle1 = glm::scale(m_triangle1, { 2.0f, 2.0f, 2.0f });
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_triangle1));
	glDrawArrays(GL_TRIANGLES, 42, vPos.size());

	glm::mat4 m_triangle2(1.0f);
	m_triangle2 = glm::rotate(m_triangle2, glm::radians(90.0f), { 0.0f, 1.0f, 0.0f });
	//m_triangle2 = glm::scale(m_triangle2, { 2.0f, 2.0f, 2.0f });
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_triangle2));
	glDrawArrays(GL_TRIANGLES, 42, vPos.size());

	glm::mat4 m_triangle3(1.0f);
	m_triangle3 = glm::rotate(m_triangle3, glm::radians(180.0f), { 0.0f, 1.0f, 0.0f });
	//m_triangle3 = glm::scale(m_triangle3, { 2.0f, 2.0f, 2.0f });
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_triangle3));
	glDrawArrays(GL_TRIANGLES, 42, vPos.size());

	glm::mat4 m_triangle4(1.0f);
	m_triangle4 = glm::rotate(m_triangle4, glm::radians(270.0f), { 0.0f, 1.0f, 0.0f });
	//m_triangle4 = glm::scale(m_triangle4, { 2.0f, 2.0f, 2.0f });
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_triangle4));
	glDrawArrays(GL_TRIANGLES, 42, vPos.size());



	//// Use VAO
	glBindVertexArray(VAO);

	// mercury
	for (const auto& i : planet)
	{
		glm::mat4 m_planet(1.0f);
		m_planet = glm::rotate(m_planet, glm::radians(i.OrbitAngle), { 1.0f, 0.0f, 0.0f });
		m_planet = glm::translate(m_planet, { i.Radius * glm::cos(glm::radians(i.Angle)), 1.0f, i.Radius * glm::sin(glm::radians(i.Angle)) });
		m_planet = glm::scale(m_planet, { i.Scale, i.Scale, i.Scale });
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_planet));
		glUniform4f(pObjColor, i.Color.r, i.Color.g, i.Color.b, 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, num_Triangle);
	}

	//snow
	if (snowing)
	{
		glUniform4f(pObjColor, 1.0f, 1.0f, 1.0f, 1.0f);
		for (const auto& i : snow)
		{
			glm::mat4 m_snow(1.0f);
			m_snow = glm::translate(m_snow, { i.pos.x, i.pos.y, i.pos.z });
			m_snow = glm::scale(m_snow, { 0.05f, 0.05, 0.05f });
			glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_snow));
			glDrawArrays(GL_TRIANGLES, 0, num_Triangle);
		}
	}

	// Blend Obj
	int cubeRendering[5] = {0, 1, 2, 3, 4};
	for (int i = 0; i<5; ++i)
	{
		for (int j = i; j<5; ++j)
		{
			if ((cameraPos.x - CubePosition[cubeRendering[i]].x) + (cameraPos.z - CubePosition[cubeRendering[i]].z) <
				(cameraPos.x - CubePosition[cubeRendering[j]].x) + (cameraPos.z - CubePosition[cubeRendering[j]].z))
			{
				int tmp = cubeRendering[i];
				cubeRendering[i] = cubeRendering[j];
				cubeRendering[j] = tmp;
			}
		}
	}
	glBindVertexArray(My_VAO);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4f(pObjColor, 1.0f, 0.0f, 1.0f, 0.2f);
	for (const int i : cubeRendering)
	{
		glm::mat4 mat_modle(1.0f);
		mat_modle = glm::translate(mat_modle, CubePosition[cubeRendering[i]]);
		mat_modle = glm::scale(mat_modle, glm::vec3(1.0f, 2.0f, 1.0f));
		mat_modle = glm::translate(mat_modle, {0.f, 0.25f, 0.f});
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(mat_modle));
		glDrawArrays(GL_TRIANGLES, 6, 36);
	}
	glDisable(GL_BLEND);

	glutSwapBuffers();
}


void Reshape(int w, int h)
{
	g_window_w = w;
	g_window_h = h;
	glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'r':
	case 'R':
		lightRotate = !lightRotate;
		break;

	case 'i':
		lightRadius += moveVal;
		break;

	case 'o':
		lightRadius -= moveVal;
		break;

	case '+':
		if (brightness < 1.0f)
			brightness += 0.1f;
		break;

	case '-':
		if (brightness > 0.0f)
			brightness -= 0.1f;
		break;

	case 's':
	case 'S':
		snowing = !snowing;
		if (snowing)
			SnowSet();
		break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		triLevel = key - '0';
		InitBuffer_My();
		break;

	case 'y':
	case 'Y':
		isCamRotate = !isCamRotate;
		break;

	case 'q':
	case 'Q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;
	}

}

GLvoid Animate(int val)
{
	if (lightRotate)
	{
		lightAngle += 1.0f;
	}

	for (auto& i : planet)
	{
		i.Angle += i.Speed;
	}

	if (snowing)
	{
		for (auto& i : snow)
		{
			i.pos.y -= i.speed;

			if (i.pos.y < 0.0f)
				i.pos.y = 2.0f;
		}
	}

	if (isCamRotate)
		camAngle += angleVal;

	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}


/******************************세이더 프로그램 만들기*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_specular.glsl");

	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);

	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	GLchar* fragmentSource = filetobuf("fragment_specular_blending.glsl");

	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);

	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- 버텍스 세이더 만들기
	make_fragmentShader(); //--- 프래그먼트 세이더 만들기

	//-- shader Program
	s_program = glCreateProgram();

	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);

	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void InitBuffer()
{
	num_Triangle = loadObj_normalize_center("sphere.obj");

	//// 5.1. VAO 객체 생성 및 바인딩
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO_position);
	glGenBuffers(1, &VBO_normal);

	// 2 triangles for quad floor
	glUseProgram(s_program);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, outvertex.size() * sizeof(glm::vec3), &outvertex[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, outnormal.size() * sizeof(glm::vec3), &outnormal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
}

void InitBuffer_My()
{
	vPos.clear();
	vNormal.clear();

	const glm::vec3 floor[] = { {-1.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, -1.0f}, {1.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f} };
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	for (const auto& i : floor)
	{
		vPos.push_back(i);
		vNormal.push_back({ 0.0f, 1.0f, 0.0f });
	}

	int cnt = 0;
	int normalindex = 0;
	for (const int& i : cubeIndex)
	{
		if (cnt == 6)
			normalindex++;
		cnt %= 6;
		vPos.push_back(Cube[i]);
		vNormal.push_back(CubeNormal[normalindex]);
		cnt++;
	}

	makeTriangle(0, triLevel, Pyramid);

	// 바닥면과 삼각형 그릴꺼
	glGenVertexArrays(1, &My_VAO);
	glGenBuffers(1, &My_VBO_position);
	glGenBuffers(1, &My_VBO_normal);

	glBindVertexArray(My_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, My_VBO_position);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, My_VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, vNormal.size() * sizeof(glm::vec3), vNormal.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
}
/**************************************************************************************/
/**************************************************************************************/

void makeTriangle(int curr, int level, glm::vec3 vert[3])
{
	if (curr == level)
	{
		for (int i = 0; i < 3; ++i)
		{
			vPos.push_back(vert[i]);
			vNormal.push_back(PyramidNormal[0]);
		}
		return;
	}
	else
	{
		glm::vec3 tmp[3];
		// 위
		tmp[0] = vert[0];
		tmp[1] = { vert[1].x + ((vert[2].x - vert[1].x) / 4), vert[1].y + ((vert[0].y - vert[1].y) / 2), vert[1].z - ((vert[1].z - vert[0].z) / 2) };
		tmp[2] = { vert[2].x - ((vert[2].x - vert[1].x) / 4), vert[2].y + ((vert[0].y - vert[1].y) / 2), vert[2].z - ((vert[1].z - vert[0].z) / 2) };
		makeTriangle(curr + 1, level, tmp);

		// 왼쪽
		tmp[0] = { vert[1].x + ((vert[2].x - vert[1].x) / 4), vert[1].y + ((vert[0].y - vert[1].y) / 2), vert[1].z - ((vert[1].z - vert[0].z) / 2) };
		tmp[1] = vert[1];
		tmp[2] = { vert[2].x - ((vert[2].x - vert[1].x) / 2), vert[2].y, vert[2].z };
		makeTriangle(curr + 1, level, tmp);

		// 오른쪽
		tmp[0] = { vert[2].x - ((vert[2].x - vert[1].x) / 4), vert[2].y + ((vert[0].y - vert[1].y) / 2), vert[2].z - ((vert[1].z - vert[0].z) / 2) };
		tmp[1] = { vert[2].x - ((vert[2].x - vert[1].x) / 2), vert[2].y, vert[2].z };
		tmp[2] = vert[2];
		makeTriangle(curr + 1, level, tmp);
	}
}

void OrbitSet()
{
	planet[0].Angle = 0.0f;
	planet[0].OrbitAngle = 10.0;
	planet[0].Color = { 0.3f, 0.3f, 0.3f };
	planet[0].Radius = 0.5f;
	planet[0].Scale = 0.1f;
	planet[0].Speed = 2.0f;

	planet[1].Angle = 0.0f;
	planet[1].OrbitAngle = -30.0f;
	planet[1].Color = { 1.0f, 1.0f, 0.7f };
	planet[1].Radius = 1.0f;
	planet[1].Scale = 0.3f;
	planet[1].Speed = 0.8f;

	planet[2].Angle = 0.0f;
	planet[2].OrbitAngle = 7.0f;
	planet[2].Color = { 0.3f, 0.6f, 0.8f };
	planet[2].Radius = 2.0f;
	planet[2].Scale = 0.5f;
	planet[2].Speed = 0.3f;
}

void SnowSet()
{
	snow.clear();

	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float> PosX(-2.0f, 2.0f);
	uniform_real_distribution<float> PosZ(-2.0f, 2.0f);
	uniform_real_distribution<float> Speed(0.01f, 0.05f);


	for (int i = 0; i < 100; ++i)
	{
		Snow tmp;
		tmp.pos.x = PosX(dre);
		tmp.pos.y = 2.0f;
		tmp.pos.z = PosZ(dre);
		tmp.speed = Speed(dre);
		snow.push_back(tmp);
	}
}

void BlendCubeSet()
{
	random_device rd;
	default_random_engine dre(rd());
	uniform_real_distribution<float> urd(-5.0f, 5.0f);

	for (int i = 0; i<5; ++i)
	{
		while (1)
		{
			float x = urd(dre);
			float y = urd(dre);

			if ((-0.5f < x && x < 0.5f) || (-0.5f < y && y < 0.5f))
				continue;
			else
			{
				CubePosition[i] = { x, 0.0f, y };
				break;
			}
		}
	}
}