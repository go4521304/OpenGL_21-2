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
const glm::vec3 Cube[] = {
	{-0.5f, 1.0f, +0.5f},
	{+0.5f, 1.0f, +0.5f},
	{-0.5f, 0.0f, +0.5f},
	{+0.5f, 0.0f, +0.5f},
	{-0.5f, 1.0f, -0.5f},
	{+0.5f, 1.0f, -0.5f},
	{-0.5f, 0.0f, -0.5f},
	{+0.5f, 0.0f, -0.5f}
};
const glm::vec3 CubeColor[] = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 0.0f},
	{1.0f, 1.0f, 1.0f}
};

GLvoid drawScene();
GLvoid Reshape(int w, int h);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();
void InitSet();

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Animate(int val);

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo;

int WN, HN;
glm::vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
float cameraAngle = 0.0f, cameraAngleX = -60.0f;

bool isCamRotateX = false, isCamMoveZ = false, isCamRotate = false;
float camRotatedirX = -1.0f, camZdir = -1.0f, camRotatedir = -1.0f;

struct Pillar {
	float height = 0.0f;
	float maxHeight = 0.0f, minHeight = 0.0f;
	float speed = 0.0f;
};

int max[100][100] = { 0 };
Pillar pillars[100][100];
float sizeW, sizeH;
bool isMoving = false;

bool isPerspective = true, isDownP = false;;
int maze[20][20] = { 0 };
bool isMaze = false;
bool viewSet = false;

float angleVal = 1.0f, moveVal = 0.05f;

struct P
{
	float x = 0.0f, y = 0.0f;
	glm::vec3 look = { 0.0f, 0.0f, 1.0f };
	bool isActive = false;
};

P player;

void playerMove(unsigned char key);

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutCreateWindow("HW1");

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

		cout << "가로: ";
		cin >> WN;
		cout << "세로: ";
		cin >> HN;

		InitSet();

		cout << endl << endl;;

		cout << "X: 카메라 x축 방향 회전" << endl;
		cout << "Y: 카메라 y축 방향 회전" << endl;
		cout << "z: 카메라 z축 이동" << endl;
		cout << "o/p: 원근/직교투영" << endl;
		cout << "r: 미로제작" << endl;
		cout << "s: 객체생성" << endl;
		cout << "v: 낮은높이로" << endl;
		cout << "wasd: 객체이동" << endl;
		cout << "+/-: 속도" << endl;
		cout << "c: 초기화" << endl;

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
	unsigned int pColor = glGetUniformLocation(s_program, "vColor");

	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	glm::mat4 m_model = glm::mat4(1.0f);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	if (viewSet == false)
	{
		glm::vec3 cameraPos = { 1.0f * glm::sin(glm::radians(cameraAngleX)) * glm::cos(glm::radians(cameraAngle)), 1.0f * glm::cos(glm::radians(cameraAngleX)),  1.0f * glm::sin(glm::radians(cameraAngleX)) * glm::sin(glm::radians(cameraAngle)) };

		cameraPos = cameraPos * 20.0f;
		cameraPos += cameraTarget;

		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
		cameraRight.z = glm::cos(glm::radians(cameraAngle));
		cameraRight = glm::normalize(cameraRight);
		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

		m_view = glm::lookAt(cameraPos, cameraDirection, cameraUp);

		if (isPerspective)
			m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		else
			m_proj = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}
	else
	{

	}

	glUniformMatrix4fv(pView, 1, GL_FALSE, glm::value_ptr(m_view));
	glUniformMatrix4fv(pProj, 1, GL_FALSE, glm::value_ptr(m_proj));

	const float floorY = -5.0f;
	// 바닥
	glUniform3f(pColor, 0.2f, 0.2f, 0.2f);
	m_model = glm::translate(m_model, { 0.0f, floorY - 0.1f, 0.0f });
	m_model = glm::scale(m_model, { 10.0f, 0.1f, 10.0f });
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model));
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// 기둥
	glUniform3f(pColor, 0.8f, 0.8f, 0.8f);

	for (int i = 0; i < WN; ++i)
	{
		for (int j = 0; j < HN; ++j)
		{
			if (isMaze == false || (isMaze && maze[i][j] == 0))
			{
				m_model = glm::mat4(1.0f);
				m_model = glm::translate(m_model, { sizeW * i - 5.0f + (sizeW / 2), floorY, sizeH * j - 5.0f + (sizeH / 2) });
				if (isDownP)
					m_model = glm::scale(m_model, { sizeW, 1.0f, sizeH });
				else
					m_model = glm::scale(m_model, { sizeW, pillars[i][j].height, sizeH });
				glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model));
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			
		}
	}

	if (player.isActive && isMaze)
	{
		m_model = glm::mat4(1.0f);
		m_model = glm::translate(m_model, { player.x, -4.7f, player.y });
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model));

		glUniform3f(pColor, 1.0f, 0.0f, 0.0f);
		GLUquadricObj* planet = gluNewQuadric();
		gluSphere(planet, 0.2, 20, 20);

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
	GLchar* vertexSource = filetobuf("vertex.vert");

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
	GLchar* fragmentSource = filetobuf("fragment.frag");

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
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	vector<glm::vec3> vPos;

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

