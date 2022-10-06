#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using namespace std;

const float angleSpeed = 10.0f;
const float moveSpeed = 0.1f;

void convert_glpos(int x, int y, float& new_X, float& new_Y)
{
	int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);

	new_X = (float)((x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0)));
	new_Y = -(float)((y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0)));
}

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

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKey(int key, int x, int y);
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();
void InitTexture();

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo[3];

vector<glm::vec3> vPos;
vector<glm::vec3> vNormal;
vector<glm::vec2> vTexCoord;

// 인덱스 리스트로 변경해보기

glm::vec3 Cube[] = {
	{-0.5f, -0.5f, 0.5f},
	{0.5f, -0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	{0.5f, 0.5f, 0.5f},
	{-0.5f, 0.5f, 0.5f},
	{-0.5f, -0.5f, 0.5f}
};

glm::vec2 CubeTexcoord[] = {
	{0.0f, 0.0f},
	{1.0f, 0.0f},
	{1.0f, 1.0f},
	{1.0f, 1.0f},
	{0.0f, 1.0f},
	{0.0f, 0.0f}
};

// 0,1,2, 0,3,1, 0,4,3, 0,2,4, 2,1,3, 2,3,4
// 앞 왼 뒤 오 아래
glm::vec3 Pyramid[] = {
	{0.0f, 0.25f, 0.0f},
	{-0.25f, -0.25f, +0.25f},
	{+0.25f, -0.25f, +0.25f}
};
glm::vec2 PyramidTexcoord[] = {
	{0.5f, 1.0f},
	{0.0f, 0.0f},
	{1.0f, 0.0f}
};


float xRotate = 30.0f, yRotate = -30.0f;
float xRotateDir = 0.0f, yRotateDir = 0.0f;
glm::vec2 movement = { 0.0f, 0.0f };


bool selModel = true;	// true == Cube / false == Tetra
bool isBackCull = false, isFill = true;

unsigned int texture[6];

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice14");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n\n";
		std::cout << "C/P: 모델 변경\n";
		std::cout << "W: 모델 채우기\n";
		std::cout << "Q: 종료\n";

	}

	glEnable(GL_DEPTH_TEST);

	InitShader();
	InitBuffer();
	InitTexture();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKey);
	glutTimerFunc(100, Animate, 0);

	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{


	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_vert();

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);
	// 변환 행렬 생성

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(movement, 0.0f));
	model = glm::rotate(model, (float)glm::radians(-xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));

	unsigned int modelMat = glGetUniformLocation(s_program, "modelTransform");

	// 설정 적용
	if (isBackCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if (isFill)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	
	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i<4; ++i)
	{
		glm::mat4 tmp(1.0f);
		tmp = glm::rotate(tmp, glm::radians(90.0f * i), { 0.0f, 1.0f, 0.0f });
		tmp = model * tmp;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(tmp));
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	if (selModel)
	{
		glm::mat4 tmp(1.0f);
		tmp = glm::rotate(tmp, glm::radians(90.0f), { 1.0f, 0.0f, 0.0f });
		tmp = model * tmp;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(tmp));
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		tmp = glm::mat4(1.0f);
		tmp = glm::rotate(tmp, glm::radians(-90.0f), { 1.0f, 0.0f, 0.0f });
		tmp = model * tmp;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(tmp));
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h)		//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}


/******************************세이더 프로그램 만들기*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_pr28.glsl");

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
	GLchar* fragmentSource = filetobuf("fragment_pr28.glsl");

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

void set_vert()
{
	const glm::vec3 line[] = { {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, +1.0f, 0.0f} };
	const int pyramidIndex[] = { 0,1,2};

	vPos.clear();
	vTexCoord.clear();

	if (selModel)
	{
		for (const auto& i : Cube)
		{
			vPos.push_back(i);
			vNormal.push_back({ 0.0f, 0.0f, 1.0f });
		}
		for (const auto& i : CubeTexcoord)
		{
			vTexCoord.push_back(i);
		}
	}
	else
	{
		for (const int& i : pyramidIndex)
		{
			vPos.push_back(Pyramid[i]);
			vNormal.push_back({ 0.0f, 0.0f, 1.0f });
		}
		for (const auto& i : PyramidTexcoord)
		{
			vTexCoord.push_back(i);
		}
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vNormal.size() * sizeof(glm::vec3), vNormal.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vTexCoord.size() * sizeof(glm::vec2), vTexCoord.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(2);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(3, vbo);

	set_vert();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'c':
	case 'C':
		selModel = true;
		break;

	case 'p':
	case 'P':
		selModel = false;
		break;

	case 'h':
	case 'H':
		isBackCull = !isBackCull;
		break;

	case 'x':
	case 'X':
		if (xRotateDir == 0.0f)
			xRotateDir = angleSpeed;
		else
			xRotateDir *= -1;
		break;

	case 'y':
	case 'Y':
		if (yRotateDir == 0.0f)
			yRotateDir = angleSpeed;
		else
			yRotateDir *= -1;
		break;

	case 'w':
	case 'W':
		isFill = !isFill;
		break;

	case 's':
	case 'S':
		xRotate = 30.0f;
		xRotateDir = 0.0f;
		yRotate = -30.0f;
		yRotateDir = 0.0f;

		movement = glm::vec2(0.0f, 0.0f);
		break;

	case 'q':
	case 'Q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;

	default:
		break;
	}

	glutPostRedisplay();
}

GLvoid SpecialKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		movement.y += moveSpeed;
		break;

	case GLUT_KEY_DOWN:
		movement.y -= moveSpeed;
		break;

	case GLUT_KEY_LEFT:
		movement.x -= moveSpeed;
		break;

	case GLUT_KEY_RIGHT:
		movement.x += moveSpeed;
		break;

	default:
		break;
	}

	glutPostRedisplay();
}

GLvoid Animate(int val)
{
	xRotate += xRotateDir;
	yRotate += yRotateDir;

	glutTimerFunc(100, Animate, 0);
	glutPostRedisplay();
}

void InitTexture()
{
	int width[6], height[6], numberOfChannel;
	unsigned char* textureData[6];
	// 텍스처 읽기
	stbi_set_flip_vertically_on_load(true);
	textureData[0] = stbi_load("judy1.jpg", &width[0], &height[0], &numberOfChannel, 0);
	textureData[1] = stbi_load("judy2.jpg", &width[1], &height[1], &numberOfChannel, 0);
	textureData[2] = stbi_load("judy3.jpg", &width[2], &height[2], &numberOfChannel, 0);
	textureData[3] = stbi_load("molru3.jpg", &width[3], &height[3], &numberOfChannel, 0);
	textureData[4] = stbi_load("molru4.jpg", &width[4], &height[4], &numberOfChannel, 0);
	textureData[5] = stbi_load("molru5.jpg", &width[5], &height[5], &numberOfChannel, 0);



	glGenTextures(6, texture);
	for (int i = 0; i<6; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, 3, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[i]);
		stbi_image_free(textureData[i]);
	}
	

	glUseProgram(s_program);
	int tLocation = glGetUniformLocation(s_program, "outTexture");
	glUniform1i(tLocation, 0);
}