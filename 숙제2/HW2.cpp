#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <random>
#include <queue>
#include <iterator>

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

const glm::vec3 CubeNormal[] = {
	{0.0f, 0.0f, 1.0f},
	{-1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, -1.0f, 0.0f}
};

GLvoid drawScene();
GLvoid Reshape(int w, int h);

void make_vertexShader();
void make_fragmentShader();
void InitShader();
void InitObjSet();
void InitSet();

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);

void Animate(int val);

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;


const glm::vec3 boxScale(0.1f, 0.3f, 0.1f);

glm::vec3 cameraTarget = { 0.0f, 0.0f, 0.0f };
float cameraAngle = 0.0f;

bool isCamMoveX = false, isCamMoveY = false, isCamMoveZ = false, isCamRotate = false;
float camXdir = -1.0f, camZdir = -1.0f, camYdir = -1.0f, camRotatedir = -1.0f;

bool isDoorOpen = false;
float doorAngle = 0.0f;

bool isMoving = false, isJumping = false;
float r_swingAngle = 0.0f, r_swingDir = -1.0f, jumpingTime = 0.0f;


GLuint vao, vbo[2];
vector<glm::vec3> vPos, vNormal;

bool isLight = true;
glm::vec3 lightPos = { 0.0f, 0.9f, 0.0f };

class CubeObj
{
public:
	int state;	// 0 cubeObj / 1 rotating obj / -1 disable / 2 변신중

	int frame = 10;

	glm::mat4 m_model;

	glm::vec3 m_scale, m_translation, m_rotation, m_color;

	CubeObj()
	{
		state = 0;
	}

	CubeObj(glm::vec3 mScale, glm::vec3 mTranslation, glm::vec3 mRotation, glm::vec3 mColor) : m_scale(mScale), m_translation(mTranslation), m_rotation(mRotation), m_color(mColor)
	{
		state = 0;
		m_model = glm::mat4(1.0f);
	}

	virtual void update()
	{
		if (state == 2)
		{
			if (frame > 0)
			{
				frame--;
				m_scale -= boxScale / 10.0f;
			}
			else
				state = -1;
		}
		m_model = glm::mat4(1.0f);
		m_model = glm::translate(m_model, m_translation);
		m_model = glm::rotate(m_model, glm::radians(m_rotation.y), { 0.0f, 1.0f, 0.0f });
		m_model = glm::rotate(m_model, glm::radians(m_rotation.z), { 0.0f, 0.0f, 1.0f });
		m_model = glm::rotate(m_model, glm::radians(m_rotation.x), { 1.0f, 0.0f, 0.0f });
		m_model = glm::scale(m_model, m_scale);
	}

	virtual void Render(unsigned int pModel, unsigned int pColor, const glm::mat4& mModel)
	{
		if (state < 0) return;

		m_model = mModel * m_model;
		glUniform3f(pColor, m_color.r, m_color.g, m_color.b);
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_model));

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Stage : public CubeObj
{
public:
	glm::vec3 m_color[3];

	Stage(glm::vec3 mcolor1, glm::vec3 mcolor2, glm::vec3 mcolor3)
	{
		m_color[0] = mcolor1;
		m_color[1] = mcolor2;
		m_color[2] = mcolor3;
	}

	void update() override {}

	void Render(unsigned int pModel, unsigned int pColor, const glm::mat4& mModel) override
	{
		m_model = mModel;

		// stage
		glm::mat4 m_stage = glm::mat4(1.0f);
		m_stage = glm::scale(m_stage, { 2.0f, 2.0f, 2.0f });

		m_stage = m_model * m_stage;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_stage));

		// 앞 왼 오 뒤 위 아래
		glUniform3f(pColor, m_color[0].r, m_color[0].g, m_color[0].b);
		glDrawArrays(GL_TRIANGLES, 6, 12);

		glUniform3f(pColor, m_color[1].r, m_color[1].g, m_color[1].b);
		glDrawArrays(GL_TRIANGLES, 24, 12);

		glUniform3f(pColor, m_color[2].r, m_color[2].g, m_color[2].b);
		glDrawArrays(GL_TRIANGLES, 18, 6);

		glm::mat4 m_door = glm::mat4(1.0f);
		m_door = glm::translate(m_door, { 0.0f, 0.0f, 1.0f });
		m_door = glm::rotate(m_door, glm::radians(doorAngle), { 1.0f, 0.0f, 0.0f });
		m_door = glm::translate(m_door, { 0.0f, 0.0f, -1.0f });
		m_door = glm::scale(m_door, { 2.0f, 2.0f, 2.0f });
		m_door = m_model * m_door;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_door));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

