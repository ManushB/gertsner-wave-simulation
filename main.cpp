#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl_core_3_3.h"
#include <GL/freeglut.h>
//#include <GL/glui.h>
#include "util.hpp"
#include "stb_image.h"


using namespace std;
using namespace glm;

// Global state
GLint width, height;
unsigned int viewmode;	// View triangle or obj file
GLuint shader;			// Shader program
GLuint shader2;			// Shader program
GLuint uniXform;		// Shader location of xform mtx
GLuint uniXform2;		// Shader location of xform mtx
GLuint vao;				// Vertex array object
GLuint vao2;				// Vertex array object
GLuint vbuf;			// Vertex buffer
GLuint vbuf2;			// Vertex buffer
GLuint ebuf;			// Element buffer
GLuint ebuf2;			// Element buffer
GLsizei vcount;			// Number of vertices
GLsizei islandVcount;			// Number of vertices
// Camera state
vec3 camCoords;			// Spherical coordinates (theta, phi, radius) of the camera
bool camRot;			// Whether the camera is currently rotating
vec2 camOrigin;			// Original camera coordinates upon clicking
vec2 mouseOrigin;		// Original mouse coordinates upon clicking

const int MENU_AMP1_1 = 1;		// Toggle amp
const int MENU_AMP1_2 = 2;		// Toggle amp
const int MENU_AMP1_3 = 3;		// Toggle amp
const int MENU_Low = 11;		// Toggle amp
const int MENU_Medium = 12;		// Toggle amp
const int MENU_High = 13;		// Toggle amp


const int MENU_PHS_1 = 41;		// Toggle phase
const int MENU_PHS_2 = 42;		// Toggle phase
const int MENU_PHS_3 = 43;		// Toggle phase


const int MENU_EXIT = 10;			// Exit application

GLfloat amp1 = 0.03;
GLfloat amp2 = 0.05;
GLfloat amp3 = 0.03;
GLfloat amp4 = 0.01;
GLfloat phase = 0.3;

int kImageDim = 512;
const int gridpts = 512;
unsigned char* image_data;
int image_height = 0, image_width = 0, num_channels = 0;
static int winId = -1;


GLuint texture;
GLuint tex_unit_location;
GLuint tex_unit_location2;
GLuint tex_id;

// Initialization functions
void initState();
void initGLUT(int* argc, char** argv);
void initOpenGL();
void prepareQuad();
void prepareIsland();
void prepareTexture();

// Callback functions
void display();
void reshape(GLint width, GLint height);
void keyRelease(unsigned char key, int x, int y);
void mouseBtn(int button, int state, int x, int y);
void mouseMove(int x, int y);
void idle();
void menu(int cmd);
void cleanup();

//static GLUI *glui;


struct Vertex {
	glm::vec3 pos;
	glm::vec3 texture_uv;
};

std::vector<Vertex> verts = {};
std::vector<Vertex> islandVerts = {};

struct index {
	GLuint a, b, c;
};

std::vector<index> indices = {};
std::vector<index> islandIndices = {};

//void MakeGUI()
//{
//	glui = GLUI_Master.create_glui("GUI", 0, 0, 0);
//	glui->add_statictext("GPGPU example");
//
//	glui->set_main_gfx_window(winId);
//
//	/* We register the idle callback with GLUI, *not* with GLUT */
//	GLUI_Master.set_glutIdleFunc(idle);
//}

int main(int argc, char** argv) {
	try {
		// Initialize
		initState();
		initGLUT(&argc, argv);
		initOpenGL();
		prepareQuad();
		prepareIsland();
		prepareTexture();
	}
	catch (const exception & e) {
		// Handle any errors
		cerr << "Fatal error: " << e.what() << endl;
		cleanup();
		return -1;
	}

	// Execute main loop
	glutMainLoop();

	return 0;
}

void initState() {
	// Initialize global state
	width = 0;
	height = 0;
	shader = 0;
	shader2 = 0;
	vao = 0;
	vao2 = 1;
	vbuf = 0;
	vbuf2 = 1;
	vcount = 0;
	islandVcount = 0;
	uniXform = 0;
	uniXform2 = 0;
	camCoords = vec3(-10.0, -70.0, 6.0);
	camRot = false;
}

