#pragma once
#include "ECS/Base/BaseSystem.h"
#include "ECS/Components/Components.h"
#include "Resource/Resource.h"

class MeshRendererSystem : public ECS::BaseSystem {
public:
	MeshRendererSystem() {
		AddComponentSignature<Transform>();
		AddComponentSignature<MeshRenderer>();

		shader.use();
		shader = Resource.ShaderProgram("LIGHT");
	}

	void Update() override {
		for (auto& entity : entities) {
			auto& transform = ECS::Manager.GetComponent<Transform>(entity);
			auto& mesh = ECS::Manager.GetComponent<MeshRenderer>(entity);

			shader.setMat4("model", transform.Model());

			mesh.Renderer->Draw(shader);
		}
	}

private:
	Shader shader;
};