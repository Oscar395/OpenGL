#pragma once

#include "ECS/Base/BaseComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform : public ECS::BaseComponent {
	glm::vec3 Scale = glm::vec3(1.0f);
	glm::vec3 Rotation = glm::vec3(0.0f);
	glm::vec3 Position = glm::vec3(0.0f);

	Transform(const glm::vec3& translate, const glm::vec3& scale, const glm::vec3& rotation) :
		Position(translate), Scale(scale), Rotation(rotation) { }

	Transform(const glm::vec3& translate, const glm::vec3& scale) :
		Position(translate), Scale(scale) { }

	Transform(const glm::vec3& translate) : 
		Position(translate) { }

	glm::mat4 Model() {
		glm::mat4 matrix = glm::translate(glm::mat4(1.0f), Position);
		matrix = glm::rotate(matrix, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
		matrix = glm::rotate(matrix, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
		matrix = glm::rotate(matrix, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
		matrix = glm::scale(matrix, Scale);
		return matrix;
	}

	Transform() = default;
	~Transform() = default;
};