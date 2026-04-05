#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"

class InputSystem {
public:
	void Init(GLFWwindow* window);
	void Update(EntityManager& entityManager, float deltaTime);
	void Flush(EntityManager& entityManager);
	void ProcessMouseMovement(EntityManager& entityManager, float xpos, float ypos);
	void ProcessScroll(EntityManager& entityManager, float yoffset);
	
private:
	GLFWwindow* m_window = nullptr;
	bool m_firstMouse = true;
	float m_lastX = 0.0f;
	float m_lastY = 0.0f;
	bool m_fWasPressed = false; // for flashlight toggle
};

