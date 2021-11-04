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

using namespace std;

const float angleSpeed = 5.0f;

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
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;

GLuint vao, vbo[2];

vector<glm::vec3> vPos;
vector<glm::vec3> vColor;

// �ε��� ����Ʈ�� �����غ���

// 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7
// �� �� �� �� �� �Ʒ�
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
glm::vec3 CubeColor[] = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 0.0f},
	{1.0f, 1.0f, 1.0f}
};

float xRotate_cube = 0.0f, yRotate_cube = 0.0f;
float xRotateDir_cube = 0.0f, yRotateDir_cube = 0.0f;

float xRotate_pyramid = 0.0f, yRotate_pyramid = 180.0f;
float xRotateDir_pyramid = 0.0f, yRotateDir_pyramid = 0.0f;

float coordRotateX = 30.0f, coordRotateY = -30.0f, coordRotateDir = 0.0f;

bool changeShape = true;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice15");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(100, Animate, 0);

	glutMainLoop();
}

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	// ��ȯ ��� ����
	unsigned int modelMat = glGetUniformLocation(s_program, "modelTransform");

	////// ��ǥ�� ���
	glm::mat4 coord = glm::mat4(1.0f);
	coord = glm::rotate(coord, (float)glm::radians(coordRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	coord = glm::rotate(coord, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(coord));
	glDrawArrays(GL_LINES, 0, 6);


	// ����ü ���
	glm::mat4 model1 = glm::mat4(1.0f);
	model1 = glm::rotate(model1, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	model1 = glm::translate(model1, glm::vec3(-0.7f, 0.0f, 0.0f));
	model1 = glm::rotate(model1, (float)glm::radians(yRotate_cube), glm::vec3(0.0f, 1.0f, 0.0f));
	model1 = glm::rotate(model1, (float)glm::radians(coordRotateX + xRotate_cube), glm::vec3(1.0f, 0.0f, 0.0f));

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(model1));
	if (changeShape)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 6, 36);
	}
	else
	{
		GLUquadricObj* qobj1;
		qobj1 = gluNewQuadric();
		gluQuadricDrawStyle(qobj1, GLU_LINE);
		gluSphere(qobj1, 0.3, 20, 20);
	}
	

	// �Ƕ�̵� ���
	glm::mat4 model2 = glm::mat4(1.0f);
	model2 = glm::rotate(model2, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	model2 = glm::translate(model2, glm::vec3(0.7f, 0.0f, 0.0f));
	model2 = glm::rotate(model2, (float)glm::radians(yRotate_pyramid), glm::vec3(0.0f, 1.0f, 0.0f));
	model2 = glm::rotate(model2, (float)glm::radians(coordRotateX + xRotate_pyramid), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(model2));

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_LINE);
	if (changeShape)
		gluCylinder(qobj, 0.3, 0.0, 1.0, 20, 8);
	else
		gluCylinder(qobj, 0.3, 0.3, 1.0, 20, 8);

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
	GLchar* vertexSource = filetobuf("vertex_pr13.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr13.frag");

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
	const glm::vec3 line[] = { {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f} };
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	vPos.clear();
	vColor.clear();

	for (const glm::vec3& i : line)
	{
		vPos.push_back(i);
		vColor.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	}
	int count = 0;
	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(CubeColor[i]);
		count++;
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vColor.size() * sizeof(glm::vec3), vColor.data(), GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	set_vert();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'x':
	case 'X':
		if (xRotateDir_cube == 0.0f)
			xRotateDir_cube = angleSpeed;
		else
			xRotateDir_cube *= -1;
		break;

	case 'y':
	case 'Y':
		if (yRotateDir_cube == 0.0f)
			yRotateDir_cube = angleSpeed;
		else
			yRotateDir_cube *= -1;
		break;

	case 'a':
	case 'A':
		if (xRotateDir_pyramid == 0.0f)
			xRotateDir_pyramid = angleSpeed;
		else
			xRotateDir_pyramid *= -1;
		break;

	case 'b':
	case 'B':
		if (yRotateDir_pyramid == 0.0f)
			yRotateDir_pyramid = angleSpeed;
		else
			yRotateDir_pyramid *= -1; 
		break;

	case 'r':
	case 'R':
		if (coordRotateDir == 0.0f)
			coordRotateDir = angleSpeed;
		else
			coordRotateDir *= -1;
			
		break;

	case 'c':
	case 'C':
		changeShape = !changeShape;
		break;

	case 's':
	case 'S':
		xRotate_cube = 0.0f;
		yRotate_cube = 0.0f;
		xRotateDir_cube = 0.0f;
		yRotateDir_cube = 0.0f;

		xRotate_pyramid = 0.0f;
		yRotate_pyramid = 180.0f;
		xRotateDir_pyramid = 0.0f;
		yRotateDir_pyramid = 0.0f;

		coordRotateX = 30.0f;
		coordRotateY = -30.0f;
		coordRotateDir = 0.0f;
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid Animate(int val)
{
	xRotate_cube += xRotateDir_cube;
	yRotate_cube += yRotateDir_cube;

	xRotate_pyramid += xRotateDir_pyramid;
	yRotate_pyramid += yRotateDir_pyramid;

	coordRotateY += coordRotateDir;

	glutTimerFunc(100, Animate, 0);
	glutPostRedisplay();
}