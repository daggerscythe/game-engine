#include "CollisionSystem.h"
#include <algorithm>
#include <iostream>

void CollisionSystem::Update(EntityManager& entityManager) {
    auto entities = entityManager.GetEntitiesWith<ColliderComponent, TransformComponent>();

	std::unordered_set<uint64_t> currentCollisions;

	// check every pair of entities
	for (int i = 0; i < entities.size(); i++) {
		EntityID a = entities[i];
		ColliderComponent& ccA = entityManager.GetComponent<ColliderComponent>(a);
		TransformComponent& tcA = entityManager.GetComponent<TransformComponent>(a);
		bool aStatic = entityManager.HasComponent<RigidBodyComponent>(a) &&
					   entityManager.GetComponent<RigidBodyComponent>(a).isStatic;
		bool aHasRB = entityManager.HasComponent<RigidBodyComponent>(a);

		for (int j = i + 1; j < entities.size(); j++) {
			EntityID b = entities[j];
			ColliderComponent& ccB = entityManager.GetComponent<ColliderComponent>(b);
			TransformComponent& tcB = entityManager.GetComponent<TransformComponent>(b);
			bool bStatic = entityManager.HasComponent<RigidBodyComponent>(b) &&
						   entityManager.GetComponent<RigidBodyComponent>(b).isStatic;

			if (aStatic && bStatic) continue;

			// pick what test to do
			CollisionInfo info;
			if (ccA.shape == ColliderComponent::Shape::AABB && ccB.shape == ColliderComponent::Shape::AABB)
				info = m_testAABBvsAABB(tcA, ccA, tcB, ccB);
			else if (ccA.shape == ColliderComponent::Shape::Sphere && ccB.shape == ColliderComponent::Shape::Sphere)
				info = m_testSphereVsSphere(tcA, ccA, tcB, ccB);
			else if (ccA.shape == ColliderComponent::Shape::Sphere && ccB.shape == ColliderComponent::Shape::AABB)
				info = m_testSphereVsAABB(tcA, ccA, tcB, ccB);
			else if (ccA.shape == ColliderComponent::Shape::AABB && ccB.shape == ColliderComponent::Shape::Sphere) {
				// flip so sphere is first
				info = m_testSphereVsAABB(tcB, ccB, tcA, ccA);
				info.normal = -info.normal; // compensate for earlier flip
			}

			if (!info.colliding) continue;

			bool bHasRB = entityManager.HasComponent<RigidBodyComponent>(b);
			if (!aHasRB && !bHasRB) continue;

			// dummy static RBs for entities that don't have one
			RigidBodyComponent staticRB{};
			staticRB.isStatic = true;

			RigidBodyComponent& rbcA = aHasRB ? entityManager.GetComponent<RigidBodyComponent>(a) : staticRB;
			RigidBodyComponent& rbcB = bHasRB ? entityManager.GetComponent<RigidBodyComponent>(b) : staticRB;

			// DEBUG
			std::cout << "DEBUG: Resolving collision between " << entities[i] << " and " << entities[j] << std::endl;
			m_resolveCollision(rbcA, tcA, rbcB, tcB, info);

            // record this collision pair in current set (normalize ordering)
			uint32_t u = std::min(a, b);
			uint32_t v = std::max(a, b);
			uint64_t key = (static_cast<uint64_t>(u) << 32) | static_cast<uint64_t>(v);
			currentCollisions.insert(key);

            // play collision sounds only on collision-enter (not while persisting)
			if (m_prevCollisions.find(key) == m_prevCollisions.end()) {
				if (entityManager.HasComponent<AudioSourceComponent>(a)) {
					entityManager.GetComponent<AudioSourceComponent>(a).playState = AudioSourceComponent::PlayState::RequestPlay;
				}
				else if (entityManager.HasComponent<AudioSourceComponent>(b)) {
					entityManager.GetComponent<AudioSourceComponent>(b).playState = AudioSourceComponent::PlayState::RequestPlay;
				}
			}
		}
	}

    // move current collisions into previous for next frame. Moving allows the previous
	// container to release its previous memory allocation when currentCollisions is small,
	// which helps keep memory usage bounded.
	m_prevCollisions = std::move(currentCollisions);
}

CollisionInfo CollisionSystem::m_testAABBvsAABB(const TransformComponent& tcA, const ColliderComponent& ccA, const TransformComponent& tcB, const ColliderComponent& ccB) {
	CollisionInfo info;

	glm::vec3 minA = tcA.position + ccA.offset - ccA.size;
	glm::vec3 maxA = tcA.position + ccA.offset + ccA.size;
	glm::vec3 minB = tcB.position + ccB.offset - ccB.size;
	glm::vec3 maxB = tcB.position + ccB.offset + ccB.size;

	// no overlap if at least 1 axis isn't overlapping
	if (maxA.x < minB.x || minA.x > maxB.x) return info;
	if (maxA.y < minB.y || minA.y > maxB.y) return info;
	if (maxA.z < minB.z || minA.z > maxB.z) return info;

	// they overlap, find least penetration axis
	float overlapX = std::min(maxA.x, maxB.x) - std::max(minA.x, minB.x);
	float overlapY = std::min(maxA.y, maxB.y) - std::max(minA.y, minB.y);
	float overlapZ = std::min(maxA.z, maxB.z) - std::max(minA.z, minB.z);

	// resolve along axis of least penetration
	if (overlapX < overlapY && overlapX < overlapZ) {
		info.penetration = overlapX;
		info.normal = (tcA.position.x < tcB.position.x) ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
	}
	else if (overlapY < overlapZ) {
		info.penetration = overlapY;
		info.normal = (tcA.position.y < tcB.position.y) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
	}
	else {
		info.penetration = overlapZ;
		info.normal = (tcA.position.z < tcB.position.z) ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);
	}

	info.colliding = true;
	return info;
}

