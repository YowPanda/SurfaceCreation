#include <iostream>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include "Matrix.h"

using namespace std;

// Клеток в стороне сетки
#define dotCount 100

GLFWwindow *g_window;

GLuint g_shaderProgram;

GLint g_uMVP;
GLint g_uMV;
GLint g_uN;
GLint g_uL;

GLfloat g_delta;

class Model
{
public:
	GLuint vbo;
	GLuint ibo;
	GLuint vao;
	GLsizei indexCount;
};

Model g_model;

GLuint createShader(const GLchar *code, GLenum type)
{
	GLuint result = glCreateShader(type);

	glShaderSource(result, 1, &code, NULL);
	glCompileShader(result);

	GLint compiled;
	glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0)
		{
			char *infoLog = (char *)alloca(infoLen);
			glGetShaderInfoLog(result, infoLen, NULL, infoLog);
			cout << "Shader compilation error" << endl << infoLog << endl;
		}
		glDeleteShader(result);
		return 0;
	}

	return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh)
{
	GLuint result = glCreateProgram();

	glAttachShader(result, vsh);
	glAttachShader(result, fsh);

	glLinkProgram(result);

	GLint linked;
	glGetProgramiv(result, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint infoLen = 0;
		glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0)
		{
			char *infoLog = (char *)alloca(infoLen);
			glGetProgramInfoLog(result, infoLen, NULL, infoLog);
			cout << "Shader program linking error" << endl << infoLog << endl;
		}
		glDeleteProgram(result);
		return 0;
	}

	return result;
}

bool createShaderProgram()
{
	g_shaderProgram = 0;

	const GLchar vsh[] =
		"#version 330\n"
		""
		"layout(location = 0) in vec2 a_position;"
		""
		"uniform mat3 u_n;"
		"uniform mat4 u_mv;"
		"uniform mat4 u_mvp;"
		"uniform float delta;"
		""
		"out vec3 v_normal;"
		"out vec3 v_pos;"
		""
		"void main()"
		"{"
		"	float sin_delta = sin(delta);"
		"	vec4 pos = vec4(a_position[0], sin(5.0 * sin_delta * a_position[0] + 1.0) * cos(7.0 * sin_delta * a_position[1] + 1.0) * 0.1 + sin_delta * 0.1, a_position[1], 1.0);"
		"	float dx = -0.5 * sin_delta * cos(5.0 * a_position[0] * sin_delta + 1.0) * cos(7.0 * a_position[1] * sin_delta + 1.0);"
		"	float dz = 0.7 * sin_delta * sin(5.0 * a_position[0] * sin_delta + 1.0) * sin(7.0 * a_position[1] * sin_delta + 1.0);"
		"	v_normal = normalize(u_n * vec3(dx, 1.0, dz));"
		"	v_pos = (u_mv * pos).xyz;"
		"   gl_Position = u_mvp * pos;"
		"}"
		;

	const GLchar fsh[] =
		"#version 330\n"
		""
		"layout(location = 0) out vec4 o_color;"
		""
		"in vec3 v_normal;"
		"in vec3 v_pos;"
		""
		"uniform vec3 u_l;"
		""
		"void main()"
		"{"
		"	vec3 n = normalize(v_normal);"
		"	vec3 l = normalize(v_pos - u_l);"
		"	float cosa = dot(-l, n);"
		"	float d = max(cosa, 0.1);"
		"	vec3 e = normalize(-v_pos);"
		"	vec3 r = reflect(l, n);"
		"	float s = int(cosa > 0) * pow(max(dot(r, e), 0.0), 7.0);"
		"	o_color = vec4(d * vec3(0.0, 0.9, 0.9) + s * vec3(1.0), 1.0);"
		"}"
		;

	GLuint vertexShader, fragmentShader;

	vertexShader = createShader(vsh, GL_VERTEX_SHADER);
	fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

	g_shaderProgram = createProgram(vertexShader, fragmentShader);

	// Программно запрашиваем декриптор юниформ переменной
	g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");
	g_uMV = glGetUniformLocation(g_shaderProgram, "u_mv");
	g_uN = glGetUniformLocation(g_shaderProgram, "u_n");
	g_uL = glGetUniformLocation(g_shaderProgram, "u_l");
	g_delta = glGetUniformLocation(g_shaderProgram, "delta");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return g_shaderProgram != 0;
}

