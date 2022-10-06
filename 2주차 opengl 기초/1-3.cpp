#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>

struct RGBA
{
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat alpha;
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
GLvoid Motion(int x, int y);

std::random_device rd;
std::default_random_engine gen(rd());
std::uniform_real_distribution<float> urd(0.0, 1.0);

RGBA color = { 0.0f , 1.0f , 0.0f , 1.0f };
RGBA back = { 0.1f, 0.1f, 0.1f , 1.0f };

GLfloat rect_pos[2] = { 0.0f, 0.0f };
GLfloat mouse_pos[2] = { 0.0f, 0.0f };
bool L_btn = false;
bool motion = false;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0);	// �������� ��ġ ����
	glutInitWindowSize(800, 600);	// �������� ũ�� ����
	glutCreateWindow("Example3");	// ������ ����(������ �̸�)

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
	glutMotionFunc(Motion);
	glutMainLoop();				// �̺�Ʈ ó�� ����
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	//--- ����� ���� ����
	glClearColor(back.red, back.green, back.blue, back.alpha);	// �������� ����
	glClear(GL_COLOR_BUFFER_BIT);								// ������ ������ ��ü�� ĥ�ϱ�


	glColor3f(color.red, color.green, color.blue);
	glRectf(rect_pos[0] - 0.2f, rect_pos[1] + 0.2f, rect_pos[0] + 0.2f, rect_pos[1] - 0.2f);

	glutSwapBuffers();						// ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	convert_glpos(x, y, mouse_pos[0], mouse_pos[1]);

	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		if (rect_pos[0] - 0.2f < mouse_pos[0] && mouse_pos[0] < rect_pos[0] + 0.2f
			&& rect_pos[1] - 0.2f < mouse_pos[1] && mouse_pos[1] < rect_pos[1] + 0.2f)
			L_btn = true;
		else
			L_btn = false;
	}

	if (state == GLUT_UP)
	{
		if (motion == false)
		{
			if (rect_pos[0] - 0.2f < mouse_pos[0] && mouse_pos[0] < rect_pos[0] + 0.2f
				&& rect_pos[1] - 0.2f < mouse_pos[1] && mouse_pos[1] < rect_pos[1] + 0.2f)
				color = { urd(gen), urd(gen) , urd(gen), 1.0f };
			else
				back = { urd(gen) , urd(gen) , urd(gen) , 1.0f };
		}

		L_btn = false;
		motion = false;
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y)
{
	if (L_btn)
	{
		motion = true;

		GLfloat tmp_xy[2];
		convert_glpos(x, y, tmp_xy[0], tmp_xy[1]);

		rect_pos[0] += (tmp_xy[0] - mouse_pos[0]);
		rect_pos[1] += (tmp_xy[1] - mouse_pos[1]);

		convert_glpos(x, y, mouse_pos[0], mouse_pos[1]);
	}
	glutPostRedisplay();
}