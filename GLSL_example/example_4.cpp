#include <iostream>
#include <fstream>
#include <string>
#include <gl/glew.h> // �ʿ��� ������� include
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

// VBO�� �Ѱ� �����ϰ� ���̴����� ���� �ε����� �����ؼ� ���

const float vertexPosition[] =
{
	0.5, 1.0, 0.0,
	0.0, 0.0, 0.0,
	1.0, 0.0, 0.0
};

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	width = 500;
	height = 500;

	//--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0);	// �������� ��ġ ����
	glutInitWindowSize(width, height);	// �������� ũ�� ����
	glutCreateWindow("Example4");	// ������ ����(������ �̸�)

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
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

	glutDisplayFunc(DrawScene);	// ��� �ݹ��Լ��� ����
	glutReshapeFunc(Reshape);	// �ٽ� �׸��� �ݹ��Լ� ����
	glutMainLoop();				// �̺�Ʈ ó�� ����
}

GLvoid DrawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	//--- ����� ���� ����
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);	// �������� ����
	glClear(GL_COLOR_BUFFER_BIT);			// ������ ������ ��ü�� ĥ�ϱ�

	glUseProgram(shaderID);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glutSwapBuffers();						// ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLuint vertexShader;
void make_vertexShaders()
{
	GLchar* vertexsource;
	vertexsource = filetobuf("vertex_4.glsl"); // ���ؽ����̴� �о����
	//--- filetobuf: ����� �����Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
	// 
	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
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
	fragmentsource = filetobuf("fragment_4.glsl"); // �����׼��̴� �о����
	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
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
	GLuint ShaderProgramID = glCreateProgram(); //--- ���̴� ���α׷� �����

	glAttachShader(ShaderProgramID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
	glAttachShader(ShaderProgramID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�

	glLinkProgram(ShaderProgramID); //--- ���̴� ���α׷� ��ũ�ϱ�

	glDeleteShader(vertexShader); //--- ���̴� ���α׷��� ��ũ�Ͽ� ���̴� ��ü ��ü�� ���� ����
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
		cerr << "ERROR: shader program ���� ����\n" << errorLog << endl;
		return false;
	}
	return ShaderProgramID;
}

//--- ���� ����
GLuint VAO, VBO_position;
GLvoid InitBuffer()
{
	//--- VAO�� VBO ��ü ����
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO_position);

	//--- ����� VAO ���ε�
	glBindVertexArray(VAO);

	//--- vertex positions ������ ���� VBO ���ε�.
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	//--- vertex positions ������ �Է�.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition), vertexPosition, GL_STATIC_DRAW);

	//--- ���� ���ε��Ǿ��ִ� VBO�� 0��° attribute�� ���������� �����ϰ� �� �ε����� attribute�� Ȱ��ȭ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}