#pragma once

#include "ECS/Base/BaseComponent.h"
#include "Objects/mesh.h"

struct MeshRenderer : public ECS::BaseComponent
{
	~MeshRenderer() = default;

	MeshRenderer() : Name("CUBE") {
		
	}

private:
	std::string Name;
	std::shared_ptr<Mesh> Renderer;
};