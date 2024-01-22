#pragma once

#include "Types.h"
#include "CompList.h"
#include "BaseSystem.h"
#include "BaseComponent.h"
#include <queue>
#include <map>
#include <iostream>

namespace ECS {
	class EntityManager {
	public:
		EntityManager() : entityCount(0) {
			for (EntityID entity = 0u; entity < MAX_ENTITY_COUNT; entity++)
				availableEntities.push(entity);
		}

		~EntityManager() {

		}

		void Update() {
			for (auto& system : registeredSystems) {
				system.second->Update();
			}
		}

		void Render() {
			for (auto& system : registeredSystems) {
				system.second->Render();
			}
		}

		const EntityID AddNewEntity() {
			const EntityID entityId = availableEntities.front();
			availableEntities.pop();
			entityCount++;
			return entityId;
		}

		void DestroyEntity(const EntityID entity) {
			assert(entity < MAX_ENTITY_COUNT && "EntityID out of range!");
			entitiesSignatures.erase(entity);

			for (auto& array : componentsArrays) {
				array.second->Erase(entity);
			}

			for (auto& system : registeredSystems) {
				system.second->RemoveEntity(entity);
			}

			entityCount--;
			availableEntities.push(entity);
		}

		template<typename T, typename... Args>
		void AddComponent(const EntityID entity, Args&&... args) {
			assert(entity < MAX_ENTITY_COUNT && "Entity out of range");
			assert(entitiesSignatures[entity].size() < MAX_COMP_COUNT && "Component count limit reached!");

			T component(std::forward<Args>(args)...);
			component.entityID = entity;
			//GetCompList<T>()->Insert(component);

			const ComponentTypeID compType = CompType<T>();
			entitiesSignatures.at(entity).insert(compType);
			//AttachEntityToSystem(entity);
		}

		template<typename T>
		void RemoveComponent(const EntityID entity) {
			assert(entity < MAX_ENTITY_COUNT && "EntityID out of range!");
			const ComponentTypeID compType = CompType<T>();
			entitiesSignatures.at(entity).erase(compType);
			//GetCompList<T>()->Erase(entity);
			//AttachEntityToSystem(entity);
		}

		/*template<typename T>
		T& GetComponent(const EntityID entity) {
			assert(entity < MAX_ENTITY_COUNT && "EntityID out of range!");
			const ComponentTypeID compType = CompType<T>();
			return GetCompList<T>()->Get(entity);
		}*/

		template<typename T>
		const bool HasComponent(const EntityID entity) {
			assert(entity < MAX_ENTITY_COUNT && "EntityID out of range!");
			const EntitySignature signature = entitiesSignatures.at(entity);
			const ComponentTypeID compType = CompType<T>();
			//auto it = std::find(signature.begin(), signature.end(), compType);
			return (signature.count(compType) > 0);
		}

	private:
		EntityID entityCount;
		std::queue<EntityID> availableEntities;
		std::map<EntityID, EntitySignature> entitiesSignatures;
		std::map<SystemTypeID, std::unique_ptr<BaseSystem>> registeredSystems;
		std::map<ComponentTypeID, std::shared_ptr<ICompList>> componentsArrays;
	};
}