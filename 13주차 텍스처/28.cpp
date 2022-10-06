#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

#include <gl/glew.h> // �ʿ��� ������� include
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

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;

GLuint vao, vbo[3];

vector<glm::vec3> vPos;
vector<glm::vec3> vNormal;
vector<glm::vec2> vTexCoord;

// �ε��� ����Ʈ�� �����غ���

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
// �� �� �� �� �Ʒ�
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

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice14");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n\n";
		std::cout << "C/P: �� ����\n";
		std::cout << "W: �� ä���\n";
		std::cout << "Q: ����\n";

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

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{


	//--- ����� ���� ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_vert();

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);
	// ��ȯ ��� ����

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(movement, 0.0f));
	model = glm::rotate(model, (float)glm::radians(-xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));

	unsigned int modelMat = glGetUniformLocation(s_program, "modelTransform");

	// ���� ����
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

	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}


/******************************���̴� ���α׷� �����*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_pr28.glsl");

	//--- ���ؽ� ���̴� ��ü �����
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexShader);

	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	GLchar* fragmentSource = filetobuf("fragment_pr28.glsl");

	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentShader);

	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- ���ؽ� ���̴� �����
	make_fragmentShader(); //--- �����׸�Ʈ ���̴� �����

	//-- shader Program
	s_program = glCreateProgram();

	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);

	//--- ���̴� �����ϱ�
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
	// �ؽ�ó �б�
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