#pragma once

#include "ECS/Components/Components.h"
#include "ECS/Base/BaseSystem.h"
#include "Objects/camera.h"
#include "Core/ForByte.h"
#include "Resource/Resource.h"
#include "ECS/Base/Types.h"

class CameraSystem : public ECS::BaseSystem {
public:
	CameraSystem() {
		AddComponentSignature<Transform>();

		shader = Resource.ShaderProgram("LIGHT");
		camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	void Update() override {
		camera.ProcessMouseMovement(ForByte::Core.Cxoffset, ForByte::Core.Cyoffset);
		proccessCameraInput(ForByte::Core.window);

		shader.use();
		const glm::mat4 projection = glm::perspective(camera.Zoom,
			ForByte::Core.VideoWidth() / ForByte::Core.VideoHeight(), cameraNearPlane, cameraFarPlane);
		const glm::mat4 view = camera.GetViewMatrix();

		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setFloat("material.shininess", 32.0f);
		shader.setVec3("lightDir", 0.2f, 0.7f, 0.4f);
		shader.setVec3("dirLight.direction", 0.2f, 0.7f, 0.4f);
		shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		shader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
		shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// view/projection transformations
		shader.setVec3("viewPos", camera.Position);
	}

	void proccessCameraInput(GLFWwindow* window) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, ForByte::Timer.DeltaTime());
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, ForByte::Timer.DeltaTime());
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, ForByte::Timer.DeltaTime());
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, ForByte::Timer.DeltaTime());
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
			ForByte::Core.Quit();
	}

private:
	Shader shader;
	Camera camera;
	float cameraNearPlane, cameraFarPlane;
};