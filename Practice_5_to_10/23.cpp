#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <random>

#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

using namespace std;

const float moveVal = 0.01f;
const int angleVal = 5;

int SCR_WIDTH = 1200, SCR_HEIGHT = 800;

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

// 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7
// 앞 왼 오 뒤 위 아래
glm::vec3 Cube[] = {
	{-0.5f, +0.5f, +0.5f},
	{+0.5f, +0.5f, +0.5f},
	{-0.5f, -0.5f, +0.5f},
	{+0.5f, -0.5f, +0.5f},
	{-0.5f, +0.5f, -0.5f},
	{+0.5f, +0.5f, -0.5f},
	{-0.5f, -0.5f, -0.5f},
	{+0.5f, -0.5f, -0.5f}
};

GLvoid drawScene();
GLvoid Reshape(int w, int h);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void MouseClick(int button, int state, int x, int y);
void Mouse(int x, int y);

void Animate(int val);

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo;

glm::vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
float cameraAngle = 0.0f;

bool isCamMoveX = false, isCamMoveY = false, isCamMoveZ = false, isCamRotate = false;
float camXdir = -1.0f, camZdir = -1.0f, camRotatedir = -1.0f;

const glm::vec3 cubeColor[] = { {1.0f, 0.87f, 0.93}, {0.87f, 0.93f, 1.0f}, {0.87f, 1.0f, 0.87f}, {0.8f, 0.8f, 0.8f}, {1.0f, 0.87f, 0.93}, {1.0f, 1.0f, 1.0f} };

bool isMouseClick = false;
float pastXpos = 0.0f;
int boxAngle = 0;
glm::vec3 smallBoxPos[3] = { {0.0f, -0.85f, -0.5}, {0.0f, -0.9f, 0.0f}, {0.0f, -0.95f, 0.5f} };
glm::vec3 smallBoxScale[3] = { { 0.3f, 0.3f, 0.3f } , { 0.2f, 0.2f, 0.2f } ,{ 0.1f, 0.1f, 0.1f } };

int smallBoxAngle = 0, boxState = 0;
bool isOpen = false;

struct ball
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
	bool active = false;
};

ball balls[5] = {
	{{0.3f, -0.1f, 0.7f}, {-1.0f, 2.0f, 1.0f}, false},
	{{-0.5f, 0.0f, 0.1f}, {-2.0f, 3.0f, 1.0f}, false},
	{{-0.25f, 0.3f, 0.6f}, {1.0f, 1.0f, -2.0f}, false},
	{{0.0f, -0.7f, 0.5f}, {1.0f, -4.0f, 2.0f}, false},
	{{0.0f, 0.1f, 0.7f}, {5.0f, 5.0f, 5.0f}, false}
};

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutCreateWindow("Practice23");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n\n";
		cout << "X: 카메라 x축 방향 이동" << endl;
		cout << "Z: 카메라 Z축 방향 이동" << endl;
		cout << "Y: 카메라 화면중심 기준 Y축 회전" << endl;
		cout << "B: 공 튀기기" << endl;
		cout << "O: 바닥 열기" << endl;
		cout << "Q: 종료" << endl;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutMouseFunc(MouseClick);
	glutMotionFunc(Mouse);
	glutTimerFunc(10, Animate, 0);
	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(s_program);
	glBindVertexArray(vao);

	unsigned int pView = glGetUniformLocation(s_program, "view");
	unsigned int pProj = glGetUniformLocation(s_program, "proj");
	unsigned int pModel = glGetUniformLocation(s_program, "model");
	unsigned int PColor = glGetUniformLocation(s_program, "vColor");

	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	glm::mat4 m_model = glm::mat4(1.0f);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glm::vec3 cameraPos = { 5.0f * glm::sin(glm::radians(cameraAngle)), 0.0f, 5.0f * glm::cos(glm::radians(cameraAngle)) };
	cameraPos += cameraTarget;
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	m_view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	glUniformMatrix4fv(pView, 1, GL_FALSE, glm::value_ptr(m_view));
	glUniformMatrix4fv(pProj, 1, GL_FALSE, glm::value_ptr(m_proj));

	m_model = glm::rotate(m_model, glm::radians((float)boxAngle), { 0.0f, 0.0f, 1.0f });
	glm::mat4 m_box = glm::mat4(1.0f);
	m_box = glm::scale(m_box, { 2.0f, 2.0f, 2.0f });
	m_box = m_model * m_box;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_box));

	int drawCube = 6;
	if (isOpen)
		drawCube = 5;
	for (int i = 0; i < drawCube; ++i)
	{
		glUniform3f(PColor, cubeColor[i].r, cubeColor[i].g, cubeColor[i].b);
		glDrawArrays(GL_TRIANGLES, i * 6, 6);
	}

	glUniform3f(PColor, 1.0f, 0.0f, 0.0f);
	for (int i = 0; i < 3; ++i)
	{
		glm::mat4 m_smallBox = glm::mat4(1.0f);
		m_smallBox = glm::rotate(m_smallBox, glm::radians((float)boxAngle), { 0.0f, 0.0f, 1.0f });
		m_smallBox = glm::translate(m_smallBox, smallBoxPos[i]);
		m_smallBox = glm::scale(m_smallBox, smallBoxScale[i]);
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_smallBox));
		glDrawArrays(GL_TRIANGLES, 36, 36);
	}

	glUniform3f(PColor, 0.0f, 0.0f, 1.0f);
	GLUquadricObj* planet = gluNewQuadric();

	glm::mat4 m_ball;
	for (int i = 0; i < 5; ++i)
	{
		if (balls[i].active == true)
		{
			m_ball = glm::mat4(1.0f);
			m_ball = glm::translate(m_ball, balls[i].position);
			m_ball = m_model * m_ball;
			glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_ball));
			gluSphere(planet, 0.03, 20, 20);
		}
	}

	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h)		//--- 콜백 함수: 다시 그리기 콜백 함수
{
	SCR_WIDTH = w;
	SCR_HEIGHT = h;
}