void initGLUT(int* argc, char** argv) {
	// Set window and context settings
	width = 800; height = 600;
	glutInit(argc, argv);
	glutInitWindowSize(width, height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Create the window
	glutCreateWindow("Assignment 3");

	// Create submenu
	int amp1Menu = glutCreateMenu(menu);
	glutAddMenuEntry("0.01", MENU_AMP1_1);
	glutAddMenuEntry("0.05", MENU_AMP1_2);
	glutAddMenuEntry("0.09", MENU_AMP1_3);

	int amp2Menu = glutCreateMenu(menu);
	glutAddMenuEntry("High", MENU_High);
	glutAddMenuEntry("Medium", MENU_Medium);
	glutAddMenuEntry("Low", MENU_Low);

	int phaseMenu = glutCreateMenu(menu);
	glutAddMenuEntry("0.1", MENU_PHS_1);
	glutAddMenuEntry("0.4", MENU_PHS_2);
	glutAddMenuEntry("0.6", MENU_PHS_3);

	// Create a menu
	glutCreateMenu(menu);
	glutAddSubMenu("Toggle Amplitude", amp2Menu);
	glutAddSubMenu("Toggle Amplitude By Value", amp1Menu);
	glutAddSubMenu("Toggle Phase", phaseMenu);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// GLUT callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyRelease);
	glutMouseFunc(mouseBtn);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);
	glutCloseFunc(cleanup);

	//MakeGUI();
}

void initOpenGL() {
	// Set clear color and depth
	glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
	glClearDepth(1.0f);
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Compile and link shader program
	vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "sh_v.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "sh_f.glsl"));
	shader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	uniXform = glGetUniformLocation(shader, "xform");
	shaders.clear();

	// Compile and link shader program
	vector<GLuint> shaders2;
	shaders2.push_back(compileShader(GL_VERTEX_SHADER, "sh_v2.glsl"));
	shaders2.push_back(compileShader(GL_FRAGMENT_SHADER, "sh_f2.glsl"));
	shader2 = linkProgram(shaders2);
	// Release shader sources
	for (auto s = shaders2.begin(); s != shaders2.end(); ++s)
		glDeleteShader(*s);
	uniXform2 = glGetUniformLocation(shader2, "xform");
	shaders2.clear();
	assert(glGetError() == GL_NO_ERROR);
}

void prepareQuad() {
	// Vertices of the quad, which takes up 80% of the window both horizontally and vertically.

	float x = -0.8f;
	float y = -0.8f;
	float u = 1.0f;
	float v = 1.0f;
	for (int i = 0; i < gridpts; i++) {
		for (int j = 0; j < gridpts; j++) {
			Vertex ver;
			ver.pos = glm::vec3(x, y, 0.0f);
			ver.texture_uv = glm::vec3(u, v, 0.0f);
			verts.push_back(ver);
			x = x + 0.003125f; // 0.8 *2 /512
			u = u - 0.001953125f; // 1.0/40
		}
		v = v - 0.001953125f;
		y = y + 0.003125f;
		x = -0.8f;
		u = 1.0f;
	}

	for (int i = 0; i < gridpts - 1; i++) {
		for (int j = 0; j < gridpts - 1; j++) {
			index t1 = { i * gridpts + j, i * gridpts + j + 1, (i + 1) * gridpts + j };
			index t2 = { (i + 1) * gridpts + j, i * gridpts + j + 1, (i + 1) * gridpts + j + 1 };

			indices.push_back(t1);
			indices.push_back(t2);
		}
	}
	vcount = verts.size();
	std::cout << "The number of vertices in grid are " << vcount << std::endl;

	assert(glGetError() == GL_NO_ERROR);
}

