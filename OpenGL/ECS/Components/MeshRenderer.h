#pragma once

#include "ECS/Base/BaseComponent.h"
#include "Resource/Resource.h"

struct MeshRenderer : public ECS::BaseComponent
{
	~MeshRenderer() = default;

	MeshRenderer() : Name("CUBE") {
		Renderer = Resource.GetMesh(Name);
	}

	std::shared_ptr<Mesh> Renderer;
private:
	std::string Name;
};