#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
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

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Animation(int val);
void make_vertexShader();
void make_fragmentShader();
void InitBuffer();
void InitShader();


void Init_Shape_Set();
void set_vert();

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;
GLuint vao, vbo[2], EBO;

struct Vertex {
	float pos_x, pos_y;
	float dir_x, dir_y;
};

const float color[][3] = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},

	{1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},

	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},

	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},

};
const unsigned int Index[] = {
	0, 1, 2,

	3, 4, 5,
	3, 5, 6,

	7, 8, 9,
	7, 9, 10, 
	7, 10, 11, 

	12, 13, 14,
	12, 14, 15,
	12, 15, 16
};

vector<Vertex> vShape;
int Count = 0;
const int Time = 100;

// �̰� �����Ͻ� �ӵ� �����ϽǼ� �ֽ���
const int Play = 10;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice10");

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

	Init_Shape_Set();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glutTimerFunc(Time, Animation, 0);
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
	if (Count == 0)
		glDrawArrays(GL_LINES, 0, 2);
	glDrawElements(GL_TRIANGLES, 27, GL_UNSIGNED_INT, 0);

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

	for (auto& i : vShape)
	{
		vert.push_back(i.pos_x);
		vert.push_back(i.pos_y);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);

	// �ε��� ����Ʈ ����
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index), Index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glGenBuffers(2, vbo); //--- 2���� VBO�� �����ϰ� �Ҵ��ϱ�

	glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�

	// �� VBO ����
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);


	set_vert();
}

void Init_Shape_Set()
{
	// ��
	const float Point[5][2] =
	{
		0.000f, 0.003f,
		0.0028f, 0.0005f,
		0.0018f, -0.003f,
		-0.0018f, -0.003f,
		-0.0028f, 0.0005f
	};

	// ��
	const float Line[3][2] = {
		-0.3f, -0.3f,
		0.0f, +0.3f,
		0.0f, +0.3f,
	};
	
	// �ﰢ��
	const float Triangle[4][2] = {
		-0.3f, -0.3f,
		0.0f, +0.3f,
		0.3f, -0.3f,
		0.3f, -0.3f,
	};

	// �簢��
	const float Rectangle[5][2] = {
		-0.3f, +0.3f,
		+0.3f, +0.3f, 
		+0.3f, -0.3f, 
		-0.3f, -0.3f,
		-0.3f, -0.3f
	};

	// ������
	const float Pentagon[5][2] = {
		0.0f, 0.3f,
		0.28f, 0.05f,
		0.18f, -0.3f,
		-0.18f, -0.3f,
		-0.28f, 0.05f
	};

	Vertex tmp;

	// ��->�ﰢ�� ����
	for (int i = 0; i < 3; ++i)
	{
		tmp.pos_x = -0.5f + Line[i][0];
		tmp.pos_y = +0.5f + Line[i][1];
		tmp.dir_x = ((-0.5f + Triangle[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((+0.5f + Triangle[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}

	// �ﰢ��->�簢�� ����
	for (int i = 0; i < 4; ++i)
	{
		tmp.pos_x = +0.5f + Triangle[i][0];
		tmp.pos_y = +0.5f + Triangle[i][1];
		tmp.dir_x = ((+0.5f + Rectangle[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((+0.5f + Rectangle[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}

	// �簢��->������ ����
	for (int i = 0; i < 5; ++i)
	{
		tmp.pos_x = -0.5f + Rectangle[i][0];
		tmp.pos_y = -0.5f + Rectangle[i][1];
		tmp.dir_x = ((-0.5f + Pentagon[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((-0.5f + Pentagon[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}

	// ������->�� ����
	for (int i = 0; i < 5; ++i)
	{
		tmp.pos_x = +0.5f + Pentagon[i][0];
		tmp.pos_y = -0.5f + Pentagon[i][1];
		tmp.dir_x = ((+0.5f + Point[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((-0.5f + Point[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}
}

GLvoid Animation(int val)
{
	for (auto& i : vShape)
	{
		i.pos_x += i.dir_x;
		i.pos_y += i.dir_y;
	}

	Count++;

	if (Count > Play)
	{
		Count = 0;
		vShape.clear();
		Init_Shape_Set();
	}

	glutTimerFunc(Time, Animation, 0);
	glutPostRedisplay();
}