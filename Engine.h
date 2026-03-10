#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ECS/EntityManager.h"
#include "ECS/Components.h"
#include "Systems/RenderSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/PhysicsSystem.h"
#include "Systems/CollisionSystem.h"
#include "Systems/AudioSystem.h"

class Engine
{
public:
	Engine();
	~Engine();

	void Run();

private:
	// core
	GLFWwindow* m_window;
	EntityManager m_entityManager;

	// systems
	RenderSystem m_renderSystem;
	InputSystem m_inputSystem;
	PhysicsSystem m_physicsSystem;
	CollisionSystem m_collisionSystem;
	AudioSystem m_audioSystem;

	// timing
	float m_deltaTime = 0.0f; // last frame exec time
	float m_lastFrame = 0.0f; // time of last frame

	// methods
	bool m_initWindow();
	void m_initScene();
	void m_update(float deltaTime);
	void m_render();
	void m_processEvents();

	// GLFW callbacks
	static void m_framebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void m_mouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void m_scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

