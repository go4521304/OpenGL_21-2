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

std::default_random_engine dre;
std::uniform_real_distribution<double> urd(0.0, 1.0);

GLvoid do_drawScene(GLvoid);
GLvoid do_Reshape(int w, int h);
GLvoid do_Keyboard(unsigned char key, int x, int y);
void do_Timer(int val);

bool flag_timer = false;

class GL_App
{
public:
	RGBA color = {1.0f, 1.0f, 1.0f, 1.0f};

	void drawScene(GLvoid)
	{
		//--- ����� ���� ����
		glClearColor(color.red, color.green, color.blue, color.alpha);	// �������� ����
		glClear(GL_COLOR_BUFFER_BIT);			// ������ ������ ��ü�� ĥ�ϱ�
		glutSwapBuffers();						// ȭ�鿡 ����ϱ�
	}

	void Reshape(int w, int h)
	{
		glViewport(0, 0, w, h);
	}

	void Keyboard(unsigned char key, int x, int y)
	{
		switch (key) {
		case 'R':
		case 'r':
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
			break;	//--- ������ ���������� ����

		case 'G':
		case 'g':
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
			break;	//--- ������ �ʷϻ����� ����

		case 'B':
		case 'b':
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
			break;	//--- ������ �Ķ������� ����

		case 'A':
		case 'a':
			color = { GLfloat(urd(dre)), GLfloat(urd(dre)) , GLfloat(urd(dre)) , 1.0f };
			break;

		case 'W':
		case 'w':
			color = { 1.0f, 1.0f, 1.0f, 1.0f };
			break;

		case 'K':
		case 'k':
			color = { 0.0f, 0.0f, 0.0f, 1.0f };
			break;

		case 'T':
		case 't':
			flag_timer = true;
			glutTimerFunc(100, do_Timer, 0);
			break;

		case 'S':
		case 's':
			flag_timer = false;
			break;

		case 'Q':
		case 'q':
			std::cout << "Bye Bye~!" << std::endl;
			glutLeaveMainLoop();
			exit(0);
			break;
		}
		glutPostRedisplay(); //--- ������ �ٲ𶧸��� ��� �ݹ��Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
	}

	void Timer()
	{
		color = { GLfloat(urd(dre)), GLfloat(urd(dre)) , GLfloat(urd(dre)) , 1.0f };
		glutPostRedisplay();
		if (flag_timer)
			glutTimerFunc(100, do_Timer, 0);
	}
};


GL_App* glptr;


int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0);	// �������� ��ġ ����
	glutInitWindowSize(800, 600);	// �������� ũ�� ����
	glutCreateWindow("Example1");	// ������ ����(������ �̸�)
	
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;		
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	// ������ ����
	GL_App gl_app;
	glptr = &gl_app;

	glutDisplayFunc(do_drawScene);	// ��� �ݹ��Լ��� ����
	glutReshapeFunc(do_Reshape);	// �ٽ� �׸��� �ݹ��Լ� ����
	glutKeyboardFunc(do_Keyboard); // Ű���� �Է� �ݹ��Լ� ����
	glutMainLoop();				// �̺�Ʈ ó�� ����
}

GLvoid do_drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	glptr->drawScene();
}

GLvoid do_Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glptr->Reshape(w, h);
}

GLvoid do_Keyboard(unsigned char key, int x, int y)
{
	glptr->Keyboard(key, x, y);
}

void do_Timer(int val)
{
	glptr->Timer();
}