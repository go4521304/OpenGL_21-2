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
		//--- 변경된 배경색 설정
		glClearColor(color.red, color.green, color.blue, color.alpha);	// 바탕색을 변경
		glClear(GL_COLOR_BUFFER_BIT);			// 설정된 색으로 전체를 칠하기
		glutSwapBuffers();						// 화면에 출력하기
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
			break;	//--- 배경색을 빨강색으로 설정

		case 'G':
		case 'g':
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
			break;	//--- 배경색을 초록색으로 설정

		case 'B':
		case 'b':
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
			break;	//--- 배경색을 파랑색으로 설정

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
		glutPostRedisplay(); //--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
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


int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);	// 윈도우의 위치 지정
	glutInitWindowSize(800, 600);	// 윈도우의 크기 지정
	glutCreateWindow("Example1");	// 윈도우 생성(윈도우 이름)
	
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;		
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	// 포인터 설정
	GL_App gl_app;
	glptr = &gl_app;

	glutDisplayFunc(do_drawScene);	// 출력 콜백함수의 지정
	glutReshapeFunc(do_Reshape);	// 다시 그리기 콜백함수 지정
	glutKeyboardFunc(do_Keyboard); // 키보드 입력 콜백함수 지정
	glutMainLoop();				// 이벤트 처리 시작
}

GLvoid do_drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glptr->drawScene();
}

GLvoid do_Reshape(int w, int h)		//--- 콜백 함수: 다시 그리기 콜백 함수
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