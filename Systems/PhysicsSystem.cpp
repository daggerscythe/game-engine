#include "PhysicsSystem.h"
#include <iostream>

void PhysicsSystem::Update(EntityManager& entityManager, float deltaTime) {
	// TODO: implement SpawnPoints and remove this later
	InputComponent& input = entityManager.GetComponent<InputComponent>(entityManager.GetEntitiesWith<InputComponent>()[0]);
	
	auto entities = entityManager.GetEntitiesWith<RigidBodyComponent, TransformComponent>();

	for (EntityID entity : entities) {
		RigidBodyComponent& rbc = entityManager.GetComponent<RigidBodyComponent>(entity);
		TransformComponent& tc = entityManager.GetComponent<TransformComponent>(entity);

		// skip static objects
		if (rbc.isStatic) continue;

		// apply gravity
		rbc.acceleration = GRAVITY;

		// integrate velocity
		rbc.velocity += rbc.acceleration * deltaTime;

		// integrate position
		tc.position += rbc.velocity * deltaTime;

		// DEBUG
		std::cout << "Entity " << entity << " is at ("
			<< tc.position.x << ", "
			<< tc.position.y << ", "
			<< tc.position.z << ")" << std::endl;

		// check for spawnpoint reset
		if (entityManager.HasComponent<SpawnpointComponent>(entity) && entityManager.GetComponent<SpawnpointComponent>(entity).reset) {
			auto spawnpoint = entityManager.GetComponent<SpawnpointComponent>(entity).spawnpoint;
			// DEBUG
			std::cout << "DEBUG: Respawning entity "
				<< entity << " to ("
				<< spawnpoint.x << ", "
				<< spawnpoint.y << ", "
				<< spawnpoint.z << ")" << std::endl;
			tc.position = spawnpoint;
			rbc.velocity = glm::vec3(0.0f);
			rbc.acceleration = glm::vec3(0.0f);
			entityManager.GetComponent<SpawnpointComponent>(entity).reset = false;
			// DEBUG
			std::cout << "Respawned object " << entity << std::endl;
		}
	}
}
