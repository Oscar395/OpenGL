#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Objects/FrameBuffer.h"

namespace ForByte {

	const int SCREEN_WIDTH(1280);
	const int SCREEN_HEIGHT(720);

	class Engine {
	public:
		GLFWwindow* window;

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
		void setVideoWidth(float width);
		void setVideoHeight(float height);

		float Cxoffset, Cyoffset;

		void calcCameraMovement(double xposIn, double yposIn);

	private:
		Engine();

	private:
		bool run;
		float videoWidth, videoHeight;
		FrameBuffer sceneBuffer;
		// camera positions precalculated
		float lastX = 400, lastY = 300;
		bool firstMouse = true;
	};
	static Engine& Core = Engine::ref();
}