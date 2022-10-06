#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include <gl/glew.h> // 필요한 헤더파일 include
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

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;
GLuint vao, vbo[2];

float vRC[][2] = {
	{-0.5f, +0.5f},
	{+0.5f, +0.5f},
	{+0.5f, -0.5f},
	{-0.5f, -0.5f}
};

const float vRC_clr[] = {
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f
};

bool Btn_left = false, Click_Vert= false;
int Index_vert;
float mouse_pos[2];

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice11");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
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

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);
	set_vert();

	glDrawArrays(GL_LINE_LOOP, 0, 4);

	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h)		//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}


/******************************세이더 프로그램 만들기*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_pr5.vert");

	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);

	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	GLchar* fragmentSource = filetobuf("fragment_pr5.frag");

	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);

	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- 버텍스 세이더 만들기
	make_fragmentShader(); //--- 프래그먼트 세이더 만들기

	//-- shader Program
	s_program = glCreateProgram();

	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);

	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//--- Shader Program 사용하기
	glUseProgram(s_program);
}
/**************************************************************************************/
/**************************************************************************************/

void set_vert()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vRC), vRC, GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vRC_clr), vRC_clr, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	set_vert();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		Btn_left = true;

		float new_X, new_Y;
		convert_glpos(x, y, new_X, new_Y);

		for (int i = 0; i < 4; i++)
		{
			if (abs(new_X - vRC[i][0]) < 0.05f && abs(new_Y - vRC[i][1]) < 0.05f)
			{
				Click_Vert = true;
				Index_vert = i;
				break;
			}
		}

		if (Click_Vert == false)
		{
			int cross = 0;
			for (int i = 0; i < 4; ++i)
			{
				int j = (i + 1) % 4;

				if ((new_Y < vRC[i][1]) != (new_Y < vRC[j][1]))
				{
					float check_x = ((new_Y - vRC[j][1]) * (vRC[j][0] - vRC[i][0]) / (vRC[j][1] - vRC[i][1])) + vRC[j][0];

					if (new_X < check_x)
						cross++;
				}
			}

			if (cross % 2 == 0)
				Btn_left = false;
		}

		mouse_pos[0] = new_X;
		mouse_pos[1] = new_Y;
	}

	else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
	{
		Btn_left = false;
		Click_Vert = false;
	}
}

GLvoid Motion(int x, int y)
{
	if (Btn_left)
	{
		float new_X, new_Y;
		convert_glpos(x, y, new_X, new_Y);

		if (Click_Vert)
		{
			vRC[Index_vert][0] = new_X;
			vRC[Index_vert][1] = new_Y;
		}

		else
		{
			for (int i = 0; i < 4; ++i)
			{
				vRC[i][0] += (new_X - mouse_pos[0]);
				vRC[i][1] += (new_Y - mouse_pos[1]);
			}
			mouse_pos[0] = new_X;
			mouse_pos[1] = new_Y;
		}
	}
	glutPostRedisplay();
}