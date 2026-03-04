#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum Camera_Movement {
	FORWARD, 
	BACKWARD, 
	LEFT, 
	RIGHT
};

class Camera
{
public:
	// constants
	static constexpr float YAW = -90.0f;
	static constexpr float PITCH = 0.0f;
	static constexpr float SPEED = 2.5f;
	static constexpr float SENSITIVITY = 1.0f;
	static constexpr float ZOOM = 45.0f;

	// attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	// euler angles
	float yaw;
	float pitch;

	// camera settings
	float speed;
	float sensitivity;
	float zoom;

	// with vectors
	Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
		float yaw = YAW,
		float pitch = PITCH
	);
	// with scalars
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
	
	glm::mat4 getViewMatrix();
	void processKeyboard(Camera_Movement direction, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset, GLboolean constraintPitch = true);
	void processMouseScroll(float yoffset);
	
private:
	void m_updateCameraVectors();
};

