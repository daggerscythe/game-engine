#include "InputSystem.h"
#include <GLFW/glfw3.h>

#include <iostream>

void InputSystem::Init(GLFWwindow* window) {
	m_window = window;
}

void InputSystem::Update(EntityManager& entityManager, float deltaTime) {
	
	// get the player entites (only entities with inputcomponent)
	auto entities = entityManager.GetEntitiesWith<InputComponent>();

	for (EntityID entity : entities) {
		InputComponent& input = entityManager.GetComponent<InputComponent>(entity);

		// write current keyboard state
		// WASD - MOVEMENT
		input.moveForward = glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS;
		input.moveBackward = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;
		input.moveLeft = glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS;
		input.moveRight = glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS;

		// SPACE - JUMP
		if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			std::cout << "DEBUG: Space pressed" << std::endl;
			input.jumpRequested = true;
		}
		//input.jumpRequested = glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS;

		// R - SPAWNPOINT RESET
		if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
			auto spawnpoints = entityManager.GetEntitiesWith<SpawnpointComponent>();
			for (EntityID spawnpoint : spawnpoints) {
				SpawnpointComponent& sp = entityManager.GetComponent<SpawnpointComponent>(spawnpoint);
				sp.reset = true;
			}
		}

		// esc to close
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(m_window, true);
	}
}

void InputSystem::Flush(EntityManager& entityManager) {
	auto entities = entityManager.GetEntitiesWith<InputComponent>();
	for (EntityID entity : entities) {
		InputComponent& input = entityManager.GetComponent<InputComponent>(entity);
		input.mouseDeltaX = 0.0f;
		input.mouseDeltaY = 0.0f;
		input.scrollDelta = 0.0f;
	}
}

void InputSystem::ProcessMouseMovement(EntityManager& entityManager, float xpos, float ypos) {
	if (m_firstMouse) {
		int width, height;
		glfwGetWindowSize(m_window, &width, &height);
		m_lastX = width / 2.0f;
		m_lastY = height / 2.0f;
		m_firstMouse = false;
	}

	float xoffset = xpos - m_lastX;
	float yoffset = m_lastY - ypos; // reversed because y goes bottom to top in opengl

	m_lastX = xpos;
	m_lastY = ypos;

	// get all player entities
	auto entities = entityManager.GetEntitiesWith<InputComponent>();
	for (EntityID entity : entities) {
		InputComponent& input = entityManager.GetComponent<InputComponent>(entity);
		input.mouseDeltaX = xoffset;
		input.mouseDeltaY = yoffset;
	}
}

void InputSystem::ProcessScroll(EntityManager& entityManager, float yoffset) {
	// get all player entities
	auto entities = entityManager.GetEntitiesWith<InputComponent>();
	for (EntityID entity : entities) {
		InputComponent& input = entityManager.GetComponent<InputComponent>(entity);
		input.scrollDelta = yoffset;
	}
}
