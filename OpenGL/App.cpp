#include "Core/ForByte.h"

#include "ECS/Base/Types.h"
#include "ECS/Base/Entity.h"
#include "ECS/Systems/MeshRendererSystem.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Components/Components.h"
#include <glad/glad.h>

int main(int argc, char** argv) {

	ECS::Manager.RegisterSystem<CameraSystem>();
	auto cameraEntity = ECS::Manager.AddNewEntity();
	ECS::Entity camEntity(cameraEntity, &ECS::Manager);
	camEntity.AddComponent<Transform>();

	std::cout << camEntity.GetID() << std::endl;

	/*ECS::Manager.RegisterSystem<MeshRendererSystem>();

	auto entityId = ECS::Manager.AddNewEntity();
	ECS::Manager.AddComponent<Transform>(entityId);
	ECS::Manager.AddComponent<MeshRenderer>(entityId);*/
	//ECS::Entity entity(entityId, &ECS::Manager);
	//entity.AddComponent<Transform>();
	//entity.AddComponent<MeshRenderer>();

	ECS::Manager.Update();

	ForByte::Core.Initialize();
	ForByte::Timer.Initialize();
	ForByte::Event.Initialize();

	while (ForByte::Core.Run())
	{
		ForByte::Timer.Tick();
		ForByte::Core.Update();
		// ECS::Manager.Update();
		ForByte::Event.Poll();
	}

	return EXIT_SUCCESS;
}