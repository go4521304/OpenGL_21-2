#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
GLvoid Animation(int val);
void make_vertexShader();
void make_fragmentShader();
void InitBuffer();
void InitShader();


void Init_Shape_Set();
void set_vert();

GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint s_program;
GLuint vao, vbo[2], EBO;

struct Vertex {
	float pos_x, pos_y;
	float dir_x, dir_y;
};

const float color[][3] = {
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},

	{1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},
	{1.0f, 0.0f, 0.0f},

	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},

	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},

};
const unsigned int Index[] = {
	0, 1, 2,

	3, 4, 5,
	3, 5, 6,

	7, 8, 9,
	7, 9, 10, 
	7, 10, 11, 

	12, 13, 14,
	12, 14, 15,
	12, 15, 16
};

vector<Vertex> vShape;
int Count = 0;
const int Time = 100;

// 이거 수정하심 속도 변경하실수 있슴다
const int Play = 10;

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice10");

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

	Init_Shape_Set();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glutTimerFunc(Time, Animation, 0);
	glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	set_vert();

	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(s_program);

	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	//--- 삼각형 그리기
	if (Count == 0)
		glDrawArrays(GL_LINES, 0, 2);
	glDrawElements(GL_TRIANGLES, 27, GL_UNSIGNED_INT, 0);

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
	GLchar* vertexSource = filetobuf("vertex_pr5.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr5.frag");

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
	vector<float> vert;

	for (auto& i : vShape)
	{
		vert.push_back(i.pos_x);
		vert.push_back(i.pos_y);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vert.size(), vert.data(), GL_STATIC_DRAW);

	// 인덱스 리스트 생성
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index), Index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기

	glBindVertexArray(vao); //--- VAO를 바인드하기

	// 색 VBO 생성
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(s_program, "Color");
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);


	set_vert();
}

void Init_Shape_Set()
{
	// 점
	const float Point[5][2] =
	{
		0.000f, 0.003f,
		0.0028f, 0.0005f,
		0.0018f, -0.003f,
		-0.0018f, -0.003f,
		-0.0028f, 0.0005f
	};

	// 선
	const float Line[3][2] = {
		-0.3f, -0.3f,
		0.0f, +0.3f,
		0.0f, +0.3f,
	};
	
	// 삼각형
	const float Triangle[4][2] = {
		-0.3f, -0.3f,
		0.0f, +0.3f,
		0.3f, -0.3f,
		0.3f, -0.3f,
	};

	// 사각형
	const float Rectangle[5][2] = {
		-0.3f, +0.3f,
		+0.3f, +0.3f, 
		+0.3f, -0.3f, 
		-0.3f, -0.3f,
		-0.3f, -0.3f
	};

	// 오각형
	const float Pentagon[5][2] = {
		0.0f, 0.3f,
		0.28f, 0.05f,
		0.18f, -0.3f,
		-0.18f, -0.3f,
		-0.28f, 0.05f
	};

	Vertex tmp;

	// 선->삼각형 정점
	for (int i = 0; i < 3; ++i)
	{
		tmp.pos_x = -0.5f + Line[i][0];
		tmp.pos_y = +0.5f + Line[i][1];
		tmp.dir_x = ((-0.5f + Triangle[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((+0.5f + Triangle[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}

	// 삼각형->사각형 정점
	for (int i = 0; i < 4; ++i)
	{
		tmp.pos_x = +0.5f + Triangle[i][0];
		tmp.pos_y = +0.5f + Triangle[i][1];
		tmp.dir_x = ((+0.5f + Rectangle[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((+0.5f + Rectangle[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}

	// 사각형->오각형 정점
	for (int i = 0; i < 5; ++i)
	{
		tmp.pos_x = -0.5f + Rectangle[i][0];
		tmp.pos_y = -0.5f + Rectangle[i][1];
		tmp.dir_x = ((-0.5f + Pentagon[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((-0.5f + Pentagon[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}

	// 오각형->점 정점
	for (int i = 0; i < 5; ++i)
	{
		tmp.pos_x = +0.5f + Pentagon[i][0];
		tmp.pos_y = -0.5f + Pentagon[i][1];
		tmp.dir_x = ((+0.5f + Point[i][0]) - tmp.pos_x) / Play;
		tmp.dir_y = ((-0.5f + Point[i][1]) - tmp.pos_y) / Play;
		vShape.push_back(tmp);
	}
}

GLvoid Animation(int val)
{
	for (auto& i : vShape)
	{
		i.pos_x += i.dir_x;
		i.pos_y += i.dir_y;
	}

	Count++;

	if (Count > Play)
	{
		Count = 0;
		vShape.clear();
		Init_Shape_Set();
	}

	glutTimerFunc(Time, Animation, 0);
	glutPostRedisplay();
}