CollisionInfo CollisionSystem::m_testSphereVsSphere(const TransformComponent& tcA, const ColliderComponent& ccA, const TransformComponent& tcB, const ColliderComponent& ccB) {
	CollisionInfo info;

	glm::vec3 centerA = tcA.position + ccA.offset;
	glm::vec3 centerB = tcB.position + ccB.offset;
	float radiusA = ccA.size.x;
	float radiusB = ccB.size.x;

	glm::vec3 diff = centerA - centerB;
	float distance = glm::length(diff);
	float radiusSum = radiusA + radiusB;

	// no overlap
	if (distance >= radiusSum) return info;

	// they do overlap
	info.colliding = true;
	info.penetration = radiusSum - distance;
	info.normal = distance > 0.0001f ? glm::normalize(diff) : glm::vec3(0, 1, 0);

	return info;
}

CollisionInfo CollisionSystem::m_testSphereVsAABB(const TransformComponent& tcSphere, const ColliderComponent& ccSphere, const TransformComponent& tcAABB, const ColliderComponent& ccAABB) {
	CollisionInfo info;

	glm::vec3 center = tcSphere.position + ccSphere.offset;
	float radius = ccSphere.size.x;

	glm::vec3 aabbMin = tcAABB.position + ccAABB.offset - ccAABB.size;
	glm::vec3 aabbMax = tcAABB.position + ccAABB.offset + ccAABB.size;

	// find closest AABB point to sphere's center
	glm::vec3 closest = glm::clamp(center, aabbMin, aabbMax);

	glm::vec3 diff = center - closest;
	float distance = glm::length(diff);

	// no overlap
	if (distance >= radius) return info;

	// they do overlap
	info.colliding = true;
	info.penetration = radius - distance;
	info.normal = distance > 0.0001f ? glm::normalize(diff) : glm::vec3(0, 1, 0);

	return info;
}

void CollisionSystem::m_resolveCollision(RigidBodyComponent& rbcA, TransformComponent& tcA, RigidBodyComponent& rbcB, TransformComponent& tcB, CollisionInfo info) {
	// ensure normal points from A(floor) up toward B(ball)
	if (info.normal.y < 0 && tcB.position.y > tcA.position.y)
		info.normal = -info.normal;
	
	// inverse masses, static objects have infinite mass = 0 inverse
	float invMassA = rbcA.isStatic ? 0.0f : 1.0f / rbcA.mass;
	float invMassB = rbcB.isStatic ? 0.0f : 1.0f / rbcB.mass;
	float invMassSum = invMassA + invMassB;
	if (invMassSum == 0.0f) return; // both static

	// relative velocity along collision normal
	glm::vec3 relativeVelocity = rbcB.velocity - rbcA.velocity;
	float velocityAlongNormal = glm::dot(relativeVelocity, info.normal);

	// positional correction 
	const float correctionFactor = 0.8f;
	const float slop = 0.001f; // small jitter tolerance
	float correctionMag = std::max(info.penetration - slop, 0.0f) / invMassSum * correctionFactor;
	glm::vec3 correction = correctionMag * info.normal;
	if (!rbcA.isStatic) tcA.position -= invMassA * correction;
	if (!rbcB.isStatic) tcB.position += invMassB * correction;

	// don't resolve if objects are separating
	if (velocityAlongNormal > 0) return;

	// calculate impulse
	float restitution = std::max(rbcA.restitution, rbcB.restitution);
	//float restitution = (rbcA.restitution + rbcB.restitution) / 2;
	float j = -(1.0f + restitution) * velocityAlongNormal / invMassSum;

	// apply impulse
	glm::vec3 impulse = j * info.normal;
	if (!rbcA.isStatic) rbcA.velocity -= invMassA * impulse;
	if (!rbcB.isStatic) rbcB.velocity += invMassB * impulse;

	// sleep threshold -- stop resolving micro bounces
	const float sleepThreshold = 0.05f;
	if (!rbcB.isStatic && glm::length(rbcB.velocity) < sleepThreshold && info.penetration <= slop)
		rbcB.velocity = glm::vec3(0.0f);
	if (!rbcA.isStatic && glm::length(rbcA.velocity) < sleepThreshold && info.penetration <= slop)
		rbcA.velocity = glm::vec3(0.0f);
}
