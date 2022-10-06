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

void make_vertexShader();
void make_fragmentShader();
void set_vert();
void InitBuffer();
void InitShader();

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;
GLuint vao, vbo[2];

vector<glm::vec3> vPos;
vector<glm::vec3> vColor;

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

// 0,1,2, 0,3,1, 0,2,3, 1,3,2
// 앞 왼 오 아래
glm::vec3 Tetra[] = {
	{-0.25f, +0.25f, -0.25f},
	{-0.25f, -0.25f, +0.25f},
	{+0.25f, -0.25f, -0.25f},
	{-0.25f, -0.25f, -0.25f}
};
glm::vec3 TetraColor[] = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 1.0f, 1.0f}
};

int drawFace1 = -1, drawFace2 = -1;

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 700);
	glutCreateWindow("Practice13");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_vert();

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	glm::mat4 model = glm::mat4(1.0f);

	model = glm::rotate(model, (float)glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, (float)glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	unsigned int modelMat = glGetUniformLocation(s_program, "modelTransform");
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(model));

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0, vPos.size());

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

	//--- Shader Program 사용하기
	glUseProgram(s_program);
}
/**************************************************************************************/
/**************************************************************************************/

void setVector(int drawFace)
{
	const int cubeIndex[] = { 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7 };
	const int tetraIndex[] = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };

	if (drawFace == -1)
		return;

	if (1 <= drawFace && drawFace <= 6)
	{
		for (int i = (drawFace - 1) * 6; i < drawFace * 6; ++i)
		{
			vPos.push_back(Cube[cubeIndex[i]]);
			vColor.push_back(CubeColor[cubeIndex[i]]);
		}
	}
	else if (drawFace == 0)
	{
		for (int i = 9; i < 12; ++i)
		{
			vPos.push_back(Tetra[tetraIndex[i]]);
			vColor.push_back(TetraColor[tetraIndex[i]]);
		}
	}
	else
	{
		for (int i = (drawFace - 7) * 3; i < (drawFace - 6) * 3; ++i)
		{
			vPos.push_back(Tetra[tetraIndex[i]]);
			vColor.push_back(TetraColor[tetraIndex[i]]);
		}
	}
}

void set_vert()
{
	vPos.clear();
	vColor.clear();
	setVector(drawFace1);
	setVector(drawFace2);


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

// 1앞 2왼 3오 4뒤 5위 6아래
GLvoid Keyboard(unsigned char key, int x, int y)
{
	if ('0' <= key && key <= '9')
	{
		drawFace1 = key - '0';
		drawFace2 = -1;
	}
	else
	{
		switch (key)
		{
		case 'a':
		case 'A':
			drawFace1 = 1;
			drawFace2 = 4;
			break;

		case 'b':
		case 'B':
			drawFace1 = 2;
			drawFace2 = 3;
			break;

		case 'c':
		case 'C':
			drawFace1 = 5;
			drawFace2 = 6;
			break;

		case 'e':
		case 'E':
			drawFace1 = 0;
			drawFace2 = 7;
			break;

		case 'f':
		case 'F':
			drawFace1 = 0;
			drawFace2 = 8;
			break;

		case 'g':
		case 'G':
			drawFace1 = 0;
			drawFace2 = 9;
			break;
		}
	}
		

	glutPostRedisplay();
}