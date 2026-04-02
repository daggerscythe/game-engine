#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>


struct TransformComponent { // default to origin
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
};

struct RigidBodyComponent {
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);
	float mass = 1.0f;
	float restitution = 0.0f; // bounciness (0-1)
	bool isStatic = false;
	int groundContactCount = 0; // how many points of the body are in contact w the ground
};

struct ColliderComponent {
	enum class Shape { Sphere, AABB } shape = Shape::AABB;
	glm::vec3 size = glm::vec3(1.0f); // for AABB - half-extents, for Sphere - x is radius
	glm::vec3 offset = glm::vec3(0.0f); // offset from transform pos
};

struct SpawnpointComponent {
	glm::vec3 spawnpoint = glm::vec3(0.0f, 0.0f, -3.0f);
	bool reset = false;
};

struct RenderComponent {
	uint32_t meshID = 0;
	uint32_t textureID = 0;
	uint32_t shaderID = 0;
	bool isVisible = true;
};

struct LightComponent {
	enum class Type { Directional, Point, Spot } type = Type::Point;
	
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 direction = glm::vec3(0.0f); // for directional and spot

	// attentuation (point & spot)
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	// spotlight cone (in degrees)
	float innerCutOff = 12.5f; 
	float outerCutOff = 15.0f;

	bool isOn = true;
};

struct CameraComponent {
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float yaw = -90.0f;
	float pitch = 0.0f;
	float speed = 2.5f;
	float sensitivity = 0.1f;
	float zoom = 45.0f; // FOV
};

struct AudioSourceComponent {
    enum class PlayState { Idle = 0, RequestPlay = 1, Playing = 2 } playState = PlayState::Idle;
	float volume = 1.0f;
	float range = 25.0f;
	bool isLooping = false;
	bool spatial = true;
};

struct InputComponent {
	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
	bool jumpRequested = false;

	float mouseDeltaX = 0.0f;
	float mouseDeltaY = 0.0f;
	float scrollDelta = 0.0f;
};

struct ButtonComponent {
	bool isPressed = false;
	float cooldownTimer = 0.0f;
	float cooldownDuration = 0.5f;
	void (*onPress) () = nullptr;
};

struct TagComponent {
	std::string tag = "";
};