/******************************세이더 프로그램 만들기*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_pr18.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr18.frag");

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
}
/**************************************************************************************/
/**************************************************************************************/

void set_vert()
{
	const int cubeBackIndex[] = { 2,0,1, 3,2,1, 6,4,0, 2,6,0, 3,1,5, 7,3,5, 7,5,4, 6,7,4, 0,4,5, 1,0,5, 6,2,3, 7,6,3 };
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	vector<glm::vec3> vPos;
	for (const auto& i : cubeBackIndex)
	{
		vPos.push_back(Cube[i]);
	}

	for (const auto& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
	}

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "vPos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	set_vert();
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'X':
	case 'x':
		isCamMoveX = true;
		break;

	case 'Y':
	case 'y':
		isCamRotate = true;
		break;

	case 'Z':
	case 'z':
		isCamMoveZ = true;
		break;

	case 'b':
	case 'B':
		for (int i = 0; i < 5; ++i)
			if (balls[i].active == false)
			{
				balls[i].active = true;
				break;
			}
		break;

	case 'o':
	case 'O':
		isOpen = !isOpen;
		break;

	case 'Q':
	case 'q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;
	}
}

void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'X':
	case 'x':
		isCamMoveX = false;
		camXdir *= -1.0f;
		break;

	case 'Y':
	case 'y':
		isCamRotate = false;
		camRotatedir *= -1.0f;
		break;

	case 'Z':
	case 'z':
		isCamMoveZ = false;
		camZdir *= -1.0f;
		break;
	}
}

void MouseClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			isMouseClick = true;
			pastXpos = x;
		}
		else if (state == GLUT_UP)
			isMouseClick = false;
	}
}
void Mouse(int x, int y)
{
	if (x > pastXpos)
	{
		boxAngle -= angleVal;
		smallBoxAngle -= angleVal;
	}
	else if (x <= pastXpos)
	{
		boxAngle += angleVal;
		smallBoxAngle += angleVal;
	}
	smallBoxAngle = smallBoxAngle % 90;

	if (abs(smallBoxAngle) == 45)
	{
		if (smallBoxAngle > 0)
			smallBoxAngle = -45;
		else
			smallBoxAngle = 45;
	}

	int tmpAngle = boxAngle % 360;

	if ((-45 <= tmpAngle && tmpAngle < 45) || (315 <= tmpAngle || tmpAngle <= -315))
	{
		boxState = 0;
	}
	else if ((45 <= tmpAngle && tmpAngle < 135) || (-225 >= tmpAngle && tmpAngle > -315))
	{
		boxState = 1;
	}
	else if ((135 <= tmpAngle && tmpAngle < 225) || (-135 > tmpAngle && tmpAngle >= -225))
	{
		boxState = 2;
	}
	else
	{
		boxState = 3;
	}
}

