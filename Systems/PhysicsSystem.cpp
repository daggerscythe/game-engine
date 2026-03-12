#include "PhysicsSystem.h"

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
		if (input.resetBall) {
			tc.position = glm::vec3(0.0f, 5.0f, -3.0f);
			rbc.velocity = glm::vec3(0.0f);
			rbc.acceleration = glm::vec3(0.0f);
		}
		tc.position += rbc.velocity * deltaTime;
	}
}
