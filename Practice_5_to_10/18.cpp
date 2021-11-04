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

static const int SCR_WIDTH = 1200, SCR_HEIGHT = 800;

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
void InitBuffer();
void InitShader();

GLuint vertexShader, fragmentShader; //--- ���̴� ��ü
GLuint s_program;

GLuint vao, vbo;

vector<glm::vec3> vPos;

bool isPerspective = true;
glm::vec3 movement = { 0.0f, 0.0f, 0.0f };

float yRotate = 0.0f, yDir = 0.0f;
static const float pi = (float)acos(-1);

bool isFill = false;
float angle1 = 0.0f, angle1_m = 90.0f, angle2 = 60.0f, angle2_m = 90.0f, angle3 = 90.0f, angle3_m = 0.0f;

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
	glutCreateWindow("Practice18");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		// glew �ʱ�ȭ
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
	glutTimerFunc(10, Animate, 0);

	glutMainLoop();
}

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);

	//--- uniform ���� �� �޾ƿ���
	unsigned int modelMat = glGetUniformLocation(s_program, "model");
	unsigned int viewMat = glGetUniformLocation(s_program, "view");
	unsigned int projMat = glGetUniformLocation(s_program, "proj");
	unsigned int colorLocation = glGetUniformLocation(s_program, "vColor");


	//--- ī�޶�
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
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
		m_proj = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 8.0f);
	else
		m_proj = glm::ortho(-1.5f, +1.5f, -1.0f, +1.0f, 0.1f, 8.0f);
	glUniformMatrix4fv(projMat, 1, GL_FALSE, glm::value_ptr(m_proj));

	// ���� ��ȯ
	glm::mat4 m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, movement);
	m_model = glm::rotate(m_model, (float)glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	GLUquadricObj* planet = gluNewQuadric();

	if (isFill)
	{
		gluQuadricDrawStyle(planet, GLU_FILL);
	}
	else
	{
		gluQuadricDrawStyle(planet, GLU_LINE);
	}


	// �߾�
	glm::vec3 color = { 0.9f, 0.3f, 0.1f};
	glUniform3f(colorLocation, color.r, color.g, color.b);
	gluSphere(planet, 0.7, 20, 20);

	color = { 1.0f, 1.0f, 1.0f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	glDrawArrays(GL_LINE_LOOP, 0, 360);

	// 1�� ����
	m_model = glm::translate(m_model, glm::vec3(glm::vec3(2.0f * cos(glm::radians(angle1)), 0.0f, 2.0f * sin(glm::radians(angle1)))));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	// -�༺
	color = { 0.5f, 0.5f, 0.5f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	gluSphere(planet, 0.1, 20, 10);

	// -����
	color = { 1.0f, 1.0f, 1.0f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	glDrawArrays(GL_LINE_LOOP, 360, 360);

	m_model = glm::translate(m_model, glm::vec3(glm::vec3(0.5f * cos(glm::radians(angle1_m)), 0.0f, 0.5f * sin(glm::radians(angle1_m)))));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	color = { 0.0f, 0.3f, 0.4f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	gluSphere(planet, 0.05, 10, 10);


	// 2�� ����
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, movement);
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	color = { 1.0f, 1.0f, 1.0f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	glDrawArrays(GL_LINE_LOOP, 0, 360);

	// -�༺
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, movement);
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_model = glm::translate(m_model, glm::vec3(glm::vec3(2.0f * cos(glm::radians(angle2)), 0.0f, 2.0f * sin(glm::radians(angle2)))));
	m_model = glm::rotate(m_model, (float)glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	color = { 0.3f, 0.7f, 0.9f };
	glUniform3f(colorLocation, color.r, color.g, color.b);

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	gluSphere(planet, 0.2, 20, 10);

	// -����
	color = { 1.0f, 1.0f, 1.0f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	glDrawArrays(GL_LINE_LOOP, 360, 360);

	m_model = glm::translate(m_model, glm::vec3(glm::vec3(0.5f * cos(glm::radians(angle2_m)), 0.0f, 0.5f * sin(glm::radians(angle2_m)))));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	color = { 1.0f, 1.0f, 0.7f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	gluSphere(planet, 0.06, 20, 10);

	// 3�� ����
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, movement);
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	color = { 1.0f, 1.0f, 1.0f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	glDrawArrays(GL_LINE_LOOP, 0, 360);

	// -�༺
	m_model = glm::mat4(1.0f);
	m_model = glm::translate(m_model, movement);
	m_model = glm::rotate(m_model, (float)glm::radians(yRotate), glm::vec3(0.0f, 1.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::rotate(m_model, (float)glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_model = glm::translate(m_model, glm::vec3(glm::vec3(2.0f * cos(glm::radians(angle3)), 0.0f, 2.0f * sin(glm::radians(angle3)))));
	m_model = glm::rotate(m_model, (float)glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	color = { 0.8f, 0.4f, 0.1f };
	glUniform3f(colorLocation, color.r, color.g, color.b);

	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	gluSphere(planet, 0.18, 20, 10);

	// -����
	color = { 1.0f, 1.0f, 1.0f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	glDrawArrays(GL_LINE_LOOP, 360, 360);

	m_model = glm::translate(m_model, glm::vec3(glm::vec3(0.5f * cos(glm::radians(angle3_m)), 0.0f, 0.5f * sin(glm::radians(angle3_m)))));
	glUniformMatrix4fv(modelMat, 1, GL_FALSE, glm::value_ptr(m_model));

	color = { 0.4f, 0.0f, 0.6f };
	glUniform3f(colorLocation, color.r, color.g, color.b);
	gluSphere(planet, 0.12, 20, 10);



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
	GLchar* vertexSource = filetobuf("vertex_pr18.vert");

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
	GLchar* fragmentSource = filetobuf("fragment_pr18.frag");

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
	for (int i = 0; i < 360; ++i)
	{
		vPos.push_back(glm::vec3(2.0f * cos(glm::radians((float)i)), 0.0f, 2.0f * sin(glm::radians((float)i))));
	}
	for (int i = 0; i < 360; ++i)
	{
		vPos.push_back(glm::vec3(0.5f * cos(glm::radians((float)i)), 0.0f, 0.5f * sin(glm::radians((float)i))));
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(glm::vec3), vPos.data(), GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(s_program, "vPos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	set_vert();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'm':
	case 'M':
		isFill = !isFill;
		break;

	case 'y':
	case 'Y':
		if (yDir == 0.0f)
			yDir = 1.0f;
		else
			yDir *= -1;
		break;

	case 'w':
	case 'W':
		movement.y += 0.1f;
		break;

	case 's':
	case 'S':
		movement.y -= 0.1f;
		break;

	case 'a':
	case 'A':
		movement.x -= 0.1f;
		break;

	case 'd':
	case 'D':
		movement.x += 0.1f;
		break;

	case 'z':
	case 'Z':
		movement.z += 0.1f;
		break;

	case 'x':
	case 'X':
		movement.z -= 0.1f;
		break;

	case 'p':
	case 'P':
		isPerspective = !isPerspective;
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid Animate(int val)
{
	static const float speed1 = 2.0f, speed1_m = 3.0f, speed2 = 1.0f, speed2_m = 2.0f, speed3 = 0.2f, speed3_m = 0.8f;

	angle1 += speed1;
	angle1_m += speed1_m;
	angle2 += speed2;
	angle2_m += speed2_m;
	angle3 += speed3;
	angle3_m += speed3_m;

	yRotate += yDir;


	
	glutTimerFunc(10, Animate, 0);
	glutPostRedisplay();
}