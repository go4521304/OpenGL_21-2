#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <list>

struct RGBA
{
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;
};

struct Rect
{
	GLfloat pos[2] = { 0.0f, 0.0f };
	GLfloat init_pos[2] = { 0.0f, 0.0f };
	GLfloat dir[2] = { 0.1f, 0.1f };
	RGBA rgba = { 0.1f, 0.1f, 0.1f, 1.0f };
};

void convert_glpos(int x, int y, float& new_X, float& new_Y)
{
	int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);

	new_X = (float)((x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0)));
	new_Y = -(float)((y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0)));
}


GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Animation(int val);
GLvoid Reset_pos();


std::random_device rd;
std::default_random_engine gen(rd());
std::uniform_real_distribution<float> urd(0.0, 1.0);

std::list<Rect> rect;
GLfloat rect_sz[4] = { 0.1f, 0.1f , 0.01f, -0.01f};

bool cmd_anim = false, cmd_c = false;
const int T = 50;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0);	// �������� ��ġ ����
	glutInitWindowSize(800, 600);	// �������� ũ�� ����
	glutCreateWindow("Example4");	// ������ ����(������ �̸�)

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene);	// ��� �ݹ��Լ��� ����
	glutReshapeFunc(Reshape);	// �ٽ� �׸��� �ݹ��Լ� ����
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();				// �̺�Ʈ ó�� ����
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	//--- ����� ���� ����
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);	// �������� ����
	glClear(GL_COLOR_BUFFER_BIT);								// ������ ������ ��ü�� ĥ�ϱ�

	// �簢�� �����
	for (const Rect& rc : rect)
	{
		glColor3f(rc.rgba.red, rc.rgba.green, rc.rgba.blue);
		glRectf(rc.pos[0] - rect_sz[0], rc.pos[1] + rect_sz[1], rc.pos[0] + rect_sz[0], rc.pos[1] - rect_sz[1]);
	}

	glutSwapBuffers();						// ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		if (rect.size() >= 5)
			rect.pop_front();

		Rect tmp;

		convert_glpos(x, y, tmp.init_pos[0], tmp.init_pos[1]);
		tmp.pos[0] = tmp.init_pos[0];
		tmp.pos[1] = tmp.init_pos[1];
		tmp.rgba = { urd(gen) , urd(gen) , urd(gen) , 1.0f };

		rect.push_back(tmp);
	}

	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'A':
	case 'a':
		if (rect.size() > 0)
		{
			if (cmd_anim == false)
				glutTimerFunc(T, Animation, T);
			cmd_anim = true;
		}
		break;

	case 'S':
	case 's':
		cmd_anim = false;
		cmd_c = false;
		break;

	case 'C':
	case 'c':
		if (rect.size() > 0 && cmd_anim)
		{
			cmd_c = true;
		}
		break;

	case 'M':
	case 'm':
		Reset_pos();
		rect_sz[0] = 0.1f;
		rect_sz[1] = 0.1f;
		break;

	case 'R':
	case 'r':
		rect.clear();
		rect_sz[0] = 0.1f;
		rect_sz[1] = 0.1f;
		cmd_anim = false;
		cmd_c = false;

	case 'Q':
	case 'q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid Animation(int val)
{
	for (Rect& rc : rect)
	{
		rc.pos[0] += rc.dir[0];
		rc.pos[1] += rc.dir[1];

		if (-1.0 > rc.pos[0] || rc.pos[0] > 1.0)
			rc.dir[0] *= -1;

		if (-1.0 > rc.pos[1] || rc.pos[1] > 1.0)
			rc.dir[1] *= -1;
	}

	if (cmd_c)
	{
		rect_sz[0] += rect_sz[2];
		rect_sz[1] += rect_sz[3];

		if (rect_sz[0] < 0.01f || rect_sz[0] > 0.5f)
			rect_sz[2] *= -1;
		if (rect_sz[1] < 0.01f || rect_sz[1] > 0.3f)
			rect_sz[3] *= -1;
	}

	glutPostRedisplay();

	if (cmd_anim || cmd_c)
		glutTimerFunc(val, Animation, val);
}

GLvoid Reset_pos()
{
	for (Rect& rc : rect)
	{
		rc.pos[0] = rc.init_pos[0];
		rc.pos[1] = rc.init_pos[1];
	}
}