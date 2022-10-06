#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>

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
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

void Mouse(int button, int state, int x, int y);
void Key(unsigned char key, int x, int y);


GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;
GLuint vao, vbo[2];
GLuint vao_rc, vbo_rc[2];

enum class dir
{
	Down = 0, Up = 1, Left = 2, Right = 3
};

// for RECT
const float rc[] = {
	-0.4f, +0.4f, 
	+0.4f, +0.4f,
	+0.4f, -0.4f,
	-0.4f, -0.4f,
	};
const float rc_clr[] = { 
	1.0f, 0.0f, 0.0f, 
	0.0f, 0.0f, 0.0f , 
	0.0f, 0.0f, 1.0f , 
	0.0f, 0.0f, 0.0f 
};

// for Triangle

float vPos[4][2] = {
	0.0f, 0.3f,
	0.0f, -0.3f,
	-0.7f, 0.0f,
	0.7f, 0.0f
};
float velocity[4][2] = {
	{+0.01f, 0.0f},
	{-0.01f, 0.0f},
	{+0.01f, -0.01f},
	{-0.01f, +0.01f}
};
float vColor[4][9] = {
	0.45f, 0.3f, 0.15f,
	0.45f, 0.3f, 0.15f,
	0.45f, 0.3f, 0.15f,

	0.7f, 0.75f, 0.0f,
	0.7f, 0.75f, 0.0f,
	0.7f, 0.75f, 0.0f,

	0.8f, 0.19f, 0.22f,
	0.8f, 0.19f, 0.22f,
	0.8f, 0.19f, 0.22f,

	0.0f, 0.27f, 0.62f,
	0.0f, 0.27f, 0.62f,
	0.0f, 0.27f, 0.62f
};

dir direction[4] = { dir::Down, dir::Up, dir::Up, dir::Up };

// 방향에 따른 삼각형의 모양
float vShape[4][6] = {
	{-0.1f, +0.1f, 0.0f, -0.1f, +0.1f, +0.1f},
	{-0.1f, -0.1f, 0.0f, +0.1f, +0.1f, -0.1f},
	{-0.1f, -0.12f, -0.1f, +0.12f, +0.05f, 0.0f},
	{-0.05f, 0.0f, +0.1f, +0.12f, +0.1f, -0.12f}
};

bool InRect[] = { true, true, false, false };

int index = 0;


std::random_device rd;
std::default_random_engine gen(rd());
std::uniform_real_distribution<float> urd(0.0, 1.0);

const int Time = 10;

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice7");

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
	glutKeyboardFunc(Key);
	glutTimerFunc(Time, Animate, Time);
	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	set_vert();

	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 12);

	glBindVertexArray(vao_rc);
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
	// 삼각형 vao 바인드
	glBindVertexArray(vao);

	vector<float> vert;
	for (int i = 0; i < 4; ++i)
	{
		vert.push_back(vPos[i][0] + vShape[static_cast<int>(direction[i])][0]);
		vert.push_back(vPos[i][1] + vShape[static_cast<int>(direction[i])][1]);
		vert.push_back(vPos[i][0] + vShape[static_cast<int>(direction[i])][2]);
		vert.push_back(vPos[i][1] + vShape[static_cast<int>(direction[i])][3]);
		vert.push_back(vPos[i][0] + vShape[static_cast<int>(direction[i])][4]);
		vert.push_back(vPos[i][1] + vShape[static_cast<int>(direction[i])][5]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vColor), vColor, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}

void InitBuffer()
{
	// 삼각형 vao
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기
	
	// 사각형 vao
	glGenVertexArrays(1, &vao_rc);
	glGenBuffers(2, vbo_rc);


	// 사각형은 한번만 지정해주면 되므로 여기서 바로 데이터를 넘겨줌
	// 사각형 vao 바인드
	glBindVertexArray(vao_rc);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_rc[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rc), rc, GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_rc[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rc_clr), rc_clr, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	set_vert();
}

GLvoid Animate(int val)
{
	for (int i = 0; i < 4; ++i)
	{
		vPos[i][0] += velocity[i][0];
		vPos[i][1] += velocity[i][1];

		if (InRect[i])
		{
			if (-0.4f > vPos[i][0] || 0.4f < vPos[i][0])
			{
				velocity[i][0] *= -1;

				vColor[i][0] = urd(gen);
				vColor[i][1] = urd(gen);
				vColor[i][2] = urd(gen);

				vColor[i][3] = vColor[i][0];
				vColor[i][4] = vColor[i][1];
				vColor[i][5] = vColor[i][2];

				vColor[i][6] = vColor[i][0];
				vColor[i][7] = vColor[i][1];
				vColor[i][8] = vColor[i][2];
			}
			continue;
		}

		else
		{
			if (-0.4f <= vPos[i][0] && vPos[i][0] <= 0.4f && -0.4f <= vPos[i][1] && vPos[i][1] <= 0.4f)
			{
				float past_x = vPos[i][0] - velocity[i][0];
				float past_y = vPos[i][1] - velocity[i][1];

				// 왼쪽 혹은 오른쪽으로 튕길예정
				if (-0.4f <= past_y && past_y <= 0.4f)
				{
					velocity[i][0] *= -1;

					if (past_x < 0.0f)
						direction[i] = dir::Right;
					else
						direction[i] = dir::Left;

				}

				// 위쪽 혹은 아래쪽으로 튕길예정
				else
				{
					velocity[i][1] *= -1;

					if (past_y < 0.0f)
						direction[i] = dir::Down;
					else
						direction[i] = dir::Up;
				}

				continue;
			}


			if (-1.0f > vPos[i][0] || 1.0f < vPos[i][0])
			{
				velocity[i][0] *= -1;

				if (-1.0f > vPos[i][0])
					direction[i] = dir::Left;
				else
					direction[i] = dir::Right;
			}

			if (-1.0f > vPos[i][1] || 1.0f < vPos[i][1])
			{
				velocity[i][1] *= -1;

				if (-1.0f > vPos[i][1])
					direction[i] = dir::Up;
				else
					direction[i] = dir::Down;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(Time, Animate, val);
}

void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		float new_x, new_y;
		convert_glpos(x, y, new_x, new_y);

		vPos[index][0] = new_x;
		vPos[index][1] = new_y;

		if (-0.4f <= vPos[index][0] && vPos[index][0] <= 0.4f && -0.4f <= vPos[index][1] && vPos[index][1] <= 0.4f)
		{
			InRect[index] = true;

			velocity[index][0] = +0.01f;
			velocity[index][1] = 0.0f;

			if (vPos[index][1] < 0)
				direction[index] = dir::Up;
			else
				direction[index] = dir::Down;
		}

		else
		{
			InRect[index] = false;
			velocity[index][0] = +0.01f;
			velocity[index][1] = +0.01f;
			direction[index] = dir::Up;
		}

		index++;
		index %= 4;
	}
	glutPostRedisplay();
}

void Key(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
	case 'A':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case 'b':
	case 'B':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;

	default:
		break;
	}
	glutPostRedisplay();
}