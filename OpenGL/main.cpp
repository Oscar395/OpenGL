#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "scene.h"
#include "Objects/FrameBuffer.h"
#include "Objects/shader.h"
#include "Objects/model.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.h>
#include <cmath>
#include <limits>
#include <vector>

void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void proccesInput(GLFWwindow* window);
void setupLights(Shader lightingshader);
unsigned int loadCubemap(std::vector<std::string> faces);
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
std::vector<glm::mat4> getLightSpaceMatrices();

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float cameraNearPlane = 0.1f;
float cameraFarPlane = 500.0f;

// framebuffer size
int fb_width;
int fb_height;

std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };

// lighting info
glm::vec3 lightDir = glm::normalize(glm::vec3(40.0f, 50.0f, 20.0f));
constexpr unsigned int depthMapResolution = 1024;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400;
float lastY = 300;
bool firstMouse = true;
bool moveCamera = true;

int function() {

	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);

	//GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	//const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	//glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);

	//glfwSetWindowMonitor(window, nullptr, 0, 0, SCR_WIDTH, SCR_HEIGHT, 60);

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
	glfwSetKeyCallback(window, key_callback);

	// Scroll callback
	glfwSetScrollCallback(window, scroll_callback);
	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to load openGL function pointers" << std::endl;
		glfwTerminate();
		return -1;
	}

	stbi_set_flip_vertically_on_load(true);

	glEnable(GL_DEPTH_TEST);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	//glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_FRAMEBUFFER_SRGB);

	Shader lightShader("Resources/shaders/lightShader.vert", "Resources/shaders/lightShader.frag");
	//Shader shaderSingleColor("shaders/lightShader.vert", "shaders/shaderSingleColor.frag");
	Shader screenShader("Resources/shaders/framebuffers_screen.vert", "Resources/shaders/framebuffers_screen.frag");
	Shader skyboxShader("Resources/shaders/skybox.vert", "Resources/shaders/skybox.frag");
	Shader skyboxReflectShader("Resources/shaders/skybox_reflect.vert", "Resources/shaders/skybox_reflect.frag");
	Shader simpleDepthShader("Resources/shaders/simple_depth_shader.vert", "Resources/shaders/simple_depth_shader.frag", "Resources/shaders/simple_depth_shader.geom");

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	std::vector<std::string> faces
	{
		"Resources/textures/skybox/right.jpg",
		"Resources/textures/skybox/left.jpg",
		"Resources/textures/skybox/top.jpg",
		"Resources/textures/skybox/bottom.jpg",
		"Resources/textures/skybox/front.jpg",
		"Resources/textures/skybox/back.jpg"
	};

	unsigned int cubemapTextures = loadCubemap(faces);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

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

	// MSAA Framebuffer
	FrameBuffer sceneBuffer((float)fb_width, (float)fb_height, true);

	FrameBuffer postprocesingBuffer((float)fb_width, (float)fb_height, false);

	// cascade shadow map
	unsigned int lightFBO;
	glGenFramebuffers(1, &lightFBO);

	unsigned int lightDepthMaps;
	glGenTextures(1, &lightDepthMaps);
	glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapResolution, depthMapResolution, int(shadowCascadeLevels.size()) + 1,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMaps, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
		throw 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// matrices ubo
	//unsigned int uniformBlockDepthMatrices = glGetUniformBlockIndex(simpleDepthShader.ID, "LightSpaceMatrices");
	//unsigned int uniformBlockLightMatrices = glGetUniformBlockIndex(lightShader.ID, "LightSpaceMatrices");

	//glUniformBlockBinding(simpleDepthShader.ID, uniformBlockDepthMatrices, 0);
	//glUniformBlockBinding(lightShader.ID, uniformBlockLightMatrices, 0);

	unsigned int matricesUBO;
	glGenBuffers(1, &matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	lightShader.use();
	lightShader.setInt("shadowMap", 1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	Model Modelo("Resources/models/plano/plane.obj");
	Model Cube("Resources/models/cube/cube.obj");
	Model Tower("Resources/models/tower/wooden watch tower2.obj");
	Model Wall("Resources/models/wall/wall.obj");
	//Wall.addNormalTexture("brickwall_normal.jpg");
	//Model Grass("models/grass/grass.obj");

	Scene Escena;

	Escena.addObject(Modelo, glm::vec3(0.0f, -1.0f, 0.0f));
	Escena.addObject(Cube, glm::vec3(0.0f, 0.0f, 0.0f));
	Escena.addObject(Cube, glm::vec3(0.0f, -0.7f, 4.0f));
	Escena.addObject(Tower, glm::vec3(-2.0f, -1.7f, -4.0f));
	Escena.addObject(Wall, glm::vec3(-5.0f, 1.0f, 4.0f));

	bool shadows = true;
	bool gammaCorrection = true;
	float gammaValue = 2.2f;
	float height_scale = 0.1f;

	double lastTime = glfwGetTime();
	int nbFrames = 0;
	int displayedFrames = 0;
	int times = 2;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
			// printf and reset timer
			//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			//printf("%f fps\n", double(nbFrames));
			displayedFrames = nbFrames;
			nbFrames = 0;
			lastTime += 1.0;
		}

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		io.WantCaptureMouse = !moveCamera;

		if (!io.WantCaptureMouse) {
			proccesInput(window);
		}

		// depth shadow map rendering
		if (shadows) {
			const auto lightMatrices = getLightSpaceMatrices();
			glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
			for (size_t i = 0; i < lightMatrices.size(); ++i)
			{
				glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
			}
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			simpleDepthShader.use();

			glBindFramebuffer(GL_FRAMEBUFFER, lightFBO);
			glViewport(0, 0, depthMapResolution, depthMapResolution);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);
			//glDisable(GL_CULL_FACE);
			Escena.render(simpleDepthShader);
			//glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, fb_width, fb_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render normal
		// ---------------------------------------

		//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		sceneBuffer.Bind();

		glViewport(0, 0, fb_width, fb_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float TIME = (sin(currentTime) / 2.0f) + 0.05f;

		lightShader.use();
		const glm::mat4 projection = glm::perspective(camera.Zoom, (float)fb_width / (float)fb_height, cameraNearPlane, cameraFarPlane);
		const glm::mat4 view = camera.GetViewMatrix();

		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);
		lightShader.setFloat("material.shininess", 32.0f);
		lightShader.setVec3("lightDir", lightDir);
		// view/projection transformations
		lightShader.setVec3("viewPos", camera.Position);
		lightShader.setFloat("time", TIME);
		lightShader.setFloat("height_scale", height_scale);
		lightShader.setBool("shadows", shadows);
		lightShader.setFloat("far", cameraFarPlane);

		lightShader.setInt("cascadeCount", (int)shadowCascadeLevels.size());
		for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
		{
			lightShader.setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
		}

		setupLights(lightShader);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);
		Escena.render(lightShader);

		// draw skybox
		glDepthFunc(GL_EQUAL);
		skyboxShader.use();
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setMat4("view", view);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextures);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		// anti-aliasing
		/*glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
		glBlitFramebuffer(0, 0, fb_width, fb_height, 0, 0, fb_width, fb_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

		sceneBuffer.blitFrameBuffers((float)fb_width, (float)fb_height);

		sceneBuffer.UnBind();

		// draw screen quad texture
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		postprocesingBuffer.Bind();
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		screenShader.setBool("gammaCorrection", gammaCorrection);
		screenShader.setFloat("gamma", gammaValue);
		glBindVertexArray(quadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneBuffer.getFrameTexture());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		postprocesingBuffer.UnBind();
		//ImGui::SetNextWindowSize(ImVec2(fb_width, fb_height));

		// rendering viewport with SRGB enabled so it does not look dark
		glDisable(GL_FRAMEBUFFER_SRGB);

		ImGui::Begin("Scene window"); {

			ImGui::BeginChild("Game Render");

			float width = ImGui::GetContentRegionAvail().x;
			float height = ImGui::GetContentRegionAvail().y;

			ImVec2 availableSize = ImGui::GetWindowSize();

			// fb_width = (int)width;
			// fb_height = (int)height;

			if (width != fb_width || height != fb_height || times > 0) {
				sceneBuffer.rescaleFrameBuffer((float)fb_width, (float)fb_height);
				postprocesingBuffer.rescaleFrameBuffer((float)fb_width, (float)fb_height);
				fb_width = (int)width;
				fb_height = (int)height;
				times--;
			}

			ImGui::Image(
				(ImTextureID)postprocesingBuffer.getFrameTexture(),
				availableSize,
				ImVec2(0, 1),
				ImVec2(1, 0)
			);
		}
		ImGui::EndChild();
		ImGui::End();

		ImGui::Begin("ImGui window");
		ImGui::Text("Shadow settings: ");
		ImGui::Checkbox("Enable shadows", &shadows);
		float lightdir[3] = { lightDir.x, lightDir.y, lightDir.z };
		//ImGui::InputFloat3("Light direction", lightdir);
		ImGui::SliderFloat3("Light direction", lightdir, -1.0f, 1.0f);
		lightDir = glm::vec3(lightdir[0], lightdir[1], lightdir[2]);
		ImGui::Text("Gamma settings: ");
		ImGui::Checkbox("Gamma correction", &gammaCorrection);
		ImGui::SliderFloat("Gamma Value", &gammaValue, 0.0f, 2.2f);
		ImGui::Text("Parallax mapping: ");
		ImGui::SliderFloat("height scale", &height_scale, 0.0f, 1.0f);
		if (ImGui::Button("close program")) {
			glfwSetWindowShouldClose(window, true);
		}
		ImGui::Text("%d FPS", displayedFrames);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glEnable(GL_FRAMEBUFFER_SRGB);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteProgram(lightShader.ID);
	//glDeleteProgram(lightCubeShader.ID);
	//glDeleteProgram(shaderSingleColor.ID);
	glDeleteProgram(screenShader.ID);
	glDeleteProgram(simpleDepthShader.ID);

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteBuffers(1, &skyboxVAO);
	// glDeleteRenderbuffers(1, &rbo);
	// glDeleteFramebuffers(1, &framebuffer);

	glfwTerminate();

	return 0;
}

void proccesInput(GLFWwindow* window) {
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

	if (moveCamera) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE) return; //only handle press events
	if (key == GLFW_KEY_ESCAPE) {
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			moveCamera = false;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			moveCamera = true;
		}
	}
	if (key == GLFW_KEY_X) {
		glfwSetWindowShouldClose(window, true);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0) {
		return;
	}
	fb_width = width;
	fb_height = height;

	glViewport(0, 0, width, height);
}

void setupLights(Shader lightShader) {
	// directional light
	lightShader.setVec3("dirLight.direction", lightDir);
	lightShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	lightShader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
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
	//lightShader.setVec3("spotLight.position", camera.Position);
	//lightShader.setVec3("spotLight.direction", camera.Front);
	//lightShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	//lightShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	//lightShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	//lightShader.setFloat("spotLight.constant", 1.0f);
	//lightShader.setFloat("spotLight.linear", 0.09f);
	//lightShader.setFloat("spotLight.quadratic", 0.032f);
	//lightShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	//lightShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
	const auto inv = glm::inverse(proj * view);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt =
					inv * glm::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane) {
	const auto proj = glm::perspective(
		camera.Zoom, (float)fb_width / (float)fb_height, nearPlane,
		farPlane);
	const auto corners = getFrustumCornersWorldSpace(proj, camera.GetViewMatrix());

	glm::vec3 center = glm::vec3(-5.0f, 0, -5.0f);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	// Tune this parameter according to the scene
	constexpr float zMult = 10.0f;
	if (minZ < 0)
	{
		minZ *= zMult;
	}
	else
	{
		minZ /= zMult;
	}
	if (maxZ < 0)
	{
		maxZ /= zMult;
	}
	else
	{
		maxZ *= zMult;
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

std::vector<glm::mat4> getLightSpaceMatrices()
{
	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i]));
		}
		else if (i < shadowCascadeLevels.size())
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
		}
	}
	return ret;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
	stbi_set_flip_vertically_on_load(false);
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	stbi_set_flip_vertically_on_load(true);
	
	return textureID;
}