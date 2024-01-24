#pragma once
#include "ECS/Base/BaseSystem.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/MeshRenderer.h"
#include "ECS/Base/Types.h"

class MeshRendererSystem : public ECS::BaseSystem {
public:
	MeshRendererSystem() {
		AddComponentSignature<Transform>();
		AddComponentSignature<MeshRenderer>();
	}

	void Render() override {
		for (auto entity : entities) {
			auto& mesh = ECS::Manager.GetComponent<MeshRenderer>(entity);
			auto& transform = ECS::Manager.GetComponent<Transform>(entity);
		}

	}
};