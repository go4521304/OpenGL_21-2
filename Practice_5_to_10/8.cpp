#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

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
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

void InitSet();

void Mouse(int button, int state, int x, int y);
void Key(unsigned char key, int x, int y);


GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;
GLuint vao, vbo;

// �ð��� ����
const int Time = 10;
const float vShape[] = {
	-0.1f, -0.1f, 
	0.0f, +0.1f, 
	+0.1f, -0.1f
};

// ��ü
struct Triangle {
	// �ﰢ���� ���� ��ǥ
	float pos_x, pos_y;

	// +1.0 Ȥ�� -1.0�� ���� ���� ������ ���ϴ� �뵵
	float dir_x, dir_y;

	// Y�� �������� ���� �̵��Ÿ��� ǥ���ϴ� �뵵
	float post_y;

	bool move_y;
};
vector<Triangle> arr_tri;
vector<Triangle> arr2_tri;

bool Move = false;
bool Connect = false;

// ��ü�� �̵��� �ӵ� * dir_��ҷ� ����
// ��ü ��ü�� �帧�� ����
float velocity = 0.01f;


int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice8");

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

	InitSet();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Key);
	glutMainLoop();
}

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	set_vert();

	//--- ����� ���� ����
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, arr_tri.size() * 3);

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
	GLchar* vertexSource = filetobuf("vertex_pr8.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr8.frag");

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
	vector<float> tmp_vert;
	if (Connect)
	{
		for (auto& i : arr2_tri)
		{
			tmp_vert.push_back(i.pos_x + vShape[0]);
			tmp_vert.push_back(i.pos_y + vShape[1]);
			tmp_vert.push_back(i.pos_x + vShape[2]);
			tmp_vert.push_back(i.pos_y + vShape[3]);
			tmp_vert.push_back(i.pos_x + vShape[4]);
			tmp_vert.push_back(i.pos_y + vShape[5]);
		}
	}
	else
	{
		for (auto& i : arr_tri)
		{
			tmp_vert.push_back(i.pos_x + vShape[0]);
			tmp_vert.push_back(i.pos_y + vShape[1]);
			tmp_vert.push_back(i.pos_x + vShape[2]);
			tmp_vert.push_back(i.pos_y + vShape[3]);
			tmp_vert.push_back(i.pos_x + vShape[4]);
			tmp_vert.push_back(i.pos_y + vShape[5]);
		}
	}

	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tmp_vert.size(), tmp_vert.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	set_vert();
}

