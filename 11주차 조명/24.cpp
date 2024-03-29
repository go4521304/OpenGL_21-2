#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

using namespace std;
static const float angleSpeed = 10.0f;
static const float moveSpeed = 0.1f;
static const float sideOpenSpeed = 0.05f;

const int SCR_WIDTH = 1200, SCR_HEIGHT = 800;

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
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid KeyUp(unsigned char key, int x, int y);
GLvoid SpecialKey(int key, int x, int y);
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo[2];

vector<glm::vec3> vPos;
vector<glm::vec3> vNormal;


// 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7
// 앞 왼 오 뒤 위 아래
glm::vec3 Cube[] = {
	{-0.25f, +0.25f, +0.25f},
	{+0.25f, +0.25f, +0.25f},
	{-0.25f, -0.25f, +0.25f},
	{+0.25f, -0.25f, +0.25f},
	{-0.25f, +0.25f, -0.25f},
	{+0.25f, +0.25f, -0.25f},
	{-0.25f, -0.25f, -0.25f},
	{+0.25f, -0.25f, -0.25f}
};
glm::vec3 CubeNormal[] = {
	{0.0f, 0.0f, 1.0f},
	{-1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, -1.0f, 0.0f}
};


// 0,1,2, 0,3,1, 0,4,3, 0,2,4, 2,1,3, 2,3,4
// 앞 왼 뒤 오 아래
glm::vec3 Pyramid[] = {
	{0.0f, 0.25f, 0.0f},
	{-0.25f, -0.25f, +0.25f},
	{+0.25f, -0.25f, +0.25f},
	{-0.25f, -0.25f, -0.25f},
	{+0.25f, -0.25f, -0.25f}
};
glm::vec3 PyramidNormal[] = {
	{0.0f, 0.0f, 1.0f},
	{-1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, -1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, -1.0f, 0.0f}
};


float xRotate = 30.0f, yRotate = 30.0f;
float xRotateDir = 0.0f;
bool isYRotate = false;
glm::vec2 movement = { 0.0f, 0.0f };

float front_open = 0.0f;
bool isOpen = false;

float up_swing = 0.0f;
bool isSwing = false;

float r_open = 0.0f, l_open = 0.0f;
bool isROpen = false, isLOpen = false;

float Pyramid_open = 0.0f;
bool isPyramidOpen = false;

bool isPerspective = true;

bool selModel = true;	// true == Cube / false == Tetra
bool isBackCull = false, isFill = true;