void Animate(int val)
{
	if (isCamMoveX)
	{
		cameraTarget.x += camXdir * moveVal;
	}
	if (isCamMoveZ)
	{
		cameraTarget.z += camZdir * moveVal;
	}
	if (isCamRotate)
	{
		cameraAngle += camRotatedir;
	}

	for (int i = 0; i < 5; ++i)
	{
		if (balls[i].active == true)
		{
			balls[i].position += balls[i].direction * moveVal;

			if (1.0f <= balls[i].position.x || -1.0f >= balls[i].position.x)
				balls[i].direction.x *= -1;
			if (1.0f <= balls[i].position.y || (-1.0f >= balls[i].position.y && !isOpen))
				balls[i].direction.y *= -1;
			if (1.0f <= balls[i].position.z || -1.0f >= balls[i].position.z)
				balls[i].direction.z *= -1;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		if (0 <= smallBoxAngle)
		{
			if (boxState == 0)
			{
				if (smallBoxPos[i].x - (smallBoxScale[i].x / 2) > -1.0f)
					smallBoxPos[i].x -= moveVal;
				if (smallBoxPos[i].y - (smallBoxScale[i].y / 2) > -1.0f || isOpen)
					smallBoxPos[i].y -= moveVal;
			}
			else if (boxState == 1)
			{
				if (smallBoxPos[i].x - (smallBoxScale[i].x / 2) > -1.0f)
					smallBoxPos[i].x -= moveVal;
				if (smallBoxPos[i].y + (smallBoxScale[i].y / 2) < 1.0f)
					smallBoxPos[i].y += moveVal;
			}
			else if (boxState == 2)
			{
				if (smallBoxPos[i].x + (smallBoxScale[i].x / 2) < 1.0f)
					smallBoxPos[i].x += moveVal;
				if (smallBoxPos[i].y + (smallBoxScale[i].y / 2) < 1.0f)
					smallBoxPos[i].y += moveVal;
			}
			else
			{
				if (smallBoxPos[i].x + (smallBoxScale[i].x / 2) < 1.0f )
					smallBoxPos[i].x += moveVal;
				if (smallBoxPos[i].y - (smallBoxScale[i].y / 2) > -1.0f)
					smallBoxPos[i].y -= moveVal;
			}
		}

		if (0 >= smallBoxAngle)
		{
			if (boxState == 0)
			{
				if (smallBoxPos[i].x + (smallBoxScale[i].x / 2) < 1.0f)
					smallBoxPos[i].x += moveVal;
				if (smallBoxPos[i].y - (smallBoxScale[i].y / 2) > -1.0f || isOpen)
					smallBoxPos[i].y -= moveVal;
			}
			else if (boxState == 1)
			{
				if (smallBoxPos[i].x - (smallBoxScale[i].x / 2) > -1.0f)
					smallBoxPos[i].x -= moveVal;
				if (smallBoxPos[i].y - (smallBoxScale[i].y / 2) > -1.0f)
					smallBoxPos[i].y -= moveVal;
			}
			else if (boxState == 2)
			{
				if (smallBoxPos[i].x - (smallBoxScale[i].x / 2) > -1.0f)
					smallBoxPos[i].x -= moveVal;
				if (smallBoxPos[i].y + (smallBoxScale[i].y / 2) < 1.0f)
					smallBoxPos[i].y += moveVal;
			}
			else
			{
				if (smallBoxPos[i].x + (smallBoxScale[i].x / 2) < 1.0f)
					smallBoxPos[i].x += moveVal;
				if (smallBoxPos[i].y + (smallBoxScale[i].y / 2) < 1.0f)
					smallBoxPos[i].y += moveVal;
			}
		}
	}


	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}