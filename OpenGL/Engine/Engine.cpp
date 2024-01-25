#include "Engine.h"

namespace ForByte {
	void frame_buffer_callback(GLFWwindow* window, int width, int height);
	void mouse_pos_callback(GLFWwindow* window, double xposIn, double yposIn);

	Engine::Engine() : run(true), window(NULL), videoWidth(SCREEN_WIDTH), videoHeight(SCREEN_HEIGHT) {
		if (!glfwInit()) {
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		auto& monitor = *glfwGetVideoMode(glfwGetPrimaryMonitor());

		glfwWindowHint(GLFW_RED_BITS, monitor.redBits);
		glfwWindowHint(GLFW_BLUE_BITS, monitor.blueBits);
		glfwWindowHint(GLFW_GREEN_BITS, monitor.greenBits);
		glfwWindowHint(GLFW_REFRESH_RATE, monitor.refreshRate);

		window = glfwCreateWindow(videoWidth, videoHeight, "ForByte", NULL, NULL);

		if (!window) {
			std::cout << "Failed to create the window" << std::endl;
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, frame_buffer_callback);

		glfwSetCursorPosCallback(window, mouse_pos_callback);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to load openGL function pointers" << std::endl;
			glfwTerminate();
			return;
		}
	}

	Engine::~Engine() {
		glfwTerminate();
	}

	void Engine::Initialize() {
		// set open gl state
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		//glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_FRAMEBUFFER_SRGB);

		sceneBuffer = FrameBuffer(videoWidth, videoHeight, true);
	}

	void Engine::Update() {
		glfwSwapBuffers(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Engine::Quit() {
		run = false;
	}

	void Engine::setVideoWidth(float width) {
		videoWidth = width;
	}

	void Engine::setVideoHeight(float height) {
		videoHeight = height;
	}

	void Engine::calcCameraMovement(double xposIn, double yposIn) {
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

		Cxoffset = xoffset;
		Cyoffset = yoffset;
	}

	void frame_buffer_callback(GLFWwindow* window, int width, int height) {
		Core.setVideoWidth(width);
		Core.setVideoHeight(height);

		glViewport(0, 0, width, height);
	}

	void mouse_pos_callback(GLFWwindow* window, double xposIn, double yposIn) {
		Core.calcCameraMovement(xposIn, yposIn);
		// camera.ProcessMouseMovement(xoffset, yoffset);
	}
}