float radius = 1.0f, lightAngle = 0.0f, variable = 0.1f;
bool light = true, lightRotate = false;

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
		std::cout << "GLEW Initialized\n";
		std::cout << "\nC: 모델 바꾸기\n";
		std::cout << "X: X축 기준 회전\n";
		std::cout << "Y: Y축 기준 회전\n";
		std::cout << "H: 은면 제거\n";
		std::cout << "T: 육면체 윗면 회전 애니메이션\n";
		std::cout << "F: 육면체 앞면 열기/닫기\n";
		std::cout << "1/2: 육면체 옆면 열기/닫기\n";
		std::cout << "O: 사각뿔 열기 닫기\n";
		std::cout << "P: 직각/원근 투영 변환\n";
		std::cout << "M: 불 끄기\n";
		std::cout << "Q: 종료\n";
	}

	glEnable(GL_DEPTH_TEST);

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyUp);
	glutSpecialFunc(SpecialKey);
	glutTimerFunc(10, Animate, 0);

	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_vert();

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	//--- uniform 변수 값 받아오기
	unsigned int modelMat = glGetUniformLocation(s_program, "model");
	unsigned int viewMat = glGetUniformLocation(s_program, "view");
	unsigned int projMat = glGetUniformLocation(s_program, "projection");

	unsigned int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
	unsigned int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");

	//--- 카메라
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
	glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

	// 뷰잉 변환
	glm::mat4 m_view = glm::mat4(1.0f);
	m_view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_view));

	// 투영변환
	glm::mat4 m_proj = glm::mat4(1.0f);
	if (isPerspective)
		m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 50.0f);
	else
		m_proj = glm::ortho(-1.5f, +1.5f, -1.0f, +1.0f, 0.1f, 50.0f);
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_proj));


	// 빛
	glm::vec4 lightPos = { radius, 0.0f, 0.0f, 0.0f };

	glm::mat4 m_light(1.0f);
	m_light = glm::translate(m_light, glm::vec3(movement, 0.0f));
	m_light = glm::rotate(m_light, glm::radians(xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
	m_light = glm::rotate(m_light, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightPos = m_light * lightPos;

	glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);

	if (light)
		glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	else
		glUniform3f(lightColorLocation, 0.0f, 0.0f, 0.0f);



	glm::mat4 m_tmpMat = glm::mat4(1.0f);
	m_tmpMat = glm::translate(m_tmpMat, glm::vec3(movement, 0.0f));
	m_tmpMat = glm::translate(m_tmpMat, glm::vec3(lightPos.x, lightPos.y, lightPos.z));
	m_tmpMat = glm::rotate(m_tmpMat, glm::radians(xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
	m_tmpMat = glm::rotate(m_tmpMat, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	m_tmpMat = glm::scale(m_tmpMat, glm::vec3(0.2f, 0.2f, 0.2f));


	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
	glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	// 월드 변환
	glm::mat4 m_model = glm::mat4(1.0f);
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, glm::vec3(movement, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));


	glUniform3f(objColorLocation, 0.0, 1.0, 0.0);
	if (selModel)
	{

		// 앞
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, +0.25f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(front_open), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, -0.25f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 위
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, 0.0f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(up_swing), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, 0.0f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 24, 6);

		// 왼
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, l_open, 0.0f));
		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 6, 6);

		// 오
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, r_open, 0.0f));
		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 12, 6);

		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));
		// 뒤
		glDrawArrays(GL_TRIANGLES, 18, 6);
		// 아래
		glDrawArrays(GL_TRIANGLES, 30, 6);
	}

	else
	{
		//앞 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, 0.25f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(Pyramid_open), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, -0.25f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 36, 3);

		//왼 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(-0.25f, -0.25f, 0.0f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(Pyramid_open), glm::vec3(0.0f, 0.0f, 1.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.25f, 0.25f, 0.0f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 39, 3);

		//뒤 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, -0.25f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(-Pyramid_open), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, 0.25f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 42, 3);

		//오 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.25f, -0.25f, 0.0f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(-Pyramid_open), glm::vec3(0.0f, 0.0f, 1.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(-0.25f, 0.25f, 0.0f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 45, 3);


		//아래
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));
		glDrawArrays(GL_TRIANGLES, 48, 6);
	}



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
	GLchar* vertexSource = filetobuf("vertex_pr24.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr24.frag");

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
	static const int pyramidIndex[] = { 0,1,2, 0,3,1, 0,4,3, 0,2,4, 2,1,3, 2,3,4 };
	static const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	vPos.clear();
	vNormal.clear();

	int cnt = 0;
	int normalindex = 0;
	for (const int& i : cubeIndex)
	{
		if (cnt == 6)
			normalindex++;
		cnt %= 6;
		vPos.push_back(Cube[i]);
		vNormal.push_back(CubeNormal[normalindex]);
		cnt++;
	}
	for (const int& i : pyramidIndex)
	{
		vPos.push_back(Pyramid[i]);

	}
	int i;
	for (i = 0; i < 4; ++i)
		for (int j = 0; j < 3; ++j)
			vNormal.push_back(PyramidNormal[i]);

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

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);

	set_vert();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'c':
	case 'C':
		selModel = !selModel;
		break;

	case 'h':
	case 'H':
		isBackCull = !isBackCull;
		break;

	case 'x':
	case 'X':
		if (xRotateDir == 0.0f)
			xRotateDir = angleSpeed;
		else
			xRotateDir *= -1;
		break;

	case 'y':
	case 'Y':
		isYRotate = !isYRotate;
		break;

	case 'w':
	case 'W':
		isFill = !isFill;
		break;

	case 's':
	case 'S':
		xRotate = 30.0f;
		xRotateDir = 0.0f;
		yRotate = -30.0f;
		isYRotate = false;

		movement = glm::vec2(0.0f, 0.0f);
		break;

	case 'p':
	case 'P':
		isPerspective = !isPerspective;
		break;

	case 'r':
	case 'R':
		lightRotate = !lightRotate;
		break;

	case 'm':
	case 'M':
		light = !light;
		break;

	case 'Q':
	case 'q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;

	case 'z':
	case 'Z':
		if (radius > 0.4f)
			radius += variable;
		break;

	default:
		break;
	}

	if (selModel)
	{
		switch (key)
		{
		case 'f':
		case 'F':
			isOpen = !isOpen;
			break;

		case 't':
		case 'T':
			isSwing = !isSwing;
			break;

		case '1':
			isLOpen = !isLOpen;
			break;

		case '2':
			isROpen = !isROpen;
			break;
		default:
			break;
		}
	}
	else
	{
		if (key == 'o' || key == 'O')
			isPyramidOpen = !isPyramidOpen;
	}
	glutPostRedisplay();
}

GLvoid KeyUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'z':
	case 'Z':
		variable *= -1;
		break;
	}
}

GLvoid SpecialKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		movement.y += moveSpeed;
		break;

	case GLUT_KEY_DOWN:
		movement.y -= moveSpeed;
		break;

	case GLUT_KEY_LEFT:
		movement.x -= moveSpeed;
		break;

	case GLUT_KEY_RIGHT:
		movement.x += moveSpeed;
		break;

	default:
		break;
	}

	glutPostRedisplay();
}

GLvoid Animate(int val)
{

	xRotate += xRotateDir;
	if (isYRotate)
	{
		yRotate += angleSpeed;
	}

	if (lightRotate)
	{
		lightAngle += angleSpeed;
	}

	if (selModel)
	{
		if (isOpen && front_open < 90.0f)
			front_open += angleSpeed;
		else if (!isOpen && front_open > 0.0f)
			front_open -= angleSpeed;

		if (isSwing)
			up_swing += angleSpeed;

		if (isROpen && r_open < 0.5f)
			r_open += sideOpenSpeed;
		else if (!isROpen && r_open > 0.0f)
		{
			r_open -= sideOpenSpeed;
			if (r_open < 0.05f)
				r_open = 0.0f;
		}

		if (isLOpen && l_open < 0.5f)
			l_open += sideOpenSpeed;
		else if (!isLOpen && l_open > 0.0f)
		{
			l_open -= sideOpenSpeed;
			if (l_open < 0.05f)
				l_open = 0.0f;
		}
	}
	else
	{
		if (isPyramidOpen && Pyramid_open < 230.0f)
		{
			Pyramid_open += angleSpeed;
			if (Pyramid_open >= 230.0f)
			{
				Pyramid_open = 233.5f;
			}
		}
		else if (!isPyramidOpen && Pyramid_open > 0.0f)
		{
			if (Pyramid_open >= 230.0f)
				Pyramid_open = 230.0f;
			Pyramid_open -= angleSpeed;
		}
	}

	glutTimerFunc(100, Animate, 0);
	glutPostRedisplay();
}