#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

#include <gl/glew.h> // �ʿ��� ������� include
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

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;

GLuint vao, vbo[2];

vector<glm::vec3> vPos;
vector<glm::vec3> vNormal;


// 1,0,2, 1,2,3, 0,4,6, 0,6,2, 5,1,3, 5,3,7, 4,5,7, 4,7,6, 5,4,0, 5,0,1, 3,2,6, 3,6,7
// �� �� �� �� �� �Ʒ�
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
// �� �� �� �� �Ʒ�
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

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutCreateWindow("Practice17");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n";
		std::cout << "\nC: �� �ٲٱ�\n";
		std::cout << "X: X�� ���� ȸ��\n";
		std::cout << "Y: Y�� ���� ȸ��\n";
		std::cout << "H: ���� ����\n";
		std::cout << "T: ����ü ���� ȸ�� �ִϸ��̼�\n";
		std::cout << "F: ����ü �ո� ����/�ݱ�\n";
		std::cout << "1/2: ����ü ���� ����/�ݱ�\n";
		std::cout << "O: �簢�� ���� �ݱ�\n";
		std::cout << "P: ����/���� ���� ��ȯ\n";
		std::cout << "M: �� ����\n";
		std::cout << "Q: ����\n";
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

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_vert();

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- uniform ���� �� �޾ƿ���
	unsigned int modelMat = glGetUniformLocation(s_program, "model");
	unsigned int viewMat = glGetUniformLocation(s_program, "view");
	unsigned int projMat = glGetUniformLocation(s_program, "projection");

	unsigned int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
	unsigned int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");

	//--- ī�޶�
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
	glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));

	// ���� ��ȯ
	glm::mat4 m_view = glm::mat4(1.0f);
	m_view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewMat, 1, GL_FALSE, glm::value_ptr(m_view));

	// ������ȯ
	glm::mat4 m_proj = glm::mat4(1.0f);
	if (isPerspective)
		m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 50.0f);
	else
		m_proj = glm::ortho(-1.5f, +1.5f, -1.0f, +1.0f, 0.1f, 50.0f);
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_proj));


	// ��
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


	// ���� ��ȯ
	glm::mat4 m_model = glm::mat4(1.0f);
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, glm::vec3(movement, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));


	glUniform3f(objColorLocation, 0.0, 1.0, 0.0);
	if (selModel)
	{

		// ��
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, +0.25f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(front_open), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, -0.25f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// ��
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, 0.0f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(up_swing), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, 0.0f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 24, 6);

		// ��
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, l_open, 0.0f));
		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 6, 6);

		// ��
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, r_open, 0.0f));
		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 12, 6);

		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));
		// ��
		glDrawArrays(GL_TRIANGLES, 18, 6);
		// �Ʒ�
		glDrawArrays(GL_TRIANGLES, 30, 6);
	}

	else
	{
		//�� 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, 0.25f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(Pyramid_open), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, -0.25f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 36, 3);

		//�� 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(-0.25f, -0.25f, 0.0f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(Pyramid_open), glm::vec3(0.0f, 0.0f, 1.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.25f, 0.25f, 0.0f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 39, 3);

		//�� 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, -0.25f, -0.25f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(-Pyramid_open), glm::vec3(1.0f, 0.0f, 0.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.0f, +0.25f, 0.25f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 42, 3);

		//�� 
		m_tmpMat = glm::mat4(1.0f);
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(0.25f, -0.25f, 0.0f));
		m_tmpMat = glm::rotate(m_tmpMat, glm::radians(-Pyramid_open), glm::vec3(0.0f, 0.0f, 1.0f));
		m_tmpMat = glm::translate(m_tmpMat, glm::vec3(-0.25f, 0.25f, 0.0f));

		m_tmpMat = m_model * m_tmpMat;
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_tmpMat));
		glDrawArrays(GL_TRIANGLES, 45, 3);


		//�Ʒ�
		glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));
		glDrawArrays(GL_TRIANGLES, 48, 6);
	}



	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)		//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}


/******************************���̴� ���α׷� �����*************************************/
/**************************************************************************************/
void make_vertexShader()
{
	GLchar* vertexSource = filetobuf("vertex_pr24.vert");

	//--- ���ؽ� ���̴� ��ü �����
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);

	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexShader);

	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	GLchar* fragmentSource = filetobuf("fragment_pr24.frag");

	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);

	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentShader);

	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- ���ؽ� ���̴� �����
	make_fragmentShader(); //--- �����׸�Ʈ ���̴� �����

	//-- shader Program
	s_program = glCreateProgram();

	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);

	//--- ���̴� �����ϱ�
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