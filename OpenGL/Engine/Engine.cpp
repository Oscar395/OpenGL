#include "Engine.h"

namespace ForByte {
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

		/*if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to load openGL function pointers" << std::endl;
			glfwTerminate();
			return;
		}*/
	}

	Engine::~Engine() {
		glfwTerminate();
	}

	void Engine::Initialize() {

	}

	void Engine::Update() {

	}

	void Engine::Quit() {
		run = false;
	}
}