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

const float angleVal = 1.0f, moveVal = 0.05f;

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
	{-1.0f, 1.0f, +1.0f},
	{+1.0f, 1.0f, +1.0f},
	{-1.0f, 0.0f, +1.0f},
	{+1.0f, 0.0f, +1.0f},
	{-1.0f, 1.0f, -1.0f},
	{+1.0f, 1.0f, -1.0f},
	{-1.0f, 0.0f, -1.0f},
	{+1.0f, 0.0f, -1.0f}
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
void Animate(int val);

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo[2];

enum class STATE
{
	active, dissolve, inactive
};

struct ObjInfo
{
	glm::vec3 pos;
	glm::vec3 size;
	STATE state;
};

glm::vec3 cameraMovement = { 0.0f, 0.0f, 0.0f };
float cameraAngle = 0.0f;

vector<ObjInfo> objects;
glm::vec3 erasePos = {0.0f, 0.0f, 0.0f};
ObjInfo eraser_1 = { {0.0f, 0.05f, 0.0f}, {0.2f, 0.1f, 0.1f} };
ObjInfo eraser_2 = { {0.0f, 0.15f, 0.0f}, {0.1f, 0.2f, 0.1f} };

bool isCamMoveX = false, isCamMoveY = false, isCamMoveZ = false, isCamRotate = false;
float camXdir = -1.0f, camYdir = -1.0f, camZdir = -1.0f, camRotatedir = -1.0f;

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutCreateWindow("Practice17");

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
		cout << "Y: 카메라 y축 방향 이동" << endl;
		cout << "Z: 카메라 z축 방향 이동" << endl;
		cout << "R: 원점 기준 카메라 회전" << endl;
		cout << "C: 모든 움직임 초기화" << endl;
		cout << "Q: 종료" << endl;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	InitShader();
	InitBuffer();
	InitSet();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
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

	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	glm::mat4 m_model = glm::mat4(1.0f);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f) + cameraMovement;
	glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 5.0f) + cameraMovement;
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	m_view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	glUniformMatrix4fv(pView, 1, GL_FALSE, glm::value_ptr(m_view));
	glUniformMatrix4fv(pProj, 1, GL_FALSE, glm::value_ptr(m_proj));
	
	m_model = glm::rotate(m_model, glm::radians(cameraAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	// 좌표계
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model));
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);

	// 바닥면
	glm::mat4 m_floor = glm::mat4(1.0f);
	m_floor = glm::scale(m_floor, { 2.0f, 0.0f, 2.0f });
	m_floor = m_model * m_floor;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_floor));
	glDrawArrays(GL_TRIANGLES, 6, 36);

	// 지우개 메인
	glm::mat4 m_erasePos = glm::mat4(1.0f);
	m_erasePos = glm::translate(m_erasePos, erasePos);

	// 지우개 1단
	glm::mat4 m_erase1 = glm::mat4(1.0f);
	m_erase1 = glm::translate(m_erase1, eraser_1.pos);
	m_erase1 = glm::scale(m_erase1, eraser_1.size);
	m_erase1 = m_model * m_erasePos * m_erase1;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_erase1));
	glDrawArrays(GL_TRIANGLES, 42, 36);

	// 지우개 1단
	glm::mat4 m_erase2 = glm::mat4(1.0f);
	m_erase2 = glm::translate(m_erase2, eraser_2.pos);
	m_erase2 = glm::scale(m_erase2, eraser_2.size);
	m_erase2 = m_model * m_erasePos * m_erase2;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_erase2));
	glDrawArrays(GL_TRIANGLES, 78, 36);

	for (const auto& i : objects)
	{
		if (i.state != STATE::inactive)
		{
			glm::mat4 m_obj = glm::mat4(1.0f);
			m_obj = glm::translate(m_obj, i.pos);
			m_obj = glm::scale(m_obj, i.size);
			m_obj = m_model * m_obj;
			glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_obj));
			glDrawArrays(GL_TRIANGLES, 114, 36);
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
	GLchar* vertexSource = filetobuf("vertex_pr17.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr17.frag");

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
	const glm::vec3 line[] = { {-5.0f, 0.0f, 0.0f}, {5.0f, 0.0f, 0.0f}, {0.0f, -5.0f, 0.0f}, {0.0f, 5.0f, 0.0f}, {0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 5.0f} };
	const glm::vec3 lineColor[] = { {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} };

	vector<glm::vec3> vPos;
	vector<glm::vec3> vColor;

	// X 좌표계 0 / Y 좌표계 2 / Z 좌표계 4
	// 6 / 42 / 78 / 114
	for (const auto& i : line)
		vPos.push_back(i);
	for (const auto& i : lineColor)
		vColor.push_back(i);

	// 6
	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(glm::vec3(0.2f, 0.2f, 0.2f));
	}
	// 42
	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
	}
	// 78
	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	}
	// 114
	const glm::vec3 customCubeColor[] = {
	{1.0f, 0.84f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{1.0f, 0.84f, 0.0f},
	{1.0f, 0.84f, 0.0f},
	{1.0f, 0.84f, 0.0f},
	{1.0f, 0.84f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{1.0f, 0.84f, 0.0f}
	};
	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(customCubeColor[i]);
	}

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "vPos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);


	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vColor.size() * sizeof(glm::vec3), vColor.data(), GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "vColor");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);
	set_vert();
}

