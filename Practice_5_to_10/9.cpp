#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
using namespace std;

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

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

GLvoid Animate(int val);
void Mouse(int button, int state, int x, int y);
void Key(unsigned char key, int x, int y);

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;
GLuint vao, vbo;

std::random_device rd;
std::default_random_engine gen(rd());
std::uniform_real_distribution<float> urd(0.0, 1.0);

const float pi = (float)acos(-1);
const int Time = 10;

int Index = 0, N = 0;
float Center[2] = { 0, 0 };
float Back_color[] = { 0.1f, 0.1f, 0.1f };
bool PointDraw = true;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice9");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Key);
	glutTimerFunc(Time, Animate, 0);
	glutMainLoop();
}

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{

	//--- ����� ���� ����
	glClearColor(Back_color[0], Back_color[1], Back_color[2], 0.1f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	if (PointDraw)
	{
		glPointSize(1.5);
		glDrawArrays(GL_POINTS, 0, Index);
	}
	else
	{
		glDrawArrays(GL_LINES, 0, Index);
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
	GLchar* vertexSource = filetobuf("vertex_pr9.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr9.frag");

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

	//--- Shader Program ����ϱ�
	glUseProgram(s_program);
}
/**************************************************************************************/
/**************************************************************************************/

void set_vert()
{
	vector<float> vert;
	float R = 0.0f;
	float degree = 0.0f;
	float weight, second_weight;
	if (urd(gen) > 0.5f)
	{
		weight = +pi / 18;
		second_weight = -pi;
	}
	else
	{
		weight = -(pi / 18);
		second_weight = +pi;
	}

	for (; abs(degree) < (2 * pi * 3); degree += weight)
	{
		vert.push_back(Center[0] + (R * cos(degree)));
		vert.push_back(Center[1] + (R * sin(degree)));
		R += 0.002f;
	}


	Center[0] = vert.at(vert.size() - 2) + R;
	for (degree = pi; abs(degree) < (2 * pi * 3) + second_weight; degree -= weight)
	{
		vert.push_back(Center[0] + (R * cos(degree)));
		vert.push_back(Center[1] + (R * sin(degree)));
		R -= 0.002f;
	}

	N = vert.size() / 2;
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
	glEnableVertexAttribArray(0);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
}

GLvoid Animate(int val)
{
	if (0 < Index && Index < N)
	{
		++Index;
	}
	glutTimerFunc(Time, Animate, val);
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		float new_X, new_Y;
		convert_glpos(x, y, new_X, new_Y);
		Center[0] = new_X;
		Center[1] = new_Y;

		Index = 1;
		set_vert();

		Back_color[0] = urd(gen);
		Back_color[1] = urd(gen);
		Back_color[2] = urd(gen);
	}
}

void Key(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
	case 'A':
		PointDraw = true;
		break;

	case 'b':
	case 'B':
		PointDraw = false;
		break;

	default:
		break;
	}
	glutPostRedisplay();
}