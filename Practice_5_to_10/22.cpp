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

const float angleVal = 1.0f, moveVal = 0.01f;

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

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);

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

glm::vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
float cameraAngle = 0.0f;

bool isCamMoveX = false, isCamMoveY = false, isCamMoveZ = false, isCamRotate = false;
float camXdir = -1.0f, camZdir = -1.0f, camRotatedir = -1.0f;

bool isDoorOpen = false;
float doorAngle = 0.0f;

bool isMoving = false, isJumping = false;
float r_swingAngle = 0.0f, r_swingDir = -1.0f, r_lookAngle = 0.0f, jumpingTime = 0.0f;
glm::vec3 r_position = { 0.0f, 0.0f, 0.0f };

glm::vec3 box_scale = { 0.5f, 0.1f, 0.5f }, box_position = { 0.5f, 0.0f, 0.5f };

default_random_engine dre;
uniform_real_distribution<float> rd(-0.8, 0.8f);


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
		cout << "Z: 카메라 Z축 방향 이동" << endl;
		cout << "Y: 카메라 화면중심 기준 Y축 회전" << endl;
		cout << "O: 문 열기 / 닫기" << endl;
		cout << "WASD: 이동" << endl;
		cout << "J: 점프" << endl;
		cout << "I: 모든 움직임 초기화" << endl;
		cout << "Q: 종료" << endl;
	}

	box_position.x = rd(dre);
	box_position.y = 0.0f;
	box_position.z = rd(dre);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

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

	m_model = glm::translate(m_model, { 0.0f, -1.0f, 0.0f });

	// stage
	glm::mat4 m_stage = glm::mat4(1.0f);
	m_stage = glm::scale(m_stage, { 2.0f, 2.0f, 2.0f });

	m_stage = m_model * m_stage;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_stage));
	glDrawArrays(GL_TRIANGLES, 6, 30);

	glm::mat4 m_door = glm::mat4(1.0f);
	m_door = glm::translate(m_door, { 0.0f, 0.0f, 1.0f });
	m_door = glm::rotate(m_door, glm::radians(doorAngle), { 1.0f, 0.0f, 0.0f });
	m_door = glm::translate(m_door, { 0.0f, 0.0f, -1.0f });
	m_door = glm::scale(m_door, { 2.0f, 2.0f, 2.0f });
	m_door = m_model * m_door;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_door));
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glm::mat4 m_box = glm::mat4(1.0f);
	m_box = m_model * m_box;
	m_box = glm::translate(m_box, box_position);
	m_box = glm::scale(m_box, box_scale);
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_box));
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// robot
	const glm::vec3 legScale = { 0.01f, 0.1f, 0.01f };
	const glm::vec3 bodyScale = { 0.15f, 0.2f, 0.1f };
	const glm::vec3 bodyPos = { 0.0f, 0.1f, 0.0f };
	const glm::vec3 handPos = { 0.0f, 0.26f, 0.0f };
	const glm::vec3 headScale = { 0.08f, 0.08f, 0.08f };
	const glm::vec3 headPos = { 0.0f, 0.3f, 0.0f };

	glm::mat4 m_robot = glm::mat4(1.0f);
	m_robot = glm::translate(m_robot, r_position);
	m_robot = glm::rotate(m_robot, glm::radians(r_lookAngle), { 0.0f, 1.0f, 0.0f });

	// legs
	glm::mat4 m_legL = glm::mat4(1.0f);
	m_legL = glm::translate(m_legL, { -0.03f, legScale.y, 0.0f });
	m_legL = glm::rotate(m_legL, glm::radians(180 + r_swingAngle), { 1.0f, 0.0f, 0.0f });
	m_legL = glm::scale(m_legL, legScale);
	m_legL = m_model * m_robot * m_legL;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_legL));
	glDrawArrays(GL_TRIANGLES, 72, 36);

	glm::mat4 m_legR = glm::mat4(1.0f);
	m_legR = glm::translate(m_legR, { 0.03f, legScale.y, 0.0f });
	m_legR = glm::rotate(m_legR, glm::radians(180 + (-1 * r_swingAngle)), { 1.0f, 0.0f, 0.0f });
	m_legR = glm::scale(m_legR, legScale);
	m_legR = m_model * m_robot * m_legR;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_legR));
	glDrawArrays(GL_TRIANGLES, 108, 36);

	// body
	glm::mat4 m_body = glm::mat4(1.0f);
	m_body = glm::translate(m_body, bodyPos);
	m_body = glm::scale(m_body, bodyScale);
	m_body = m_model * m_robot * m_body;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_body));
	glDrawArrays(GL_TRIANGLES, 36, 36);

	// hands
	glm::mat4 m_handL = glm::mat4(1.0f);
	m_handL = glm::translate(m_handL, { -bodyScale.x * 0.7f, handPos.y, 0.0f });
	m_handL = glm::rotate(m_handL, glm::radians(180 + (-1 * r_swingAngle)), { 1.0f, 0.0f, 0.0f });
	m_handL = glm::scale(m_handL, legScale);
	m_handL = m_model * m_robot * m_handL;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_handL));
	glDrawArrays(GL_TRIANGLES, 108, 36);

	glm::mat4 m_handR = glm::mat4(1.0f);
	m_handR = glm::translate(m_handR, { bodyScale.x * 0.7f, handPos.y, 0.0f });
	m_handR = glm::rotate(m_handR, glm::radians(180 + r_swingAngle), { 1.0f, 0.0f, 0.0f });
	m_handR = glm::scale(m_handR, legScale);
	m_handR = m_model * m_robot * m_handR;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_handR));
	glDrawArrays(GL_TRIANGLES, 72, 36);

	// head
	glm::mat4 m_head = glm::mat4(1.0f);
	m_head = glm::translate(m_head, headPos);
	m_head = glm::scale(m_head, headScale);
	m_head = m_model * m_robot * m_head;
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_head));
	glDrawArrays(GL_TRIANGLES, 36, 36);

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
	// 앞 왼 오 뒤 위 아래
	const glm::vec3 cubeColor[] = { {1.0f, 0.87f, 0.93}, {0.87f, 0.93f, 1.0f}, {0.87f, 1.0f, 0.87f}, {0.8f, 0.8f, 0.8f}, {1.0f, 0.87f, 0.93}, {1.0f, 1.0f, 1.0f} };
	const glm::vec3 robotColor[] = { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
	vector<glm::vec3> vPos;
	vector<glm::vec3> vColor;

	int n = 0, index = 0;
	for (const auto& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(cubeColor[index]);
		n++;
		if (n % 6 == 0)
			index++;
	}

	for (int j = 0; j < 3; ++j)
	{
		for (const auto& i : cubeIndex)
		{
			vPos.push_back(Cube[i]);
			vColor.push_back(robotColor[j]);
		}
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

	case 'o':
	case 'O':
		isDoorOpen = !isDoorOpen;
		break;

	case 'w':
	case 'W':
		isMoving = true;
		r_lookAngle = 180.0f;
		break;

	case 's':
	case 'S':
		isMoving = true;
		r_lookAngle = 0.0f;
		break;

	case 'a':
	case 'A':
		isMoving = true;
		r_lookAngle = 270.0f;
		break;

	case 'd':
	case 'D':
		isMoving = true;
		r_lookAngle = 90.0f;
		break;

	case 'j':
	case 'J':
		if (jumpingTime == 0.0f && isJumping == false)
		{
			isJumping = true;
			jumpingTime = 30.0f;
		}
		break;

	case 'i':
	case 'I':
		cameraTarget = { 0.0f, 0.0f, 0.0f };
		cameraAngle = 0.0f;

		isCamMoveX = false;
		isCamMoveY = false;
		isCamMoveZ; false;
		isCamRotate = false;
		camXdir = -1.0f;
		camZdir = -1.0f;
		camRotatedir = -1.0f;

		isDoorOpen = false;
		doorAngle = 0.0f;

		isMoving = false;
		isJumping = false;
		r_swingAngle = 0.0f;
		r_swingDir = -1.0f;
		r_lookAngle = 0.0f;
		jumpingTime = 0.0f;
		r_position = { 0.0f, 0.0f, 0.0f };

		box_position.x = rd(dre);
		box_position.y = 0.0f;
		box_position.z = rd(dre);

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

	case 'w':
	case 'W':
	case 's':
	case 'S':
	case 'a':
	case 'A':
	case 'd':
	case 'D':
		isMoving = false;
		break;
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

	if (isDoorOpen == true && doorAngle != 100.0f)
	{
		doorAngle += angleVal;
	}
	else if (isDoorOpen == false && doorAngle != 0.0f)
	{
		doorAngle -= angleVal;
	}

	if (isMoving == true)
	{
		glm::vec3 backPosition = r_position;

		r_swingAngle += angleVal * r_swingDir;
		if (r_lookAngle == 0.0f)
			r_position.z += moveVal;
		else if (r_lookAngle == 180.0f)
			r_position.z -= moveVal;
		else if (r_lookAngle == 90.0f)
			r_position.x += moveVal;
		else if (r_lookAngle == 270.0f)
			r_position.x -= moveVal;

		if ((box_position.x - (box_scale.x / 2) < r_position.x && r_position.x < box_position.x + (box_scale.x / 2)) &&
			(box_position.z - (box_scale.z / 2) < r_position.z && r_position.z < box_position.z + (box_scale.z / 2)) &&
			r_position.y == 0.0f)
		{
			r_position.x = backPosition.x;
			r_position.z = backPosition.z;
		}
	}
	else if (isMoving == false && r_swingAngle != 0.0f)
	{
		r_swingAngle -= angleVal * r_swingDir;
	}

	if (r_swingAngle > 30.0f || r_swingAngle < -30.0f)
		r_swingDir *= -1.0f;

	if (r_position.x > 1.0f)
		r_position.x = -1.0f;
	else if (r_position.x < -1.0f)
		r_position.x = 1.0f;

	if (r_position.z > 1.0f)
		r_position.z = -1.0f;
	else if (r_position.z < -1.0f)
		r_position.z = 1.0f;


	if (isJumping)
	{
		jumpingTime -= 1.0f;

		if (jumpingTime > 15.0f)
			r_position.y += moveVal * 3;

		if (jumpingTime <= 0.0f)
			isJumping = false;
	}
	if (jumpingTime <= 15.0f)
	{
		if (r_position.y <= 0.0f)
		{
			r_position.y = 0.0f;
			jumpingTime = 0.0f;
			isJumping = false;
		}
		else if ((box_position.x - (box_scale.x / 2) < r_position.x && r_position.x < box_position.x + (box_scale.x / 2)) &&
			(box_position.z - (box_scale.z / 2) < r_position.z && r_position.z < box_position.z + (box_scale.z / 2)) &&
			(box_position.y - box_scale.y <= r_position.y && r_position.y <= box_position.y + box_scale.y))
		{
			r_position.y = box_position.y + box_scale.y;
			jumpingTime = 0.0f;
			isJumping = false;
		}
		else
		{
			r_position.y -= moveVal * 3;
		}
	}

	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}