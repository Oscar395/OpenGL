#include "Events.h"
#include "Engine/Engine.h"

namespace ForByte {

	void WindowCloseCallback(GLFWwindow* window);

	Events::Events() { }

	Events::~Events() { }

	void Events::Poll() {
		glfwPollEvents();
	}

	void Events::Initialize() {
		GLFWwindow& window = Core.Window();
		glfwSetWindowCloseCallback(&window, WindowCloseCallback);
	}

	// callback function implementation
	void WindowCloseCallback(GLFWwindow* window) {
		ForByte::Core.Quit();
	}
}