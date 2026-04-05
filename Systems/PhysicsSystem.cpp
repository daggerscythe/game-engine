#include "PhysicsSystem.h"
#include <iostream>

void PhysicsSystem::Update(EntityManager& entityManager, float deltaTime) {

	auto entities = entityManager.GetEntitiesWith<RigidBodyComponent, TransformComponent>();

	for (EntityID entity : entities) {
		RigidBodyComponent& rbc = entityManager.GetComponent<RigidBodyComponent>(entity);
		TransformComponent& tc = entityManager.GetComponent<TransformComponent>(entity);

		// skip static objects
		if (rbc.isStatic) continue;

        // check for jumps
		if (entityManager.HasComponent<InputComponent>(entity)) {
			InputComponent& input = entityManager.GetComponent<InputComponent>(entity);
			if (rbc.groundContactCount > 0 && input.jumpRequested) {
				// DEBUG
				std::cout << "DEBUG: Performing jump..." << std::endl;
				rbc.velocity.y = JUMP_VELOCITY;
				input.jumpRequested = false;
				rbc.groundContactCount = 0;
			}
		}

		// apply gravity
		rbc.acceleration = GRAVITY;

		// integrate velocity
		rbc.velocity += rbc.acceleration * deltaTime;

		// integrate position
		tc.position += rbc.velocity * deltaTime;

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
			// if this entity has a camera attached, sync the camera position as well
			if (entityManager.HasComponent<CameraComponent>(entity)) {
				auto &cam = entityManager.GetComponent<CameraComponent>(entity);
				cam.position = spawnpoint;
			}
			rbc.velocity = glm::vec3(0.0f);
			rbc.acceleration = glm::vec3(0.0f);
			entityManager.GetComponent<SpawnpointComponent>(entity).reset = false;
			// DEBUG
			std::cout << "Respawned object " << entity << std::endl;
		}
	}
}
