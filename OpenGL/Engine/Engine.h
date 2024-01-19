#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

namespace CrystalByte {

	const int SCREEN_WIDTH(1280);
	const int SCREEN_HEIGHT(720);

	class Engine {
	public:
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		static Engine& ref() {
			static Engine reference;
			return reference;
		}

		void Quit();
		void Update();
		void Initialize();

		inline const bool Run() const { return run; }
		inline GLFWwindow& Window() { return *window; }
		inline const float VideoWidth() const { return videoWidth; }
		inline const float VideoHeight() const { return videoHeight; }

	private:
		Engine();

	private:
		bool run;
		GLFWwindow* window;
		float videoWidth, videoHeight;
	};
	static Engine& Core = Engine::ref();
}