bool createModel()
{
	GLfloat *vertices = new GLfloat[(dotCount + 1) * (dotCount + 1) * 2];
	GLint *indices = new GLint[dotCount * dotCount * 6];

	int ind = 0;
	int half = dotCount / 2;

	for (int i = -half; i <= half; i++)
	{
		for (int j = -half; j <= half; j++)
		{
			vertices[ind++] = float(j) / dotCount;
			vertices[ind++] = float(i) / dotCount;
		}
	}

	ind = 0;

	for (int i = 0; i < dotCount; i++)
	{
		for (int j = 0; j < dotCount; j++)
		{
			indices[ind++] = (dotCount + 1) * i + j;
			indices[ind++] = (dotCount + 1) * i + j + 1;
			indices[ind++] = (dotCount + 1) * i + j + 2 + dotCount;
			indices[ind++] = (dotCount + 1) * i + j + 2 + dotCount;
			indices[ind++] = (dotCount + 1) * i + j + 1 + dotCount;
			indices[ind++] = (dotCount + 1) * i + j;
		}
	}

	glGenVertexArrays(1, &g_model.vao);
	glBindVertexArray(g_model.vao);

	glGenBuffers(1, &g_model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
	glBufferData(GL_ARRAY_BUFFER, (dotCount + 1) * (dotCount + 1) * 2 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_model.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dotCount * dotCount * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	g_model.indexCount = dotCount * dotCount * 6;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const GLvoid*)0);

	delete [] vertices;
	delete [] indices;

	return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init()
{
	// Set initial color of color buffer to white.
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	// Включаем тест глубины
	glEnable(GL_DEPTH_TEST);

	return createShaderProgram() && createModel();
}

void reshape(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void draw(float ax, float ay, float az, float delta)
{
	int w = 0, h = 0;

	// Очищаем буфер цвета + инициализируем буер глубины
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_shaderProgram);
	glBindVertexArray(g_model.vao);

	glfwGetFramebufferSize(g_window, &w, &h);

	// Преобразования
	MyMatrix V, M, P, MV, MVP;

	V = MyMatrix::getView(MyVector(2.5, 1.5, 0.0), MyVector(0.0, 0.3, 0.0), MyVector(0.0, 1.0, 0.0));
	M = MyMatrix();
	P = MyMatrix::perspProjFOV(0.1, 100.0, w, h, 35.0);

	// Масштабирование
	M = MyMatrix::scale(M, 2.0, 2.0, 2.0);

	// Перенос
	M = MyMatrix::translate(M, -0.5, 0.0, 0.0);

	// Поворот
	M = MyMatrix::rotate(M, ax, 1.0, 0.0, 0.0);
	M = MyMatrix::rotate(M, ay + 45.0, 0.0, 1.0, 0.0);
	M = MyMatrix::rotate(M, az, 0.0, 0.0, 1.0);

	MV = V * M;
	MVP = P * MV;

	float l[3] { 0.0, 1.0, 0.0 };
	float n[9];
	memcpy(n, MyMatrix::getInvNotTransp(MV), 9 * sizeof(float));

	// Отправляем матрицы в шейдер перед отрисовкой
	glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, MVP.get());
	glUniformMatrix4fv(g_uMV, 1, GL_FALSE, MV.get());
	glUniform3fv(g_uL, 1, l);
	glUniformMatrix3fv(g_uN, 1, GL_FALSE, n);
	glUniform1f(g_delta, delta);

	glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void cleanup()
{
	if (g_shaderProgram != 0)
		glDeleteProgram(g_shaderProgram);
	if (g_model.vbo != 0)
		glDeleteBuffers(1, &g_model.vbo);
	if (g_model.ibo != 0)
		glDeleteBuffers(1, &g_model.ibo);
	if (g_model.vao != 0)
		glDeleteVertexArrays(1, &g_model.vao);
}

bool initOpenGL()
{
	// Initialize GLFW functions.
	if (!glfwInit())
	{
		cout << "Failed to initialize GLFW" << endl;
		return false;
	}

	// Request OpenGL 3.3 without obsoleted functions.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window.
	g_window = glfwCreateWindow(800, 600, "Test", NULL, NULL);
	if (g_window == NULL)
	{
		cout << "Failed to open GLFW window" << endl;
		glfwTerminate();
		return false;
	}

	// Initialize OpenGL context with.
	glfwMakeContextCurrent(g_window);

	// Set internal GLEW variable to activate OpenGL core profile.
	glewExperimental = true;

	// Initialize GLEW functions.
	if (glewInit() != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		return false;
	}

	// Ensure we can capture the escape key being pressed.
	glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);

	// Set callback for framebuffer resizing event.
	glfwSetFramebufferSizeCallback(g_window, reshape);

	return true;
}

void tearDownOpenGL()
{
	// Terminate GLFW.
	glfwTerminate();
}

int main()
{
	// Initialize OpenGL
	if (!initOpenGL())
		return -1;

	// Initialize graphical resources.
	bool isOk = init();

	if (isOk)
	{
		float ax = 0, ay = 0, az = 0, delta = 0;

		// Получаем время перед заходом в цикл
		auto callTime = chrono::system_clock::now();

		// Main loop until window closed or escape pressed.
		while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
		{
			if (glfwGetKey(g_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				ay += 0.5;
				Sleep(5);
			}
			if (glfwGetKey(g_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				ay -= 0.5;
				Sleep(5);
			}
			if (glfwGetKey(g_window, GLFW_KEY_UP) == GLFW_PRESS) {
				ax += 0.5;
				Sleep(5);
			}
			if (glfwGetKey(g_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				ax -= 0.5;
				Sleep(5);
			}	
		
			// Получаем время с запуска цикла
			chrono::duration<double> elapsed = callTime - chrono::system_clock::now();
			delta = elapsed.count();

			// Draw scene.
			draw(ax, ay, az, delta);

			// Swap buffers.
			glfwSwapBuffers(g_window);
			// Poll window events.
			glfwPollEvents();
		}
	}

	// Cleanup graphical resources.
	cleanup();

	// Tear down OpenGL.
	tearDownOpenGL();

	system("pause");

	return isOk ? 0 : -1;
}