#include "Core/ForByte.h"

#include "ECS/Base/Types.h"
#include "ECS/Base/Entity.h"


class TestComp1 : public ECS::BaseComponent {
	int A = 5;
};

class TestComp2 : public ECS::BaseComponent {
	int A = 5;
};

class TestSystem1 : public ECS::BaseSystem {
public:
	TestSystem1() {
		AddComponentSignature<TestComp1>();
	}
};

class TestSystem2 : public ECS::BaseSystem {
public:
	TestSystem2() {
		AddComponentSignature<TestComp2>();
	}
};

class TestSystem3 : public ECS::BaseSystem {
public:
	TestSystem3() {
		AddComponentSignature<TestComp1>();
		AddComponentSignature<TestComp2>();
	}
};


int main(int argc, char** argv) {

	ECS::EntityManager mgr;
	
	mgr.RegisterSystem<TestSystem1>();
	mgr.RegisterSystem<TestSystem2>();
	mgr.RegisterSystem<TestSystem3>();

	auto entity1 = mgr.AddNewEntity();
	ECS::Entity ent(entity1, &mgr);

	ent.AddComponent<TestComp1>();

	auto entity2 = mgr.AddNewEntity();
	mgr.AddComponent<TestComp2>(entity2);

	auto entity3 = mgr.AddNewEntity();
	mgr.AddComponent<TestComp1>(entity3);
	mgr.AddComponent<TestComp2>(entity3);

	mgr.Update();

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