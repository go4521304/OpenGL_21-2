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

const float angleSpeed = 5.0f;
const float scaleValue = 0.1f;
const float movementVal = 0.05f;
const float pi = (float)acos(-1);


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
GLvoid SpecialKey(int key, int x, int y);
GLvoid Animate(int val);

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void set_vert_spin();
void InitBuffer();
void InitShader();

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;

GLuint vao, vbo[2];

vector<glm::vec3> vPos;
vector<glm::vec3> vColor;

// 인덱스 리스트로 변경해보기

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

float xRotate_cube = 0.0f, yRotate_cube = 0.0f;
float xRotateDir_cube = 0.0f, yRotateDir_cube = 0.0f;
float xRotate_pyramid = 0.0f, yRotate_pyramid = 180.0f;
float xRotateDir_pyramid = 0.0f, yRotateDir_pyramid = 0.0f;
float coordRotateX = 30.0f, coordRotateY = -30.0f, coordRotateDir = 0.0f;
bool changeShape = true;
float scaleFactor1 = 1.0f, scaleFactor2 = 1.0f, scaleFactorAll = 1.0f;
glm::vec3 movement1 = { -0.7f, 0.0f, 0.0f }, movement2 = { 0.7f, 0.0f, 0.0f }, movementAll = { 0.0f, 0.0f, 0.0f };

