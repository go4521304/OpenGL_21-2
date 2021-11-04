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
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;
GLuint vao, vbo[2];

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> rColor(0.0f, 1.0f);
uniform_real_distribution<float> rPos(-1.0f, 1.0f);

struct Position {
	float x, y;
};
struct Color {
	float r, g, b;
};
vector<Position> vPos;
vector<Color> vColor;

bool Btn_left = false;

GLfloat Point_Size = 20;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice12");

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
	glutMotionFunc(Motion);
	glutMainLoop();
}

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);
	set_vert();

	if (Btn_left)
	{
		glPointSize(Point_Size);
		glDrawArrays(GL_POINTS, 0, vPos.size() - 1);

		glPointSize(Point_Size * 2);
		glDrawArrays(GL_POINTS, vPos.size() - 1, 1);
	}
	else
	{
		glPointSize(Point_Size);
		glDrawArrays(GL_POINTS, 0, vPos.size());
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
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(Position), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vColor.size() * sizeof(Color), vColor.data(), GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}

void InitBuffer()
{
	for (int i = 0; i < 100; ++i)
	{
		vPos.emplace_back(Position{ rPos(dre), rPos(dre) });
		vColor.emplace_back(Color{rColor(dre), rColor(dre) , rColor(dre) });
	}

	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	set_vert();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_UP)
		{
			Btn_left = false;
			vPos.pop_back();
			vColor.pop_back();
		}
		else if (state == GLUT_DOWN)
		{
			Btn_left = true;
			float new_X, new_Y;
			convert_glpos(x, y, new_X, new_Y);

			while (1)
			{
				auto check = find_if(vPos.cbegin(), vPos.cend(), [&new_X, &new_Y](const Position& pos) {
					return (abs(new_X - pos.x) < (Point_Size / 350) && abs(new_Y - pos.y) < (Point_Size / 350));
					});
				if (check == vPos.cend())
					break;

				else
				{
					int dist = distance(vPos.cbegin(), check);
					vColor.erase(vColor.begin() + dist);
					vPos.erase(check);
				}
			}

			vPos.emplace_back(Position{ new_X, new_Y });
			vColor.emplace_back(Color{ 0.0f, 0.0f, 0.0f });
		}
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y)
{
	vPos.pop_back();

	float new_X, new_Y;
	convert_glpos(x, y, new_X, new_Y);

	if (Btn_left)
	{
		while (1)
		{
			auto check = find_if(vPos.cbegin(), vPos.cend(), [&new_X, &new_Y](const Position& pos) {
				return (abs(new_X - pos.x) < (Point_Size / 350) && abs(new_Y - pos.y) < (Point_Size / 350));
				});
			if (check == vPos.cend())
				break;

			else
			{
				int dist = distance(vPos.cbegin(), check);
				vColor.erase(vColor.begin() + dist);
				vPos.erase(check);
			}
		}
	}

	vPos.emplace_back(Position{ new_X, new_Y });
	glutPostRedisplay();
}