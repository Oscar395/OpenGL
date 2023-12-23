#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <shader_s.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.h>

#include <model.h>

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void proccesInput(GLFWwindow* window);
void setupLights(Shader lightingshader);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400;
float lastY = 300;
bool firstMouse = true;

glm::vec3 lightPos(1.2f, 1.0f, 0.0f);

int main() {

	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);

	if (!window) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, frame_buffer_size_callback);
	// Set camera mouse input callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	// Scroll callback
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load openGL function pointers" << std::endl;
		glfwTerminate();
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	Shader lightShader("shaders/lightShader.vert", "shaders/lightShader.frag");
	Shader shaderSingleColor("shaders/lightShader.vert", "shaders/shaderSingleColor.frag");

	//Shader lightCubeShader("shaders/light_cube.vert", "shaders/light_cube.frag");

	vector<glm::vec3> vegetation;
	vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	Model Modelo("models/plano/plano.obj");
	Model Cube("models/cube/cube.obj");
	Model Tower("models/tower/wooden watch tower2.obj");
	Model Grass("models/grass/grass.obj");

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = (float) glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		proccesInput(window);

		std::map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < vegetation.size(); i++)
		{
			float distance = glm::length(camera.Position - vegetation[i]);
			sorted[distance] = vegetation[i];
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		float TIME = (sin(currentFrame) / 2.0f) + 0.05f;

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		shaderSingleColor.use();
		shaderSingleColor.setMat4("view", view);
		shaderSingleColor.setMat4("projection", projection);

		lightShader.use();
		lightShader.setFloat("material.shininess", 32.0f);
		// view/projection transformations
		lightShader.setVec3("viewPos", camera.Position);
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);
		lightShader.setFloat("time", TIME);

		glStencilMask(0x00);

		//plane && lights
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		lightShader.setMat4("model", model);
		setupLights(lightShader);
		Modelo.Draw(lightShader);

		//tower
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, -1.8f, -4.0f));
		lightShader.setMat4("model", model);
		Tower.Draw(lightShader);

		glDisable(GL_CULL_FACE);
		// windows
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			lightShader.setMat4("model", model);
			Grass.Draw(lightShader);
		}
		glEnable(GL_CULL_FACE);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		//cubes
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f + i * 3));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			lightShader.setMat4("model", model);
			Cube.Draw(lightShader);
		}

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		shaderSingleColor.use();
		float scale = 1.1f;

		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f + i * 3));
			model = glm::scale(model, glm::vec3(scale, scale, scale));
			shaderSingleColor.setMat4("model", model);
			Cube.Draw(shaderSingleColor);
		}

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteProgram(lightShader.ID);
	//glDeleteProgram(lightCubeShader.ID);
	glDeleteProgram(shaderSingleColor.ID);

	glfwTerminate();

	return 0;
}

void proccesInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		std::cout << "Polygon Mode On" << std::endl;
	}

	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		std::cout << "Polygon Mode Off" << std::endl;
	}

	// Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void setupLights(Shader lightShader) {
	// directional light
	lightShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
	lightShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	lightShader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
	lightShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

	glm::vec3 pointLightColor = glm::vec3(0.05f, 0.05f, 0.05f);

	glm::vec3 pointLightPosition = glm::vec3(0.7f, 0.2f, 2.0f);

	// Point light 1
	lightShader.setVec3("pointLights[0].position", pointLightPosition.x, pointLightPosition.y, pointLightPosition.z);
	lightShader.setVec3("pointLights[0].ambient", pointLightColor.x, pointLightColor.y, pointLightColor.z);
	lightShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	lightShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	lightShader.setFloat("pointLights[0].constant", 1.0f);
	lightShader.setFloat("pointLights[0].linear", 0.09f);
	lightShader.setFloat("pointLights[0].quadratic", 0.032f);

	// spotLight
	lightShader.setVec3("spotLight.position", camera.Position);
	lightShader.setVec3("spotLight.direction", camera.Front);
	lightShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	lightShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	lightShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	lightShader.setFloat("spotLight.constant", 1.0f);
	lightShader.setFloat("spotLight.linear", 0.09f);
	lightShader.setFloat("spotLight.quadratic", 0.032f);
	lightShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	lightShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}