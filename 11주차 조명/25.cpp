// Reading Obj file
#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
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

GLuint g_window_w = 1000;
GLuint g_window_h = 800;

const int num_vertices = 3;
const int num_triangles = 1;

void Display();
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void make_vertexShader();
void make_fragmentShader();
void InitShader();
void InitBuffer();

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
vector<glm::vec3> vPos;
vector<glm::vec3> vNormal;

glm::vec3 lightColors[3] = { {1.0f, 1.0f, 1.0f}, {1.0f, 0.1f, 0.1f}, {0.1f, 0.1f, 1.0f} };
int selLightColor = 0;
float lightAngle = 0.0f;
bool lightRotate = false;

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
		std::cout << "GLEW Initialized\n";
		std::cout << "\nC: 조명의 색 바꾸기 \n";
		std::cout << "\nR: 회전 \n\n";
	}

	//////////////////////////////////////////////////////////////////////////////////////
	//// Create shader program an register the shader
	//////////////////////////////////////////////////////////////////////////////////////
	InitShader();
	InitBuffer();


	// callback functions
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(10, Animate, 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glutMainLoop();

	return 0;
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
	GLint pAttribute = glGetAttribLocation(s_program, "aPos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, outnormal.size() * sizeof(glm::vec3), &outnormal[0], GL_STATIC_DRAW);
	GLint nAttribute = glGetAttribLocation(s_program, "aNormal");
	glVertexAttribPointer(nAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(nAttribute);

	glEnable(GL_DEPTH_TEST);
}


void Display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//*************************************************************************
	// Drawing circle

	glUseProgram(s_program);
	glBindVertexArray(VAO);

	unsigned int pView = glGetUniformLocation(s_program, "view");
	unsigned int pProj = glGetUniformLocation(s_program, "proj");
	unsigned int pModel = glGetUniformLocation(s_program, "model");
	unsigned int pObjColor = glGetUniformLocation(s_program, "ObjectColor");
	unsigned int pLightPos = glGetUniformLocation(s_program, "lightPos");
	unsigned int pLightColor = glGetUniformLocation(s_program, "lightColor");
	unsigned int pViewPos = glGetUniformLocation(s_program, "viewPos");



	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	

	glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);	
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	m_view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
	m_proj = glm::perspective(glm::radians(45.0f), (float)g_window_w / (float)g_window_h, 0.1f, 100.0f);
	glUniformMatrix4fv(pView, 1, GL_FALSE, glm::value_ptr(m_view));
	glUniformMatrix4fv(pProj, 1, GL_FALSE, glm::value_ptr(m_proj));


	// 빛
	glUniform3f(pLightPos, 2.0f * glm::cos(glm::radians(lightAngle)), 0.0f, 2.0f * glm::sin(glm::radians(lightAngle)));
	glUniform3f(pLightColor, lightColors[selLightColor].x, lightColors[selLightColor].y, lightColors[selLightColor].z);

	glUniform3f(pViewPos, cameraPos.x, cameraPos.y, cameraPos.z);

	glm::mat4 m_model(1.0f);
	m_model = glm::scale(m_model, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model));
	glUniform3f(pObjColor, 1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_TRIANGLES, 0, num_Triangle);

	glm::mat4 m_model2(1.0f);
	m_model2 = glm::translate(m_model2, glm::vec3(-1.2f, 0.0f, 0.0f));
	m_model2 = glm::scale(m_model2, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model2));
	glUniform3f(pObjColor, 0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_TRIANGLES, 0, num_Triangle);


	glm::mat4 m_model3(1.0f);
	m_model3 = glm::translate(m_model3, glm::vec3(-2.3f, 0.0f, 0.0f));
	m_model3 = glm::scale(m_model3, glm::vec3(0.1f, 0.1f, 0.1f));
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model3));
	glUniform3f(pObjColor, 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, num_Triangle);

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
	case 'c':
	case 'C':
		selLightColor++;
		selLightColor %= 3;
		break;

	case 'r':
	case 'R':
		lightRotate = !lightRotate;
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

	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}


/******************************세이더 프로그램 만들기*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_pr25.glsl");

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
	GLchar* fragmentSource = filetobuf("fragment_pr25.glsl");

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
/**************************************************************************************/
/**************************************************************************************/