class RotatingCube : public CubeObj
{
public:
	RotatingCube(glm::vec3 mScale, glm::vec3 mTranslation, glm::vec3 mRotation, glm::vec3 mColor) : CubeObj(mScale, mTranslation, mRotation, mColor)
	{
		state = 1;

		m_model = glm::mat4(1.0f);
	}

	void update() override
	{
		m_rotation.y += angleVal;
		CubeObj::update();
	}
};

class Robot : public CubeObj
{
public:
	const glm::vec3 robotColor[3] = { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };

	queue<glm::vec3> position;
	queue<float> lookAngle;

	float r_lookAngle;
	glm::vec3 r_position;

	Robot() : r_position(0.0f, 0.0f, 0.0f), r_lookAngle(0.0f)
	{
	}

	Robot(glm::vec3 mPosition, float mLookAngle)
	{
		r_position = mPosition;
		r_lookAngle = mLookAngle;
	}

	// 저장해둔 정보로 현재 객체 업데이트
	void update() override
	{
		if (position.size() > 20)
		{
			r_position = position.front();
			r_lookAngle = lookAngle.front();

			position.pop();
			lookAngle.pop();
		}
	}

	// 이동과 방향 정보를 저장해놈
	void SetPrevInfo(const Robot* prevRobot)
	{
		position.push(prevRobot->r_position);
		lookAngle.push(prevRobot->r_lookAngle);
	}

	void Render(unsigned int pModel, unsigned int pColor, const glm::mat4& mModel) override
	{
		m_model = mModel;

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
		glUniform3f(pColor, robotColor[2].r, robotColor[2].g, robotColor[2].b);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glm::mat4 m_legR = glm::mat4(1.0f);
		m_legR = glm::translate(m_legR, { 0.03f, legScale.y, 0.0f });
		m_legR = glm::rotate(m_legR, glm::radians(180 + (-1 * r_swingAngle)), { 1.0f, 0.0f, 0.0f });
		m_legR = glm::scale(m_legR, legScale);
		m_legR = m_model * m_robot * m_legR;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_legR));
		glUniform3f(pColor, robotColor[1].r, robotColor[1].g, robotColor[1].b);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// body
		glm::mat4 m_body = glm::mat4(1.0f);
		m_body = glm::translate(m_body, bodyPos);
		m_body = glm::scale(m_body, bodyScale);
		m_body = m_model * m_robot * m_body;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_body));
		glUniform3f(pColor, robotColor[0].r, robotColor[0].g, robotColor[0].b);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// hands
		glm::mat4 m_handL = glm::mat4(1.0f);
		m_handL = glm::translate(m_handL, { -bodyScale.x * 0.7f, handPos.y, 0.0f });
		m_handL = glm::rotate(m_handL, glm::radians(180 + (-1 * r_swingAngle)), { 1.0f, 0.0f, 0.0f });
		m_handL = glm::scale(m_handL, legScale);
		m_handL = m_model * m_robot * m_handL;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_handL));
		glUniform3f(pColor, robotColor[1].r, robotColor[1].g, robotColor[1].b);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glm::mat4 m_handR = glm::mat4(1.0f);
		m_handR = glm::translate(m_handR, { bodyScale.x * 0.7f, handPos.y, 0.0f });
		m_handR = glm::rotate(m_handR, glm::radians(180 + r_swingAngle), { 1.0f, 0.0f, 0.0f });
		m_handR = glm::scale(m_handR, legScale);
		m_handR = m_model * m_robot * m_handR;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_handR));
		glUniform3f(pColor, robotColor[2].r, robotColor[2].g, robotColor[2].b);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// head
		glm::mat4 m_head = glm::mat4(1.0f);
		m_head = glm::translate(m_head, headPos);
		m_head = glm::scale(m_head, headScale);
		m_head = m_model * m_robot * m_head;
		glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_head));
		glUniform3f(pColor, robotColor[0].r, robotColor[0].g, robotColor[0].b);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};


