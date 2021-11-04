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
	{-0.25f, 0.25f, +0.25f},
	{+0.25f, 0.25f, +0.25f},
	{-0.25f, 0.00f, +0.25f},
	{+0.25f, 0.00f, +0.25f},
	{-0.25f, 0.25f, -0.25f},
	{+0.25f, 0.25f, -0.25f},
	{-0.25f, 0.00f, -0.25f},
	{+0.25f, 0.00f, -0.25f}
};
glm::vec3 CubeColor[] = {
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
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKey(int key, int x, int y);
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo[4];
GLint pAttribute, cAttribute;

vector<glm::vec3> vPos;
vector<glm::vec3> vColor;


glm::vec3 cranePos = { 0.0f, 0.0f, 0.0f };
float craneDir = 0.1f, midRotateDir = 1.0f, midAngle = 0.0f, topRotateDir = 1.0f, topAngle = 0.0f;
bool isBottonZmove = false, isMidYRotate = false, isArmRotate = false;
glm::vec3 cameraPos = { 0.0f, 3.0f, 5.0f };
bool isCamXmove = false, isCamYmove = false, isCamZmove = false, isCamRotate = false, isCamCenterRotate = false;
float camXdir = 0.05f, camYdir = -0.05f, camZdir = 0.05f, camAngle = 0.0f, camCenterAngle = 0.0f;
bool isView1 = false, isView2 = false;
float view1Angle = 0.0f, view2Angle = 0.0f;


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
		cout << "b: 크레인 아래 몸체 z축방향 이동" << endl;
		cout << "m: 크레인 중앙 몸체 y축방향 회전" << endl;
		cout << "t: 크레인 맨 위 팔 x축방향 회전" << endl << endl;
		cout << "x: 카메라 x축 방향 이동" << endl;
		cout << "y: 카메라 y축 방향 이동" << endl;
		cout << "z: 카메라 z축 방향 이동" << endl;
		cout << "r: y축 방향 카메라 회전" << endl;
		cout << "a: 화면 중심의 y축 방향 카메라 회전" << endl;
		cout << "s: 모든 움직임 멈추기" << endl;
		cout << "c: 모든 움직임 초기화" << endl;
		cout << "q: 종료" << endl;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	InitShader();
	InitBuffer();

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

	glm::vec3 cameraDirection, cameraRight, cameraUp;
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_proj = glm::mat4(1.0f);
	glm::mat4 m_world = glm::mat4(1.0f);

	glm::mat4 mat_pos = glm::mat4(1.0f);
	glm::mat4 mat_midPos = glm::mat4(1.0f);
	glm::mat4 mat_topPos_1 = glm::mat4(1.0f);
	glm::mat4 mat_topPos_2 = glm::mat4(1.0f);
	glm::mat4 mat_midScale = glm::mat4(1.0f);
	glm::mat4 mat_topScale = glm::mat4(1.0f);
	glm::mat4 mat_midRotate = glm::mat4(1.0f);
	glm::mat4 mat_topRotate_1 = glm::mat4(1.0f);
	glm::mat4 mat_topRotate_2 = glm::mat4(1.0f);

	glm::mat4 m_modelBot = glm::mat4(1.0f);
	glm::mat4 m_modelMid = glm::mat4(1.0f);
	glm::mat4 m_modelTop_1 = glm::mat4(1.0f);
	glm::mat4 m_modelTop_2 = glm::mat4(1.0f);
	glm::mat4 m_border = glm::mat4(1.0f);


	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	//--- uniform 변수 값 받아오기
	unsigned int modelMat = glGetUniformLocation(s_program, "model");
	unsigned int viewMat = glGetUniformLocation(s_program, "view");
	unsigned int projMat = glGetUniformLocation(s_program, "proj");
	unsigned int colorLocation = glGetUniformLocation(s_program, "vColor");


	//==========================================================================================================================//

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	glViewport((GLsizei)(SCR_WIDTH * 0.01), (GLsizei)(SCR_HEIGHT * 0.15), (GLsizei)(SCR_WIDTH * 0.7), (GLsizei)(SCR_HEIGHT * 0.7));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	//--- 카메라
	cameraDirection = glm::vec3(-5.0f * glm::sin(glm::radians(camAngle)), -3.0f, -5.0f * glm::cos(glm::radians(camAngle))) + cameraPos;
	cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

	// 뷰잉 변환
	m_view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_view));

	// 투영변환
	m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_proj));

	// 월드 변환
	m_world = glm::rotate(m_world, glm::radians(camCenterAngle), { 0.0f, 1.0f, 0.0f });
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_world));

	// -바닥면 출력
	glDrawArrays(GL_TRIANGLES, 0, 6);

	
	// ================================ 크레인 ================================
	mat_pos = glm::translate(mat_pos, cranePos);
	mat_midPos = glm::translate(mat_midPos, { 0.0f, 0.25f, 0.0f });
	mat_topPos_1 = glm::translate(mat_topPos_1, { -0.1f, 0.125f, 0.0f });
	mat_topPos_2 = glm::translate(mat_topPos_2, { 0.1f, 0.125f, 0.0f });
	mat_midScale = glm::scale(mat_midScale, { 0.8f, 0.5f, 0.8f });
	mat_topScale = glm::scale(mat_topScale, { 0.2f, 1.0f, 0.2f });
	mat_midRotate = glm::rotate(mat_midRotate, glm::radians(midAngle), { 0.0f, 1.0f, 0.0f });
	mat_topRotate_1 = glm::rotate(mat_topRotate_1, glm::radians(topAngle), { 1.0f, 0.0f, 0.0f });
	mat_topRotate_2 = glm::rotate(mat_topRotate_2, glm::radians(-topAngle), { 1.0f, 0.0f, 0.0f });


	// = 1단
	m_modelBot = m_world * mat_pos;
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelBot));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 2단
	m_modelMid = m_world * mat_pos * mat_midPos * mat_midRotate * mat_midScale;
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelMid));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 3단 - 1
	m_modelTop_1 = m_world * mat_pos * mat_midRotate * mat_topPos_1 * mat_midPos * mat_topRotate_1 * mat_topScale;
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelTop_1));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 3단 - 2
	m_modelTop_2 = m_world * mat_pos * mat_midRotate * mat_topPos_2 * mat_midPos * mat_topRotate_2 * mat_topScale;
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelTop_2));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	
	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glDrawArrays(GL_LINE_LOOP, 0, 4);


	//==========================================================================================================================//
	// 정면도

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	glViewport((GLsizei)(SCR_WIDTH * 0.73), (GLsizei)(SCR_HEIGHT * 0.55), (GLsizei)(SCR_WIDTH * 0.25), (GLsizei)(SCR_HEIGHT * 0.25));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	//--- 카메라
	cameraDirection = glm::vec3(-5.0f * glm::sin(glm::radians(view1Angle)), -3.0f, -5.0f * glm::cos(glm::radians(view1Angle))) + cameraPos;
	cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

	// 뷰잉 변환
	m_view = glm::lookAt(glm::vec3(cameraPos.x, cameraPos.y - 3.0f, cameraPos.z), cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_view));

	// 투영변환
	m_proj = glm::mat4(1.0f);
	m_proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_proj));

	// 월드 변환
	m_world = glm::mat4(1.0f);
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_world));

	// -바닥면 출력
	glDrawArrays(GL_TRIANGLES, 0, 6);


	// ================================ 크레인 ================================

	// = 1단	
	m_modelBot = m_world * mat_pos;
	m_modelMid = m_world * mat_pos * mat_midPos * mat_midRotate * mat_midScale;
	m_modelTop_1 = m_world * mat_pos * mat_midRotate * mat_topPos_1 * mat_midPos * mat_topRotate_1 * mat_topScale;
	m_modelTop_2 = m_world * mat_pos * mat_midRotate * mat_topPos_2 * mat_midPos * mat_topRotate_2 * mat_topScale;

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelBot));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 2단
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelMid));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 3단 - 1
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelTop_1));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 3단 - 2
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelTop_2));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);


	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glDrawArrays(GL_LINE_LOOP, 0, 4);

	//==========================================================================================================================//
	// 평면도
	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	glViewport((GLsizei)(SCR_WIDTH * 0.73), (GLsizei)(SCR_HEIGHT * 0.2), (GLsizei)(SCR_WIDTH * 0.25), (GLsizei)(SCR_HEIGHT * 0.25));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	//--- 카메라
	cameraDirection = glm::vec3(cameraPos.x, 0.0f, cameraPos.z - 5.0f);

	// 뷰잉 
	m_view = glm::mat4(1.0f);
	m_view = glm::lookAt(glm::vec3(cameraPos.x, cameraPos.y, cameraPos.z - 5.0f), cameraDirection, glm::vec3(0.0f, 0.0f, -1.0f));
	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_view));

	// 투영변환
	m_proj = glm::mat4(1.0f);
	m_proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_proj));

	// 월드 변환
	m_world = glm::mat4(1.0f);
	m_world = glm::rotate(m_world, glm::radians(view2Angle), { 0.0f, 1.0f, 0.0f });
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_world));

	// -바닥면 출력
	glDrawArrays(GL_TRIANGLES, 0, 6);


	// ================================ 크레인 ================================
	m_modelBot = m_world * mat_pos;
	m_modelMid = m_world * mat_pos * mat_midPos * mat_midRotate * mat_midScale;
	m_modelTop_1 = m_world * mat_pos * mat_midRotate * mat_topPos_1 * mat_midPos * mat_topRotate_1 * mat_topScale;
	m_modelTop_2 = m_world * mat_pos * mat_midRotate * mat_topPos_2 * mat_midPos * mat_topRotate_2 * mat_topScale;

	// = 1단
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelBot));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 2단
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelMid));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 3단 - 1
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelTop_1));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	// = 3단 - 2
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_modelTop_2));
	glDrawArrays(GL_TRIANGLES, 6, vPos.size() - 6);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);


	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_border));
	m_border = glm::translate(m_border, glm::vec3(0.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_border));
	glDrawArrays(GL_LINE_LOOP, 0, 4);

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
	const glm::vec3 floor[] = { {-5.0f, 0.0f, -5.0f}, {-5.0f, 0.0f, 5.0f}, {5.0f, 0.0f, -5.0f}, {5.0f, 0.0f, -5.0f}, {-5.0f, 0.0f, 5.0f}, {5.0f, 0.0f, 5.0f} };

	for (const auto& i : floor)
	{
		vPos.push_back(i);
		vColor.push_back(glm::vec3(0.5f, 0.2f, 0.0f));
	}

	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(CubeColor[i]);
	}

	const glm::vec3 screenLine[] = { glm::vec3(-0.999f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) , glm::vec3(1.0f, -0.999f, 0.0f) , glm::vec3(-0.999f, -0.999f, 0.0f) };
	const glm::vec3 linColor[] = { glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f) , glm::vec3(1.0f, 1.0f, 1.0f) , glm::vec3(1.0f, 1.0f, 1.0f) };


	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vColor.size() * sizeof(glm::vec3), vColor.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), screenLine, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), linColor, GL_STATIC_DRAW);

	pAttribute = glGetAttribLocation(s_program, "vPos");

	cAttribute = glGetAttribLocation(s_program, "vColor");
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(4, vbo);

	set_vert();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'b':
	case 'B':
		isBottonZmove = !isBottonZmove;
		if (isBottonZmove)
			craneDir *= -1.0f;
		break;

	case 'm':
	case 'M':
		isMidYRotate = !isMidYRotate;
		if (isMidYRotate)
			midRotateDir *= -1.0f;
		break;

	case 't':
	case 'T':
		isArmRotate = !isArmRotate;
		break;

	case 'x':
	case 'X':
		isCamXmove = !isCamXmove;
		if (isCamXmove)
			camXdir *= -1.0f;
		break;

	case 'y':
	case 'Y':
		isCamYmove = !isCamYmove;
		if (isCamYmove)
			camYdir *= -1.0f;
		break;

	case 'z':
	case 'Z':
		isCamZmove = !isCamZmove;
		if (isCamZmove)
			camZdir *= -1.0f;
		break;

	case 'r':
	case 'R':
		isCamRotate = !isCamRotate;
		break;

	case 'a':
	case 'A':
		isCamCenterRotate = !isCamCenterRotate;
		break;

	case 'c':
	case 'C':
		cranePos = { 0.0f, 0.0f, 0.0f };
		craneDir = 0.1f;
		midRotateDir = 1.0f;
		midAngle = 0.0f;
		topRotateDir = 1.0f;
		topAngle = 0.0f;
		cameraPos = { 0.0f, 3.0f, 5.0f };
		camXdir = 0.05f;
		camYdir = -0.05f;
		camZdir = 0.05f;
		camAngle = 0.0f;
		camCenterAngle = 0.0f;
		view1Angle = 0.0f;
		view2Angle = 0.0f;

	case 's':
	case 'S':
		isBottonZmove = false;
		isMidYRotate = false;
		isArmRotate = false;
		isCamXmove = false;
		isCamYmove = false;
		isCamZmove = false;
		isCamRotate = false;
		isCamCenterRotate = false;
		isView1 = false;
		isView2 = false;
		break;

	case 'Q':
	case 'q':
		std::cout << "Bye Bye~!" << std::endl;
		glutLeaveMainLoop();
		exit(0);
		break;

	case '1':
		isView1 = !isView1;
		break;

	case '2':
		isView2 = !isView2;
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid Animate(int val)
{
	if (isBottonZmove)
		cranePos.z += craneDir;

	if (isMidYRotate)
		midAngle += midRotateDir;

	if (isArmRotate)
	{
		topAngle += topRotateDir;
		if (topAngle > 90.0f || topAngle < -90.0f)
			topRotateDir *= -1;
	}

	if (isCamXmove)
	{
		cameraPos.x += camXdir;
	}

	if (isCamYmove)
	{
		cameraPos.y += camYdir;
	}

	if (isCamZmove)
	{
		cameraPos.z += camZdir;
	}

	if (isCamCenterRotate)
	{
		camCenterAngle -= 1.0f;
	}

	if (isCamRotate)
	{
		camAngle -= 1.0f;
	}

	if (isView1)
	{
		view1Angle -= 1.0f;
	}

	if (isView2)
	{
		view2Angle -= 1.0f;
	}
	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}