void InitSet()
{
	objects.clear();
	random_device rd;
	default_random_engine dre(rd());
	uniform_int_distribution<int> Num(10, 30);
	uniform_real_distribution<float> Pos(-1.8f, 1.8f);
	uniform_real_distribution<float> Size(0.1f, 0.3f);

	int num = Num(dre);
	for (int i = 0; i < num; ++i)
	{
		ObjInfo temp;
		temp.size.x = Size(dre);
		temp.size.y = Size(dre);
		temp.size.z = Size(dre);
		
		temp.pos.x = Pos(dre);
		temp.pos.y = temp.size.y / 2;
		temp.pos.z = Pos(dre);

		temp.state = STATE::active;

		objects.push_back(temp);
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'X':
	case 'x':
		isCamMoveX = true;
		camXdir *= -1.0f;
		break;

	case 'Y':
	case 'y':
		isCamMoveY = true;
		camYdir *= -1.0f;
		break;

	case 'Z':
	case 'z':
		isCamMoveZ = true;
		camZdir *= -1.0f;
		break;

	case 'R':
	case 'r':
		isCamRotate = true;
		camRotatedir *= -1.0f;
		break;

	case 'C':
	case 'c':
		isCamMoveX = false;
		isCamMoveY = false;
		isCamMoveZ = false;
		cameraMovement = { 0.0f, 0.0f, 0.0f };

		isCamRotate = false;
		cameraAngle = 0.0f;
		InitSet();
		break;

	case 'W':
	case 'w':
		erasePos.z -= moveVal;
		break;

	case 'A':
	case 'a':
		erasePos.x -= moveVal;
		break;

	case 'S':
	case 's':
		erasePos.z += moveVal;
		break;

	case 'D':
	case 'd':
		erasePos.x += moveVal;
		break;

	case 'Q':
	case 'q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;

	}
}
void Animate(int val)
{
	if (isCamMoveX)
	{
		cameraMovement.x += camXdir * moveVal;
	}
	if (isCamMoveY)
	{
		cameraMovement.y += camYdir * moveVal;
	}
	if (isCamMoveZ)
	{
		cameraMovement.z += camZdir * moveVal;
	}
	if (isCamRotate)
	{
		cameraAngle += camRotatedir * angleVal;
	}


	for (auto& i : objects)
	{
		if (i.state == STATE::active)
		{
			if ((i.pos.x - (i.size.x) < (erasePos.x - (eraser_1.size.x)) && (erasePos.x - (eraser_1.size.x)) < i.pos.x + (i.size.x)) ||
				(i.pos.x - (i.size.x) < (erasePos.x + (eraser_1.size.x)) && (erasePos.x + (eraser_1.size.x)) < i.pos.x + (i.size.x)))
			{
				if ((i.pos.z - (i.size.z) < (erasePos.z - (eraser_1.size.z)) && (erasePos.z - (eraser_1.size.z)) < i.pos.z + (i.size.z)) ||
					(i.pos.z - (i.size.z) < (erasePos.z + (eraser_1.size.z)) && (erasePos.z + (eraser_1.size.z)) < i.pos.z + (i.size.z)))
				{
					i.state = STATE::dissolve;
				}
			}
		}

		else if (i.state == STATE::dissolve)
		{
			i.size *= 0.9f;
			if (i.size.x < 0.01f && i.size.y < 0.01f && i.size.z < 0.01f)
			{
				i.state = STATE::inactive;
			}
		}
	}
	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}