bool spining = false;
float spinAngle = 0.0f, spinR = 0.0f;
int numCnt = 0;

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice16");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKey);
	glutTimerFunc(100, Animate, 0);

	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	// 변환 행렬 생성
	unsigned int modelMat = glGetUniformLocation(s_program, "modelTransform");

	////// 좌표계 출력
	glm::mat4 coord = glm::mat4(1.0f);
	coord = glm::rotate(coord, (float)glm::radians(coordRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	coord = glm::rotate(coord, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(coord));
	glDrawArrays(GL_LINES, 0, 6);

	if (spining)
		glDrawArrays(GL_LINE_STRIP, 6, numCnt);

	// 육면체 출력
	glm::mat4 model1 = glm::mat4(1.0f);

	if (spining)
	{
		model1 = glm::rotate(model1, (float)glm::radians(coordRotateX + xRotate_cube), glm::vec3(1.0f, 0.0f, 0.0f));
		model1 = glm::rotate(model1, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
		model1 = glm::translate(model1, glm::vec3(spinR * cos(spinAngle), 0.0f, spinR * sin(spinAngle)));
		model1 = glm::rotate(model1, pi - spinAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		model1 = glm::scale(model1, glm::vec3(0.2f, 0.2f, 0.2f));
	}
	else
	{
		model1 = glm::scale(model1, glm::vec3(scaleFactorAll, scaleFactorAll, scaleFactorAll));
		model1 = glm::rotate(model1, (float)glm::radians(coordRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
		model1 = glm::rotate(model1, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
		model1 = glm::translate(model1, movement1 + movementAll);
		model1 = glm::rotate(model1, (float)glm::radians(xRotate_cube), glm::vec3(1.0f, 0.0f, 0.0f));
		model1 = glm::rotate(model1, (float)glm::radians(yRotate_cube), glm::vec3(0.0f, 1.0f, 0.0f));
		model1 = glm::scale(model1, glm::vec3(scaleFactor1, scaleFactor1, scaleFactor1));
	}

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(model1));
	if (changeShape)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		if (spining)
			glDrawArrays(GL_TRIANGLES, 6 + numCnt, 36);
		else
			glDrawArrays(GL_TRIANGLES, 6, 36);
	}
	else
	{
		GLUquadricObj* qobj1;
		qobj1 = gluNewQuadric();
		gluQuadricDrawStyle(qobj1, GLU_LINE);
		gluSphere(qobj1, 0.3, 20, 20);
	}


	// 피라미드 출력
	glm::mat4 model2 = glm::mat4(1.0f);
	model2 = glm::scale(model2, glm::vec3(scaleFactorAll, scaleFactorAll, scaleFactorAll));
	model2 = glm::rotate(model2, (float)glm::radians(coordRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
	model2 = glm::rotate(model2, (float)glm::radians(coordRotateY), glm::vec3(0.0f, 1.0f, 0.0f));
	model2 = glm::translate(model2, movement2 + movementAll);
	model2 = glm::rotate(model2, (float)glm::radians(xRotate_pyramid), glm::vec3(1.0f, 0.0f, 0.0f));
	model2 = glm::rotate(model2, (float)glm::radians(yRotate_pyramid), glm::vec3(0.0f, 1.0f, 0.0f));
	model2 = glm::scale(model2, glm::vec3(scaleFactor2, scaleFactor2, scaleFactor2));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(model2));

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_LINE);
	if (changeShape)
		gluCylinder(qobj, 0.3, 0.0, 1.0, 20, 8);
	else
		gluCylinder(qobj, 0.3, 0.3, 1.0, 20, 8);

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
	GLchar* vertexSource = filetobuf("vertex_pr13.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr13.frag");

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

void set_vert_spin()
{
	const glm::vec3 line[] = { {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f} };
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	float R = 0.0f;
	spinAngle = 0.0f;
	spinR = 0.0f;

	vPos.clear();
	vColor.clear();

	for (const glm::vec3& i : line)
	{
		vPos.push_back(i);
		vColor.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	numCnt = 0;
	for (float degree = 0; abs(degree) < (2 * pi * 5); degree += (pi / 18))
	{
		vPos.push_back(glm::vec3(R * cos(degree), 0.0f, R * sin(degree)));
		vColor.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
		R += 0.005f;
		numCnt++;
	}

	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(CubeColor[i]);
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vColor.size() * sizeof(glm::vec3), vColor.data(), GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}

void set_vert()
{
	const glm::vec3 line[] = { {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f} };
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };

	vPos.clear();
	vColor.clear();

	for (const glm::vec3& i : line)
	{
		vPos.push_back(i);
		vColor.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	}
	for (const int& i : cubeIndex)
	{
		vPos.push_back(Cube[i]);
		vColor.push_back(CubeColor[i]);
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "Pos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, vColor.size() * sizeof(glm::vec3), vColor.data(), GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
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
	case 'x':
	case 'X':
		if (xRotateDir_cube == 0.0f)
			xRotateDir_cube = angleSpeed;
		else
			xRotateDir_cube *= -1;
		break;

	case 'y':
	case 'Y':
		if (yRotateDir_cube == 0.0f)
			yRotateDir_cube = angleSpeed;
		else
			yRotateDir_cube *= -1;
		break;

	case 'a':
	case 'A':
		if (xRotateDir_pyramid == 0.0f)
			xRotateDir_pyramid = angleSpeed;
		else
			xRotateDir_pyramid *= -1;
		break;

	case 'b':
	case 'B':
		if (yRotateDir_pyramid == 0.0f)
			yRotateDir_pyramid = angleSpeed;
		else
			yRotateDir_pyramid *= -1;
		break;

	case 'r':
	case 'R':
		spining = !spining;
		if (spining)
			set_vert_spin();
		else
			set_vert();
		break;

	case 's':
	case 'S':
	case 'c':
	case 'C':
		xRotate_cube = 0.0f;
		yRotate_cube = 0.0f;
		xRotateDir_cube = 0.0f;
		yRotateDir_cube = 0.0f;

		xRotate_pyramid = 0.0f;
		yRotate_pyramid = 180.0f;
		xRotateDir_pyramid = 0.0f;
		yRotateDir_pyramid = 0.0f;

		coordRotateX = 30.0f;
		coordRotateY = -30.0f;
		coordRotateDir = 0.0f;

		scaleFactor1 = 1.0f;
		scaleFactor2 = 1.0f;
		scaleFactorAll = 1.0f;
		movement1 = { -0.7f, 0.0f, 0.0f };
		movement2 = { 0.7f, 0.0f, 0.0f };
		movementAll = { 0.0f, 0.0f, 0.0f };

		spining = false;
		spinAngle = 0.0f;
		spinR = 0.0f;

		set_vert();
		break;

	case '[':
		if (coordRotateDir == 0.0f)
			coordRotateDir = angleSpeed;
		else
			coordRotateDir *= -1;
		break;

	case ']':
		changeShape = !changeShape;
		break;

	case '1':
		if (scaleFactor1 > 0.1f)
			scaleFactor1 -= scaleValue;
		break;

	case '2':
		scaleFactor1 += scaleValue;
		break;

	case '3':
		if (scaleFactor2 > 0.1f)
			scaleFactor2 -= scaleValue;
		break;

	case '4':
		scaleFactor2 += scaleValue;
		break;

	case '5':
		if (scaleFactorAll > 0.1f)
			scaleFactorAll -= scaleValue;
		break;
	case '6':
		scaleFactorAll += scaleValue;
		break;


	case 't':
	case 'T':
		movement1.z += movementVal;
		break;

	case 'f':
	case 'F':
		movement1.x -= movementVal;
		break;

	case 'g':
	case 'G':
		movement1.z -= movementVal;
		break;

	case 'h':
	case 'H':
		movement1.x += movementVal;
		break;


	case 'i':
	case 'I':
		movement2.z += movementVal;
		break;

	case 'j':
	case 'J':
		movement2.x -= movementVal;
		break;

	case 'k':
	case 'K':
		movement2.z -= movementVal;
		break;

	case'l':
	case'L':
		movement2.x += movementVal;
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_HOME:
		movement1.y += movementVal;
		break;

	case GLUT_KEY_END:
		movement1.y -= movementVal;
		break;

	case GLUT_KEY_PAGE_UP:
		movement2.y += movementVal;
		break;

	case GLUT_KEY_PAGE_DOWN:
		movement2.y -= movementVal;
		break;

	
	case GLUT_KEY_UP:
		movementAll.z -= movementVal;
		break;

	case GLUT_KEY_DOWN:
		movementAll.z += movementVal;
		break;

	case GLUT_KEY_LEFT:
		movementAll.x -= movementVal;
		break;

	case GLUT_KEY_RIGHT:
		movementAll.x += movementVal;
		break;
	}
}

GLvoid Animate(int val)
{
	xRotate_cube += xRotateDir_cube;
	yRotate_cube += yRotateDir_cube;

	xRotate_pyramid += xRotateDir_pyramid;
	yRotate_pyramid += yRotateDir_pyramid;

	coordRotateY += coordRotateDir;

	if (spining)
	{
		spinR += 0.005f; 
		spinAngle += (pi / 18);

		if (spinAngle >= (2 * pi * 5))
		{
			spinR = 0.0f;
			spinAngle = 0.0f;
		}
	}

	glutTimerFunc(100, Animate, 0);
	glutPostRedisplay();
}