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

void get_glpos(int x, int y, float& new_X, float& new_Y)
{
	int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);

	new_X = (float)((x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0)));
	new_Y = -(float)((y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0)));
}


GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);

std::random_device rd;
std::default_random_engine gen(rd());
std::uniform_real_distribution<float> urd(0.0, 1.0);

RGBA color_rect = { urd(gen) , urd(gen) , urd(gen) , 1.0f };
RGBA color_back = { urd(gen) , urd(gen) , urd(gen) , 1.0f };



int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0);	// �������� ��ġ ����
	glutInitWindowSize(800, 600);	// �������� ũ�� ����
	glutCreateWindow("Example2");	// ������ ����(������ �̸�)

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

	glutMainLoop();				// �̺�Ʈ ó�� ����
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	//--- ����� ���� ����
	glClearColor(color_back.red, color_back.green, color_back.blue, color_back.alpha);	// �������� ����
	glClear(GL_COLOR_BUFFER_BIT);			// ������ ������ ��ü�� ĥ�ϱ�

	
	glColor3f(color_rect.red, color_rect.green, color_rect.blue);
	glRectf(-0.5, 0.5, 0.5, -0.5);

	glutSwapBuffers();						// ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		GLfloat X, Y;

		get_glpos(x, y, X, Y);

		std::cout << X << " " << Y << std::endl;

		if (-0.5 < X && X < 0.5f && -0.5f < Y && Y < 0.5f)
			color_rect = { urd(gen), urd(gen) , urd(gen), 1.0f };
		else
			color_back = { urd(gen) , urd(gen) , urd(gen) , 1.0f };
	}
	glutPostRedisplay();

}

