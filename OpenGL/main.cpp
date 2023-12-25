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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	Shader lightShader("shaders/lightShader.vert", "shaders/lightShader.frag");
	Shader shaderSingleColor("shaders/lightShader.vert", "shaders/shaderSingleColor.frag");
	Shader screenShader("shaders/framebuffers_screen.vert", "shaders/framebuffers_screen.frag");

	//Shader lightCubeShader("shaders/light_cube.vert", "shaders/light_cube.frag");

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
		// positions   // texCoords
		-0.3f,  1.0f,  0.0f, 1.0f,
		-0.3f,  0.7f,  0.0f, 0.0f,
		 0.3f,  0.7f,  1.0f, 0.0f,

		-0.3f,  1.0f,  0.0f, 1.0f,
		 0.3f,  0.7f,  1.0f, 0.0f,
		 0.3f,  1.0f,  1.0f, 1.0f
	};

	// screen quad VAO
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

	// Framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERRO::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


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

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float TIME = (sin(currentFrame) / 2.0f) + 0.05f;

		glm::mat4 model = glm::mat4(1.0f);
		camera.Yaw += 180.f;
		camera.ProcessMouseMovement(0, 0, false);
		glm::mat4 view = camera.GetViewMatrix();
		camera.Yaw -= 180.0f;
		camera.ProcessMouseMovement(0, 0, true);
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setFloat("material.shininess", 32.0f);
		// view/projection transformations
		lightShader.setVec3("viewPos", camera.Position);
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);
		lightShader.setFloat("time", TIME);

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
		for (unsigned int i = 0; i < vegetation.size(); i++){
			model = glm::mat4(1.0f);
			model = glm::translate(model, vegetation[i]);
			lightShader.setMat4("model", model);
			Grass.Draw(lightShader);
		}
		glEnable(GL_CULL_FACE);

		//cubes
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f + i * 3));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			lightShader.setMat4("model", model);
			Cube.Draw(lightShader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		view = camera.GetViewMatrix();
		lightShader.setMat4("view", view);

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
		for (unsigned int i = 0; i < vegetation.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, vegetation[i]);
			lightShader.setMat4("model", model);
			Grass.Draw(lightShader);
		}
		glEnable(GL_CULL_FACE);

		//cubes
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f + i * 3));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			lightShader.setMat4("model", model);
			Cube.Draw(lightShader);
		}

		glDisable(GL_DEPTH_TEST);

		screenShader.use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glDeleteProgram(lightShader.ID);
	//glDeleteProgram(lightCubeShader.ID);
	glDeleteProgram(shaderSingleColor.ID);
	glDeleteProgram(screenShader.ID);

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &framebuffer);

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