void prepareIsland() {
	// Vertices of the quad, which takes up 80% of the window both horizontally and vertically.

	float x = -0.2f;
	float y = -0.2f;
	float u = 1.0f;
	float v = 1.0f;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			Vertex ver;
			ver.pos = glm::vec3(x, y, 100.0f);
			//ver.texture_uv = glm::vec3(u, v, 0.0f);
			islandVerts.push_back(ver);
			x = x + 0.04f; // 0.8 *2 /10
			u = u - 0.01f; // 1.0/10
		}
		v = v - 0.01f;
		y = y + 0.04f;
		x = -0.2f;
		u = 1.0f;
	}

	for (int i = 0; i < 10 - 1; i++) {
		for (int j = 0; j < 10 - 1; j++) {
			index t1 = { i * 10 + j, i * 10 + j + 1, (i + 1) * 10 + j };
			index t2 = { (i + 1) * 10 + j, i * 10 + j + 1, (i + 1) * 10 + j + 1 };

			islandIndices.push_back(t1);
			islandIndices.push_back(t2);
		}
	}
	islandVcount = islandVerts.size();
	std::cout << "The number of vertices in grid are " << islandVcount << std::endl;

	assert(glGetError() == GL_NO_ERROR);
}

void prepareTexture() {
	image_data = stbi_load("water.jpg", &image_width, &image_height, &num_channels, 0);

	// Do some simple checking.
	if (image_data == nullptr) {
		std::cerr << "Image reading failed." << std::endl;
	}
	else if (num_channels != 3 && num_channels != 4) {
		std::cerr << "The loaded image doesn't have RGB color components." << std::endl;
		std::cerr << "The loaded image has " << num_channels << " channels" << std::endl;
	}
	else {
		std::cout << "The image loaded has size " << image_width << "x" << image_height << std::endl;
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glGenBuffers(1, &ebuf);

	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texture_uv));
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D,				// The texture is 2D. (guess what, 1D and 3D are also supported by OpenGL.)
		0,							// level of mipmapping. Just make this 0.
		GL_RGB,					// How the image should be represented.
		kImageDim, kImageDim,		// width and height of the image
		0,							// Don't worry about this.
		GL_RGB,					// The format of the image data source. `image_data` has 3 channels.
		GL_UNSIGNED_BYTE,			// Data type of the image data source. `image_data` is an array of unsigned char.
		image_data);
	// Sets wrapping and filtering of the texture. Don't worry about them - just copy/paste.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generates mipmapping for better sampling.
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	stbi_image_free(image_data);

	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);
	glGenBuffers(1, &vbuf2);
	glGenBuffers(1, &ebuf2);

	glBindBuffer(GL_ARRAY_BUFFER, vbuf2);
	glBufferData(GL_ARRAY_BUFFER, islandVerts.size() * sizeof(islandVerts[0]), islandVerts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuf2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, islandIndices.size() * sizeof(islandIndices[0]), islandIndices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texture_uv));
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	assert(glGetError() == GL_NO_ERROR);
}

void display() {
	try {
		// Clear the back buffer
		glClearColor(0.1f, 0.2f, 0.25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get ready to draw
		glUseProgram(shader);
		mat4 xform;
		float aspect = (float)width / (float)height;
		// Create perspective projection matrix
		mat4 proj = perspective(45.0f, aspect, 0.1f, 100.0f);
		// Create view transformation matrix
		mat4 view = translate(mat4(1.0f), vec3(0.0, 0.0, -camCoords.z));
		mat4 rot = rotate(mat4(1.0f), radians(camCoords.y), vec3(1.0, 0.0, 0.0));
		rot = rotate(rot, radians(camCoords.x), vec3(0.0, 1.0, 0.0));
		xform = proj * view * rot;

		glBindVertexArray(vao);		
		tex_unit_location = glGetUniformLocation(shader, "tex_unit");
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(tex_unit_location, 0);
		glUniformMatrix4fv(uniXform, 1, GL_FALSE, value_ptr(xform));
		glUniform1f(3, 0.003 * glutGet(GLUT_ELAPSED_TIME));
		glUniform1f(4, amp1);
		glUniform1f(5, amp2);
		glUniform1f(6, amp3);
		glUniform1f(7, amp4);
		glUniform1f(8, phase);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, 0);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(0);

		glUseProgram(shader2);
		glBindVertexArray(vao2);
		glUniformMatrix4fv(uniXform2, 1, GL_FALSE, value_ptr(xform));
		glDrawElements(GL_TRIANGLES, 3 * islandIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		assert(glGetError() == GL_NO_ERROR);

		// Revert context state
		glUseProgram(0);

		// Display the back buffer
		glutSwapBuffers();
		//stbi_image_free(image_data);

	}
	catch (const exception & e) {
		cerr << "Fatal error: " << e.what() << endl;
		glutLeaveMainLoop();
	}
}

void reshape(GLint width, GLint height) {
	::width = width;
	::height = height;
	glutReshapeWindow(800, 600);
	//glViewport(0, 0, width, height);
}

void keyRelease(unsigned char key, int x, int y) {
	switch (key) {
	case 27:	// Escape key
		menu(MENU_EXIT);
		break;
	}
}

void mouseBtn(int button, int state, int x, int y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		// Activate rotation mode
		camRot = true;
		camOrigin = vec2(camCoords);
		mouseOrigin = vec2(x, y);

		glutPostRedisplay();
	}
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		// Deactivate rotation
		camRot = false;
	}
	if (button == 3) {
		camCoords.z = clamp(camCoords.z - 0.1f, 0.1f, 10.0f);
		glutPostRedisplay();
	}
	if (button == 4) {
		camCoords.z = clamp(camCoords.z + 0.1f, 0.1f, 10.0f);
		glutPostRedisplay();
	}
}

