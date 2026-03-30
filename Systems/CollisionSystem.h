#pragma once
#include <glm/glm.hpp>
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"
#include <unordered_set>
#include <cstdint>

struct CollisionInfo {
	bool colliding = false; // did collision happen
	glm::vec3 normal = glm::vec3(0.0f); // what direction to push in
	float penetration = 0.0f; // how deep the overlap is
};


class CollisionSystem {
public:
	void Update(EntityManager& entityManager);

private:
	// detection
	CollisionInfo m_testAABBvsAABB(
		const TransformComponent& tcA, const ColliderComponent& ccA,
		const TransformComponent& tcB, const ColliderComponent& ccB);
	CollisionInfo m_testSphereVsSphere(
		const TransformComponent& tcA, const ColliderComponent& ccA,
		const TransformComponent& tcB, const ColliderComponent& ccB);
	CollisionInfo m_testSphereVsAABB(
		const TransformComponent& tcSphere, const ColliderComponent& ccSphere,
		const TransformComponent& tcAABB, const ColliderComponent& ccAABB);

	// resulotion
	void m_resolveCollision(
		RigidBodyComponent& rbcA, TransformComponent& tcA,
		RigidBodyComponent& rbcB, TransformComponent& tcB,
		CollisionInfo info);

	// to detect collision-enter events
	std::unordered_set<uint64_t> m_prevCollisions;
};