GLvoid Animate(int val)
{
	for (auto& i : arr_tri)
	{
		// Y������ �̵��ϴ���
		if (i.move_y)
		{

			i.pos_y += i.dir_y * velocity;

			// Y�� �������� ����� �̵��ϸ� ���� 0.0���� �ٲ���
			if (abs(i.pos_y - i.post_y) > abs(vShape[3] * 2.2f))
			{
				i.move_y = false;
				i.pos_y = velocity > 0 ? i.post_y + (vShape[3] * 2.2f * i.dir_y) : i.post_y - (vShape[3] * 2.2f * i.dir_y);
			}

			if (i.pos_y + vShape[1] < -1.0f || i.pos_y + vShape[3] > +1.0f)
			{
				i.pos_y = i.pos_y > 0 ? +1.0f - vShape[3] : -1.0f - vShape[1];
				i.dir_y *= -1;
				i.move_y = false;
			}
			continue;
		}
		// �ƴϸ� X������ �̵�
		else
		{
			i.pos_x += i.dir_x * velocity;

			// �� �浹 ����
			if (i.pos_x + vShape[0] < -1.0f || i.pos_x + vShape[4] > +1.0f)
			{
				i.pos_x = i.pos_x > 0 ? +1.0f - vShape[4] : -1.0f - vShape[0];

				// Y�� �̵��� Ȱ��ȭ �ϰ� �����̱� ������ ��ġ�� ���
				i.move_y = true;
				i.post_y = i.pos_y;

				// X�� �̵� ���� ����
				i.dir_x *= -1;

				// â ���� ������ Y�� �̵� ������ ����
				if (i.pos_y + vShape[1] < -1.0f || i.pos_y + vShape[3] > +1.0f)
					i.dir_y *= -1;
			}
		}
	}

	for (auto& i : arr2_tri)
	{
		// Y������ �̵��ϴ���
		if (i.move_y)
		{
			i.pos_y += i.dir_y * velocity;

			// Y�� �������� ����� �̵��ϸ� ���� 0.0���� �ٲ���
			if (abs(i.pos_y - i.post_y) > abs(vShape[3] * 2.2f))
			{
				i.move_y = false;
				i.pos_y = velocity > 0 ? i.post_y + (vShape[3] * 2.2f * i.dir_y) : i.post_y - (vShape[3] * 2.2f * i.dir_y);
			}

			if (i.pos_y + vShape[1] < -1.0f || i.pos_y + vShape[3] > +1.0f)
			{
				i.pos_y = i.pos_y > 0 ? +1.0f - vShape[3] : -1.0f - vShape[1];
				i.dir_y *= -1;
				i.move_y = false;
			}
			continue;
		}
		// �ƴϸ� X������ �̵�
		else
		{
			i.pos_x += i.dir_x * velocity;

			// �� �浹 ����
			if (i.pos_x + vShape[0] < -1.0f || i.pos_x + vShape[4] > +1.0f)
			{
				i.pos_x = i.pos_x > 0 ? +1.0f - vShape[4] : -1.0f - vShape[0];

				// Y�� �̵��� Ȱ��ȭ �ϰ� �����̱� ������ ��ġ�� ���
				i.move_y = true;
				i.post_y = i.pos_y;

				// X�� �̵� ���� ����
				i.dir_x *= -1;

				// â ���� ������ Y�� �̵� ������ ����
				if (i.pos_y + vShape[1] < -1.0f || i.pos_y + vShape[3] > +1.0f)
					i.dir_y *= -1;
			}
		}
	}

	glutPostRedisplay();

	if (Move)
		glutTimerFunc(Time, Animate, val);
}

void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON && arr_tri.size() < 6)
	{
		Triangle tmp;

		// �̵����� �ƴ�
		if (!Move)
		{
			tmp.pos_x = -0.8f + (0.3f * arr_tri.size());
			tmp.pos_y = +0.8f;
		}

		// �̵���
		else
		{
			tmp.pos_x = +0.8f;
			tmp.pos_y = +0.8f;
		}

		tmp.dir_x = velocity > 0 ? -1.0f : +1.0f;
		tmp.dir_y = velocity > 0 ? -1.0f : +1.0f;

		tmp.post_y = tmp.pos_y;
		tmp.move_y = false;

		arr_tri.push_back(tmp);
	}
	glutPostRedisplay();
}

void Key(unsigned char key, int x, int y)
{
	if (key == 'q' || key == 'Q')
	{
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
	}

	if (arr_tri.size() > 0)
	{
		switch (key)
		{
		case 'M':
		case 'm':
			if (Move == false)
				glutTimerFunc(Time, Animate, Time);
			Move = !Move;
			break;
		
		case 'N':
		case 'n':
			velocity *= -1;
			break;

		case '+':
			if (abs(velocity) < 0.05f)
			{
				if (velocity > 0.0f)
					velocity += 0.01f;
				else
					velocity -= 0.01f;
			}
			cout << velocity << endl;

			break;

		case '-':
			if (abs(velocity) > 0.01f)
			{
				if (velocity > 0.0f)
					velocity -= 0.01f;
				else
					velocity += 0.01f;
			}
			cout << velocity << endl;
			break;

		case 'A':
		case 'a':
			Connect = true;
			break;

		case 'B':
		case 'b':
			Connect = false;
			break;

		default:
			break;
		}
	}
	
	glutPostRedisplay();
}

void InitSet()
{
	Triangle tmp;
	tmp.pos_x = -0.8f;
	tmp.pos_y = +0.8f;
	tmp.dir_x = -1.0f;
	tmp.dir_y = -1.0f;
	tmp.post_y = tmp.pos_y;
	tmp.move_y = false;

	arr2_tri.push_back(tmp);

	for (int i = 1; i < 6; ++i)
	{
		tmp.pos_x = tmp.pos_x + 0.2f;
		arr2_tri.push_back(tmp);
	}
}