#pragma once
#include "../ECS/EntityManager.h"

class PhysicsSystem
{
public:
	void Update(EntityManager& entityManager, float deltaTime);
};

