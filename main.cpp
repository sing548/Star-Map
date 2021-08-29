#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <openvr.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "journal_reader.h"

#include <iostream>

#include <string>

static void error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawOutput(glm::vec4 backgroundColor, Shader shader, JournalReader jR, Model loadedModel);
void drawOutputToTexture(glm::vec4 backgroundColor, Shader shader, Shader screenShader, JournalReader jR, unsigned int framebuffer, unsigned int textColorBuffer, unsigned int quadVAO);
glm::mat4 toGLM(const vr::HmdMatrix34_t& m);
void drawCorrectStarModel(Coordinate c, Shader shader);

//settings
const unsigned int SRC_WIDTH = 2560;
const unsigned int SRC_HEIGHT = 1080;

//camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SRC_WIDTH / 2.0f;
float lastY = SRC_HEIGHT / 2.0f;
bool firstMouse = true;

//timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Model genericStarModel		;//= Model("resources/models/stars/generic_star/star.obj");
Model classASpotlessModel	;//= Model("resources/models/stars/a_spotless/a_spotless.obj");
Model classASpotsModel		;//= Model("resources/models/stars/a_with_spots/a_with_spots.obj");
Model classBModel			;//= Model("resources/models/stars/b/b.obj");
Model classFModel			;//= Model("resources/models/stars/f/f.obj");
Model classGModel			;//= Model("resources/models/stars/g/g.obj");
Model classKModel			;//= Model("resources/models/stars/k/k.obj");
Model classLModel			;//= Model("resources/models/stars/l/l.obj");
Model classMModel			;//= Model("resources/models/stars/m/m.obj");
Model classOModel			;//= Model("resources/models/stars/o/o.obj");
Model classTModel			;//= Model("resources/models/stars/t/t.obj");
Model wolfRayetModel		;//= Model("resources/models/stars/wolf_rayet/wolf_rayet.obj");
Model classYModel			;//= Model("resources/models/stars/y/y.obj");

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwSetErrorCallback(error_callback);

	JournalReader jR = JournalReader();
	jR.readAllJounals("C:\\Users\\dario\\Saved Games\\Frontier Developments\\Elite Dangerous");

	//GLFW Fenster (zum Gucken!)
	GLFWwindow* window;

	//OpenVRPart vrPart;
	//window = glfwCreateWindow(vrPart.rtWidth, vrPart.rtHeight, "Hello OpenVR", NULL, NULL);
	//window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "HelloWindow", glfwGetPrimaryMonitor(), NULL);
	window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "HelloWindow", NULL, NULL);
	

	if (window == NULL)
	{
		std::cout << "Oh noez: GLFW window" << std::endl;
		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//glad: OpenGl Pointer Funktionen
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Oh noez: GLAD" << std::endl;
		return -1;
	}

	//stbi_set_flip_vertically_on_load(true);
	glEnable(GL_DEPTH_TEST);

	// Shader bauen
	Shader ourShader("model_loading.vert", "model_loading.frag");
	Shader screenShader("screen.vert", "screen.frag");

	// Model laden
	genericStarModel    = Model("resources/models/stars/generic_star/star.obj");
	classASpotlessModel = Model("resources/models/stars/a_spotless/a_spotless.obj");
	classASpotsModel	= Model("resources/models/stars/a_with_spots/a_with_spots.obj");
	classBModel			= Model("resources/models/stars/b/b.obj");
	classFModel			= Model("resources/models/stars/f/f.obj");
	classGModel			= Model("resources/models/stars/g/g.obj");
	classKModel			= Model("resources/models/stars/k/k.obj");
	classLModel			= Model("resources/models/stars/l/l.obj");
	classMModel			= Model("resources/models/stars/m/m.obj");
	classOModel			= Model("resources/models/stars/o/o.obj");
	classTModel			= Model("resources/models/stars/t/t.obj");
	wolfRayetModel		= Model("resources/models/stars/wolf_rayet/wolf_rayet.obj");
	classYModel			= Model("resources/models/stars/y/y.obj");

	glm::vec4 backgroundRGBA = glm::vec4(0.01f, 0.01f, 0.01f, 1.00f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	screenShader.use();
	screenShader.setInt("screenTexture", 0);

	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SRC_WIDTH, SRC_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SRC_WIDTH, SRC_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		//drawOutput(backgroundRGBA, ourShader, jR, loadedModel);
		drawOutputToTexture(backgroundRGBA, ourShader, screenShader, jR, framebuffer, textureColorbuffer, quadVAO);

		//vrPart.submitFramesToOpenVR(result, result);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void drawOutput(glm::vec4 backgroundColor, Shader shader, JournalReader jR)
{
	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 500.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	/*for (unsigned int i = 0; i < jR.mVisitedCoordinates.size(); i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, jR.mVisitedCoordinates[i].coords);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		shader.setMat4("model", model);
		loadedModel.Draw(shader);
	}*/

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setMat4("model", model);
	genericStarModel.Draw(shader);
}

void drawOutputToTexture(glm::vec4 backgroundColor, Shader shader, Shader screenShader, JournalReader jR, unsigned int framebuffer, unsigned int textColorBuffer, unsigned int quadVAO)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);

	glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SRC_WIDTH / (float)SRC_HEIGHT, 0.1f, 500.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	for (unsigned int i = 0; i < jR.mVisitedCoordinates.size(); i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, jR.mVisitedCoordinates[i].coords);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		shader.setMat4("model", model);
		drawCorrectStarModel(jR.mVisitedCoordinates[i], shader);
	}

	/*glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setMat4("model", model);
	classASpotsModel.Draw(shader);*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, textColorBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(SPRINT, deltaTime);
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(JOG, deltaTime);
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(CRAWL, deltaTime);
	
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
		camera.ProcessKeyboard(WALK, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

static void error_callback(int error, const char* description)
{
	cout << "Error " << error << " : " << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	
	if (data)
	{
		GLenum format;

		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void drawCorrectStarModel(Coordinate c, Shader shader)
{
	switch (c.starClass)
	{
		case StarClass::O: classOModel.Draw(shader); break;
		case StarClass::B: classBModel.Draw(shader); break;
		case StarClass::A: 
		{
			classASpotsModel.Draw(shader); break;
		}
		case StarClass::F: classFModel.Draw(shader); break;
		case StarClass::G: classGModel.Draw(shader); break;
		case StarClass::K: classKModel.Draw(shader); break;
		case StarClass::L: classLModel.Draw(shader); break;
		case StarClass::M: classMModel.Draw(shader); break;
		case StarClass::T: classTModel.Draw(shader); break;
		case StarClass::Y: classYModel.Draw(shader); break;
		case StarClass::D: wolfRayetModel.Draw(shader); break;
		case StarClass::GENERIC: classASpotsModel.Draw(shader); break;
	}
}
