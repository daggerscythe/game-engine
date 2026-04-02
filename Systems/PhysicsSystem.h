#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"

class PhysicsSystem {
public:
	void Update(EntityManager& entityManager, float deltaTime);

private:
	const glm::vec3 GRAVITY = glm::vec3(0.0f, -9.8f, 0.0f);
	const float JUMP_VELOCITY = 3.0f;
};