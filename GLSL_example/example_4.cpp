#include <iostream>
#include <fstream>
#include <string>
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

void InitBuffer();
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid DrawScene();
GLvoid Reshape(int w, int h);

GLint width, height;
GLuint shaderID;

// VBO를 한개 설정하고 세이더에서 직접 인덱스를 지정해서 사용

const float vertexPosition[] =
{
	0.5, 1.0, 0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0
};

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	width = 500;
	height = 500;

	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);	// 윈도우의 위치 지정
	glutInitWindowSize(width, height);	// 윈도우의 크기 지정
	glutCreateWindow("Example4");	// 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew 초기화
	{
		cerr << "Unable to initialize GLEW" << endl;
		exit(EXIT_FAILURE);
	}
	else
		cout << "GLEW Initialized\n";


	make_vertexShaders();
	make_fragmentShaders();
	shaderID = make_shaderProgram();

	InitBuffer();

	glutDisplayFunc(DrawScene);	// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);	// 다시 그리기 콜백함수 지정
	glutMainLoop();				// 이벤트 처리 시작
}

GLvoid DrawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);	// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);			// 설정된 색으로 전체를 칠하기

	glUseProgram(shaderID);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glutSwapBuffers();						// 화면에 출력하기
}

GLvoid Reshape(int w, int h)		//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLuint vertexShader;
void make_vertexShaders()
{
	GLchar* vertexsource;
	vertexsource = filetobuf("vertex_4.glsl"); // 버텍스세이더 읽어오기
	//--- filetobuf: 사용자 정의함수로 텍스트를 읽어서 문자열에 저장하는 함수
	// 
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexsource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader error\n" << errorLog << endl;
		return;
	}
}

GLuint fragmentShader;
void make_fragmentShaders()
{
	GLchar* fragmentsource;
	fragmentsource = filetobuf("fragment_4.glsl"); // 프래그세이더 읽어오기
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader error\n" << errorLog << endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLuint ShaderProgramID = glCreateProgram(); //--- 세이더 프로그램 만들기

	glAttachShader(ShaderProgramID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(ShaderProgramID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기

	glLinkProgram(ShaderProgramID); //--- 세이더 프로그램 링크하기

	glDeleteShader(vertexShader); //--- 세이더 프로그램에 링크하여 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
		cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
		return false;
	}
	return ShaderProgramID;
}

//--- 변수 선언
GLuint VAO, VBO_position;
GLvoid InitBuffer()
{
	//--- VAO와 VBO 객체 생성
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO_position);

	//--- 사용할 VAO 바인딩
	glBindVertexArray(VAO);

	//--- vertex positions 저장을 위한 VBO 바인딩.
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	//--- vertex positions 데이터 입력.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition), vertexPosition, GL_STATIC_DRAW);

	//--- 현재 바인딩되어있는 VBO를 0번째 attribute에 가져오도록 지정하고 그 인덱스의 attribute를 활성화
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}