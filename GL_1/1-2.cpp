#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
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



int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);	// 윈도우의 위치 지정
	glutInitWindowSize(800, 600);	// 윈도우의 크기 지정
	glutCreateWindow("Example2");	// 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene);	// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);	// 다시 그리기 콜백함수 지정
	glutMouseFunc(Mouse);

	glutMainLoop();				// 이벤트 처리 시작
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(color_back.red, color_back.green, color_back.blue, color_back.alpha);	// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);			// 설정된 색으로 전체를 칠하기

	
	glColor3f(color_rect.red, color_rect.green, color_rect.blue);
	glRectf(-0.5, 0.5, 0.5, -0.5);

	glutSwapBuffers();						// 화면에 출력하기
}

GLvoid Reshape(int w, int h)		//--- 콜백 함수: 다시 그리기 콜백 함수
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