void mouseMove(int x, int y) {
	if (camRot) {
		// Convert mouse delta into degrees, add to rotation
		float rotScale = std::min(width / 450.0f, height / 270.0f);
		vec2 mouseDelta = vec2(x, y) - mouseOrigin;
		vec2 newAngle = camOrigin + mouseDelta / rotScale;
		newAngle.y = clamp(newAngle.y, -90.0f, 90.0f);
		while (newAngle.x > 180.0f) newAngle.x -= 360.0f;
		while (newAngle.y < -180.0f) newAngle.y += 360.0f;
		if (length(newAngle - vec2(camCoords)) > FLT_EPSILON) {
			camCoords.x = newAngle.x;
			camCoords.y = newAngle.y;
			glutPostRedisplay();
		}
	}
}

void idle() {
	glutPostRedisplay();
}

void menu(int cmd) {
	switch (cmd) {
	case MENU_AMP1_1:
		amp1 = 0.01;
		amp2 = 0.04;
		amp3 = 0.06;
		glutPostRedisplay();
		break;

	case MENU_AMP1_2:
		amp1 = 0.05;
		amp2 = 0.04;
		amp3 = 0.06;
		glutPostRedisplay();
		break;

	case MENU_AMP1_3:
		amp1 = 0.09;
		amp2 = 0.04;
		amp3 = 0.06;
		glutPostRedisplay();
		break;

	case MENU_Low:
		amp1 = 0.02;
		amp2 = 0.04;
		amp3 = 0.06;
		glutPostRedisplay();
		break;

	case MENU_Medium:
		amp1 = 0.06;
		amp2 = 0.04;
		amp3 = 0.06;
		glutPostRedisplay();
		break;

	case MENU_High:
		amp1 = 0.09;
		amp2 = 0.04;
		amp3 = 0.06;
		glutPostRedisplay();
		break;

	case MENU_PHS_1:
		phase = 0.1;
		glutPostRedisplay();
		break;

	case MENU_PHS_2:
		phase = 0.4;
		glutPostRedisplay();
		break;

	case MENU_PHS_3:
		phase = 0.6;
		glutPostRedisplay();
		break;


	case MENU_EXIT:
		glutLeaveMainLoop();
		break;
	}
}

void cleanup() {
	// Release all resources
	if (shader) { glDeleteProgram(shader); shader = 0; }
	if (shader2) { glDeleteProgram(shader2); shader2 = 0; }
	uniXform = 0;
	uniXform2 = 0;
	if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
	if (vao2) { glDeleteVertexArrays(1, &vao2); vao2 = 0; }
	if (vbuf) { glDeleteBuffers(1, &vbuf); vbuf = 0; }
	if (vbuf2) { glDeleteBuffers(1, &vbuf2); vbuf2 = 0; }
	vcount = 0;
	islandVcount = 0;
}