#include "Core/ForByte.h"

#include "ECS/Base/Types.h"
#include "ECS/Base/EntityManager.h"

class TestComp1 : public ECS::BaseComponent {
public:
	TestComp1() = default;

	TestComp1(int a) : A(a) {

	}
	int A;
};

int main(int argc, char** argv) {

	ECS::EntityManager mgr;
	auto id = mgr.AddNewEntity();
	mgr.AddComponent<TestComp1>(id, 5);
	mgr.RemoveComponent<TestComp1>(id);

	auto TypeID1 = ECS::CompType<TestComp1>();

	std::cout << id << " " << TypeID1 << std::endl;

	ForByte::Core.Initialize();
	ForByte::Timer.Initialize();
	ForByte::Event.Initialize();

	while (ForByte::Core.Run())
	{
		ForByte::Timer.Tick();
		ForByte::Event.Poll();
		ForByte::Core.Update();
	}

	return EXIT_SUCCESS;
}