vector<CubeObj*> gameObj;
vector<Robot*> robotObj;
Stage stage({ 0.87f, 0.93f, 1.0f }, { 0.87f, 1.0f, 0.87f }, { 1.0f, 1.0f, 1.0f });


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
		cout << "WASD: 카메라 X축, Z축 방향 이동" << endl;
		cout << "RF: 카메라 Y축 방향 이동" << endl;
		cout << "C: 카메라 화면 중점 기준 회전" << endl << endl;
		cout << "IJKL: 로봇 이동" << endl;
		cout << "V: 로봇 점프" << endl << endl;
		cout << "T: 조명" << endl;
		cout << "2468: 조명 이동 (XZ축)" << endl;
		cout << "+-: 조명 이동 (Y축)" << endl << endl;
		cout << "B: 초기화" << endl;
		cout << "Q: 종료" << endl;
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	InitShader();
	InitObjSet();
	InitSet();

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

	const unsigned int pView = glGetUniformLocation(s_program, "view");
	const unsigned int pProj = glGetUniformLocation(s_program, "proj");
	const unsigned int pModel = glGetUniformLocation(s_program, "model");
	const unsigned int pColor = glGetUniformLocation(s_program, "vColor");
	const unsigned int pLightPos = glGetUniformLocation(s_program, "lightPos");
	const unsigned int pLightColor = glGetUniformLocation(s_program, "lightColor");
	const unsigned int pViewPos = glGetUniformLocation(s_program, "viewPos");

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

	glUniform3f(pViewPos, cameraPos.x, cameraPos.y, cameraPos.z);

	// 빛
	glUniform3f(pLightPos, lightPos.x, lightPos.y, lightPos.z);
	if (isLight)
		glUniform3f(pLightColor, 1.0f, 1.0f, 1.0f);
	else
		glUniform3f(pLightColor, 0.0f, 0.0f, 0.0f);

	glm::mat4 m_light(1.0f);
	m_light = glm::translate(m_light, lightPos);
	m_light = glm::scale(m_light, { 0.05f, 0.05f, 0.05f });
	glUniform3f(pColor, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(pModel, 1, GL_FALSE, glm::value_ptr(m_light));
	glDrawArrays(GL_TRIANGLES, 0, 36);

	m_model = glm::translate(m_model, { 0.0f, -1.0f, 0.0f });

	stage.Render(pModel, pColor, m_model);

	for (const auto& i : gameObj)
		i->Render(pModel, pColor, m_model);
	for (const auto& i : robotObj)
		i->Render(pModel, pColor, m_model);

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
	GLchar* vertexSource = filetobuf("vertex.glsl");

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
	GLchar* fragmentSource = filetobuf("fragment.glsl");

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

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 'W':
		camZdir = -1.0f;
		isCamMoveZ = true;
		break;

	case 's':
	case 'S':
		camZdir = 1.0f;
		isCamMoveZ = true;
		break;

	case 'a':
	case 'A':
		camXdir = -1.0f;
		isCamMoveX = true;
		break;

	case 'd':
	case 'D':
		camXdir = 1.0f;
		isCamMoveX = true;
		break;

	case 'r':
	case 'R':
		camYdir = 1.0f;
		isCamMoveY = true;
		break;

	case 'f':
	case 'F':
		camYdir = -1.0f;
		isCamMoveY = true;
		break;


	case 'c':
	case 'C':
		isCamRotate = true;
		break;

	case 'o':
	case 'O':
		isDoorOpen = !isDoorOpen;
		break;

	case 'i':
	case 'I':
		isMoving = true;
		robotObj.front()->r_lookAngle = 180.0f;
		break;

	case 'k':
	case 'K':
		isMoving = true;
		robotObj.front()->r_lookAngle = 0.0f;
		break;

	case 'j':
	case 'J':
		isMoving = true;
		robotObj.front()->r_lookAngle = 270.0f;
		break;

	case 'l':
	case 'L':
		isMoving = true;
		robotObj.front()->r_lookAngle = 90.0f;
		break;

	case 'v':
	case 'V':
		if (jumpingTime == 0.0f && isJumping == false)
		{
			isJumping = true;
			jumpingTime = 30.0f;
		}
		break;

	case '8':
		lightPos.z -= moveVal * 2;
		break;

	case '2':
		lightPos.z += moveVal * 2;
		break;

	case '4':
		lightPos.x -= moveVal * 2;
		break;

	case '6':
		lightPos.x += moveVal * 2;
		break;

	case '+':
		lightPos.y += moveVal * 2;
		break;

	case '-':
		lightPos.y -= moveVal * 2;
		break;

	case 't':
	case 'T':
		isLight = !isLight;
		break;

	case 'b':
	case 'B':
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
		jumpingTime = 0.0f;
		
		InitObjSet();

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
	case 'w':
	case 'W':
	case 's':
	case 'S':
		isCamMoveZ = false;
		break;

	case 'a':
	case 'A':
	case 'd':
	case 'D':
		isCamMoveX = false;
		break;

	case 'r':
	case 'R':
	case 'f':
	case 'F':
		isCamMoveY = false;
		break;

	case 'c':
	case 'C':
		isCamRotate = false;
		camRotatedir *= -1.0f;
		break;

	case 'i':
	case 'I':
	case 'k':
	case 'K':
	case 'j':
	case 'J':
	case 'l':
	case 'L':
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
	if (isCamMoveY)
	{
		cameraTarget.y += camYdir * moveVal;
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
		glm::vec3 backPosition = robotObj.front()->r_position;

		r_swingAngle += angleVal * r_swingDir;
		if (robotObj.front()->r_lookAngle == 0.0f)
			robotObj.front()->r_position.z += moveVal;
		else if (robotObj.front()->r_lookAngle == 180.0f)
			robotObj.front()->r_position.z -= moveVal;
		else if (robotObj.front()->r_lookAngle == 90.0f)
			robotObj.front()->r_position.x += moveVal;
		else if (robotObj.front()->r_lookAngle == 270.0f)
			robotObj.front()->r_position.x -= moveVal;

		for (const auto& i : gameObj)
		{
			if ((i->m_translation.x - (i->m_scale.x) < robotObj.front()->r_position.x && robotObj.front()->r_position.x < i->m_translation.x + (i->m_scale.x)) &&
				(i->m_translation.z - (i->m_scale.z) < robotObj.front()->r_position.z && robotObj.front()->r_position.z < i->m_translation.z + (i->m_scale.z)) &&
				i->m_translation.y - (i->m_scale.y / 2) < robotObj.front()->r_position.y && robotObj.front()->r_position.y < i->m_translation.y + (i->m_scale.y / 2))
			{
				if (i->state == 1)
				{
					robotObj.front()->r_position.x = backPosition.x;
					robotObj.front()->r_position.z = backPosition.z;
				}
				else if (i->state == 0)
				{
					Robot* newRobot = new Robot(robotObj.front()->r_position, robotObj.front()->r_lookAngle);
					robotObj.push_back(newRobot);
					i->state = 2;
				}
			}
		}

		if (backPosition != robotObj.front()->r_position)
		{
			auto it = robotObj.begin();
			++it;
			while (it != robotObj.end())
			{
				(*it)->SetPrevInfo(*(it - 1));

				(*it)->update();
				++it;
			}
		}
	}
	else if (isMoving == false && r_swingAngle != 0.0f)
	{
		r_swingAngle -= angleVal * r_swingDir;
	}

	if (r_swingAngle > 30.0f || r_swingAngle < -30.0f)
		r_swingDir *= -1.0f;

	if (robotObj.front()->r_position.x > 1.0f)
		robotObj.front()->r_position.x = -1.0f;
	else if (robotObj.front()->r_position.x < -1.0f)
		robotObj.front()->r_position.x = 1.0f;

	if (robotObj.front()->r_position.z > 1.0f)
		robotObj.front()->r_position.z = -1.0f;
	else if (robotObj.front()->r_position.z < -1.0f)
		robotObj.front()->r_position.z = 1.0f;


	if (isJumping)
	{
		jumpingTime -= 1.0f;

		if (jumpingTime > 15.0f)
			robotObj.front()->r_position.y += moveVal * 3;

		if (jumpingTime <= 0.0f)
			isJumping = false;
	}
	if (jumpingTime <= 15.0f)
	{
		if (robotObj.front()->r_position.y <= 0.0f)
		{
			robotObj.front()->r_position.y = 0.0f;
			jumpingTime = 0.0f;
			isJumping = false;
		}
		else
		{
			robotObj.front()->r_position.y -= moveVal * 3;
		}
	}

	for (const auto& i : gameObj)
		i->update();


	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}

