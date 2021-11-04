#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void Mouse(int button, int state, int x, int y);
void Key(unsigned char key, int x, int y);
void set_vert();
void InitBuffer();
void InitShader();


GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;
GLuint vao, vbo[2];

enum class dir
{
	Down = 0, Up = 1, Left = 2, Right = 3
};

float vPos[4][2] = {
	-0.2f, +0.0f,
	+0.9f, +0.1f, 
	-0.7f, -0.3f,
	+0.5f, -0.7f
};
float velocity[4][2] = {
	{+0.01f, +0.01f},
	{-0.01f, -0.01f},
	{+0.01f, -0.01f},
	{-0.01f, +0.01f}
};

float vColor[] = {
	0.45f, 0.3f, 0.15f,
	0.45f, 0.3f, 0.15f,
	0.45f, 0.3f, 0.15f,

	0.7f, 0.75f, 0.0f,
	0.7f, 0.75f, 0.0f,
	0.7f, 0.75f, 0.0f,

	0.15f, 0.1f, 0.35f,
	0.15f, 0.1f, 0.35f,
	0.15f, 0.1f, 0.35f,

	0.2f, 0.62f, 0.17f,
	0.2f, 0.62f, 0.17f,
	0.2f, 0.62f, 0.17f
};
dir direction[4] = { dir::Up, dir::Up, dir::Up, dir::Up };

// ���⿡ ���� �ﰢ���� ���
float vShape[4][6] = {
	{-0.1f, +0.1f, 0.0f, -0.1f, +0.1f, +0.1f},
	{-0.1f, -0.1f, 0.0f, +0.1f, +0.1f, -0.1f},

	{-0.1f, -0.12f, -0.1f, +0.12f, +0.05f, 0.0f},
	{-0.05f, 0.0f, +0.1f, +0.12f, +0.1f, -0.12f}
};

int index = 0;
const int Time = 10;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice6");

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
	glutTimerFunc(Time, Animate, Time);
	glutMainLoop();
}

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	set_vert();

	//--- ����� ���� ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	//--- �ﰢ�� �׸���
	glDrawArrays(GL_TRIANGLES, 0, 12);
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
	GLchar* vertexSource = filetobuf("vertex_pr5.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr5.frag");

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
	for (int i = 0; i < 4; ++i)
	{
		vert.push_back(vPos[i][0] + vShape[static_cast<int>(direction[i])][0]);
		vert.push_back(vPos[i][1] + vShape[static_cast<int>(direction[i])][1]);
		vert.push_back(vPos[i][0] + vShape[static_cast<int>(direction[i])][2]);
		vert.push_back(vPos[i][1] + vShape[static_cast<int>(direction[i])][3]);
		vert.push_back(vPos[i][0] + vShape[static_cast<int>(direction[i])][4]);
		vert.push_back(vPos[i][1] + vShape[static_cast<int>(direction[i])][5]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�

	glGenBuffers(2, vbo); //--- 2���� VBO�� �����ϰ� �Ҵ��ϱ�

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vColor), vColor, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	set_vert();
}

GLvoid Animate(int val)
{
	for (int i = 0; i < 4; ++i)
	{
		vPos[i][0] += velocity[i][0];
		vPos[i][1] += velocity[i][1];

		if (-1.0f > vPos[i][0] || 1.0f < vPos[i][0])
		{
			velocity[i][0] *= -1;

			if (-1.0f > vPos[i][0])
				direction[i] = dir::Left;
			else
				direction[i] = dir::Right;
		}

		if (-1.0f > vPos[i][1] || 1.0f < vPos[i][1])
		{
			velocity[i][1] *= -1;

			if (-1.0f > vPos[i][1])
				direction[i] = dir::Up;
			else
				direction[i] = dir::Down;
		}
	}

	glutPostRedisplay();
	glutTimerFunc(Time, Animate, val);
}

void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		float new_x, new_y;
		convert_glpos(x, y, new_x, new_y);

		vPos[index][0] = new_x;
		vPos[index][1] = new_y;
		direction[index] = dir::Up;
		velocity[index][0] = +0.01f;
		velocity[index][1] = +0.01f;

		index++;
		index %= 4;
	}
	glutPostRedisplay();
}

void Key(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
	case 'A':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case 'b':
	case 'B':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;

	default:
		break;
	}
	glutPostRedisplay();
}