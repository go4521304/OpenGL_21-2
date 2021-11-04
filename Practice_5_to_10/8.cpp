#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include <gl/glew.h> // 필요한 헤더파일 include
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


GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;
GLuint vao, vbo;

// 시간은 고정
const int Time = 10;
const float vShape[] = {
	-0.1f, -0.1f, 
	0.0f, +0.1f, 
	+0.1f, -0.1f
};

// 객체
struct Triangle {
	// 삼각형의 중점 좌표
	float pos_x, pos_y;

	// +1.0 혹은 -1.0의 값을 가짐 방향을 정하는 용도
	float dir_x, dir_y;

	// Y축 방향으로 남은 이동거리를 표시하는 용도
	float post_y;

	bool move_y;
};
vector<Triangle> arr_tri;
vector<Triangle> arr2_tri;

bool Move = false;
bool Connect = false;

// 객체의 이동은 속도 * dir_요소로 계산됨
// 전체 객체의 흐름을 제어
float velocity = 0.01f;


int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice8");

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

	InitSet();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Key);
	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	set_vert();

	//--- 변경된 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, arr_tri.size() * 3);

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
	GLchar* vertexSource = filetobuf("vertex_pr8.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr8.frag");

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
		// Y축으로 이동하는지
		if (i.move_y)
		{

			i.pos_y += i.dir_y * velocity;

			// Y축 방향으로 충분히 이동하면 값을 0.0으로 바꿔줌
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
		// 아니면 X축으로 이동
		else
		{
			i.pos_x += i.dir_x * velocity;

			// 벽 충돌 감지
			if (i.pos_x + vShape[0] < -1.0f || i.pos_x + vShape[4] > +1.0f)
			{
				i.pos_x = i.pos_x > 0 ? +1.0f - vShape[4] : -1.0f - vShape[0];

				// Y축 이동을 활성화 하고 움직이기 시작한 위치를 기록
				i.move_y = true;
				i.post_y = i.pos_y;

				// X축 이동 방향 변경
				i.dir_x *= -1;

				// 창 끝에 닿으면 Y축 이동 방향을 변경
				if (i.pos_y + vShape[1] < -1.0f || i.pos_y + vShape[3] > +1.0f)
					i.dir_y *= -1;
			}
		}
	}

	for (auto& i : arr2_tri)
	{
		// Y축으로 이동하는지
		if (i.move_y)
		{
			i.pos_y += i.dir_y * velocity;

			// Y축 방향으로 충분히 이동하면 값을 0.0으로 바꿔줌
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
		// 아니면 X축으로 이동
		else
		{
			i.pos_x += i.dir_x * velocity;

			// 벽 충돌 감지
			if (i.pos_x + vShape[0] < -1.0f || i.pos_x + vShape[4] > +1.0f)
			{
				i.pos_x = i.pos_x > 0 ? +1.0f - vShape[4] : -1.0f - vShape[0];

				// Y축 이동을 활성화 하고 움직이기 시작한 위치를 기록
				i.move_y = true;
				i.post_y = i.pos_y;

				// X축 이동 방향 변경
				i.dir_x *= -1;

				// 창 끝에 닿으면 Y축 이동 방향을 변경
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

		// 이동중이 아님
		if (!Move)
		{
			tmp.pos_x = -0.8f + (0.3f * arr_tri.size());
			tmp.pos_y = +0.8f;
		}

		// 이동중
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