void InitObjSet()
{
	
	std::random_device rd;
	std::default_random_engine gen(rd());
	uniform_real_distribution<float> rdPos(-0.8, 0.8f);
	uniform_int_distribution<int> rdCnt(4, 10);

	int cntRotateObj = rdCnt(gen), cntCube = rdCnt(gen);

	gameObj.clear();
	robotObj.clear();

	for (int i = 0; i < cntCube; ++i)
	{
		CubeObj* newCube = new CubeObj(boxScale, { rdPos(gen), 0.0f, rdPos(gen) }, { 0.0f, 0.0f, 0.0f }, { 0.8f, 0.5f, 0.5f });
		gameObj.push_back(newCube);
	}
	for (int i = 0; i < cntRotateObj; ++i)
	{
		RotatingCube* newCube = new RotatingCube(boxScale, { rdPos(gen), 0.0f, rdPos(gen) }, { 10.0f, 0.0f, 10.0f }, { 0.5f, 0.5f, 0.8f });
		gameObj.push_back(newCube);
	}

	robotObj.push_back(new Robot());
}

void InitSet()
{
	const int cubeIndex[36] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	int n = 0, index = 0;
	for (const auto& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vNormal.push_back(CubeNormal[index]);
		n++;
		if (n % 6 == 0)
			index++;
	}

	glGenBuffers(3, vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vNormal.size() * sizeof(glm::vec3), vNormal.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
}