void InitSet()
{
	random_device rd;
	default_random_engine gen(rd());
	uniform_real_distribution<float> dis(1.0f, 10.0f);

	sizeW = 10.0f / WN;
	sizeH = 10.0f / HN;
	for (int i = 0; i < WN; ++i)
	{
		for (int j = 0; j < HN; ++j)
		{
			float height = dis(gen);
			pillars[i][j].height = height;
			pillars[i][j].maxHeight = height + dis(gen);
			float minHeight = 0.0f;
			while (minHeight > pillars[i][j].height)
				minHeight = dis(gen);
			pillars[i][j].minHeight = height - minHeight;
			pillars[i][j].speed = dis(gen);
		}
	}

	int pointX = 0, pointY = 0;
	uniform_int_distribution<int> rd2(1, 4);
	maze[pointX][pointY] = 1;
	while (1)
	{
		if (pointX == WN - 1 && pointY == HN - 1)
			break;

		bool ok = false;
		while (ok == false)
		{
			int sel = rd2(gen);
			ok = true;
			if (sel == 1 && pointX != 0)
				pointX--;
			else if (sel == 2 && pointX != WN - 1)
				pointX++;
			else if (sel == 3 && pointY != 0)
				pointY--;
			else if (sel == 4 && pointY != HN - 1)
				pointY++;
			else
				ok = false;
		}

		maze[pointX][pointY] = 1;
	}
	player.isActive = false;
	player.look = { 0.0f, 0.0f, 1.0f };
	player.x = -5.0f + (sizeW / 2);
	player.y = -5.0f + (sizeH / 2);
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'o':
	case 'O':
		isPerspective = false;
		break;

	case 'p':
	case 'P':
		isPerspective = true;
		break;

	case 'X':
	case 'x':
		isCamRotateX = true;
		break;

	case 'Y':
	case 'y':
		isCamRotate = true;
		break;

	case 'Z':
	case 'z':
		isCamMoveZ = true;
		break;

	case 'm':
	case 'M':
		isMoving = !isMoving;
		break;

	case 'r':
	case 'R':
		isMaze = !isMaze;
		break;

	case 'c':
	case 'C':
		InitSet();
		break;

	case 'v':
	case 'V':
		isDownP = !isDownP;
		break;

	case '+':
		if (moveVal < 0.1f)
			moveVal += 0.01f;
		break;

	case '-':
		if (moveVal > 0.02f)
			moveVal -= 0.01f;
		else
			moveVal = 0.01f;
		break;

	case 't':
	case 'T':
		player.isActive = !player.isActive;
		break;

	case 'w':
	case 'W':
	case 's':
	case 'S':
	case 'a':
	case 'A':
	case 'd':
	case 'D':
		playerMove(key);
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
		isCamRotateX = false;
		camRotatedirX *= -1.0f;
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

void Animate(int val)
{
	if (isCamRotateX)
	{
		cameraAngleX += camRotatedirX;
	}
	if (isCamMoveZ)
	{
		cameraTarget.x += camZdir * moveVal;
	}
	if (isCamRotate)
	{
		cameraAngle += camRotatedir;
	}

	if (isMoving)
	{
		for (int i = 0; i < WN; ++i)
		{
			for (int j = 0; j < HN; ++j)
			{
				pillars[i][j].height += pillars[i][j].speed * moveVal;

				if (pillars[i][j].height >= pillars[i][j].maxHeight || pillars[i][j].height <= pillars[i][j].minHeight)
					pillars[i][j].speed *= -1.0f;
			}
		}
	}

	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}

void playerMove(unsigned char key)
{
	float tmpX = player.x;
	float tmpY = player.y;
	if (key == 'w' || key == 'W')
	{
		tmpX += 0.1f;
	}
	if (key == 's' || key == 'S')
	{
		tmpX -= 0.1f;
	}
	if (key == 'a' || key == 'A')
	{
		tmpY -= 0.1f;
	}
	if (key == 'd' || key == 'D')
	{
		tmpY += 0.1f;
	}
	
	bool check = false;
	for (int i = 0; i < WN; ++i)
	{
		for (int j = 0; j < HN; ++j)
		{
			if (maze[i][j] == 1 && sizeW * i - 5.0f< tmpX - 0.02f && tmpX + 0.02f < sizeW * i - 5.0f + (sizeW / 2) && sizeH * j - 5.0f< tmpY - 0.02f && tmpY + 0.02f < sizeH * j - 5.0f + (sizeH / 2))
			{
				check = true;
			}
		}
	}

	if (!check)
	{
		player.x = tmpX;
		player.y = tmpY;
	}
}