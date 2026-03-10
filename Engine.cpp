#include "Engine.h"
#include "ECS/Components.h"
#include "stb_image.h"
#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Engine::Engine() : m_window(nullptr) {
    if (!m_initWindow()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return;
    }
    m_initScene();
}

Engine::~Engine() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Engine::Run() {
	while (!glfwWindowShouldClose(m_window)) {
		// pre-frame logic
		float currentFrame = static_cast<float>(glfwGetTime());
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		
		glfwPollEvents(); // check and call events
		m_update(m_deltaTime); // update all logic systems
		m_render(); // render
		m_inputSystem.Flush(m_entityManager); // clear input
		glfwSwapBuffers(m_window); // swap buffers
	}
}

bool Engine::m_initWindow() {
	glfwInit();

	// configuring GLFW for version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creating a window object
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Dani's Game Engine Demo", NULL, NULL);
	if (m_window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(m_window);
	// tell GLFW we want to call resize function by registering
	glfwSetFramebufferSizeCallback(m_window, m_framebufferSizeCallback);
	// call the function each time mouse moves
	glfwSetCursorPosCallback(m_window, m_mouseCallback);
	// call the function each time mouse scrolls
	glfwSetScrollCallback(m_window, m_scrollCallback);
	// tell GLFW we want to capture the mouse
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// store pointer to this Engine instance so static callbacks can access it
	glfwSetWindowUserPointer(m_window, this);

	// initialize GLAD before calling any OpenGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);

	// flip loaded textures
	stbi_set_flip_vertically_on_load(true);

	// pass in reference to the input system
	m_inputSystem.Init(m_window);
	return true;
}

void Engine::m_initScene() {
	// initialize render system
	m_renderSystem.Init(SCR_WIDTH, SCR_HEIGHT);
	m_renderSystem.LoadShader(1, "ModelVertexShader.glsl", "ModelFragmentShader.glsl");
	m_renderSystem.LoadModel(1, "models/backpack/backpack.obj");

	// create player entity
	EntityID player = m_entityManager.CreateEntity();
	m_entityManager.AddComponent<TransformComponent>(player, TransformComponent{});
	m_entityManager.AddComponent<CameraComponent>(player, CameraComponent{});
	m_entityManager.AddComponent<InputComponent>(player, InputComponent{});
	m_entityManager.AddComponent<TagComponent>(player, TagComponent{ "Player" });

	// sun entity - directional light
	EntityID sun = m_entityManager.CreateEntity();
	m_entityManager.AddComponent<TransformComponent>(sun, TransformComponent{});
	LightComponent sunComp{};
	sunComp.type = LightComponent::Type::Directional;
	sunComp.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	sunComp.ambient = glm::vec3(0.3f);
	sunComp.diffuse = glm::vec3(0.8f);
	sunComp.specular = glm::vec3(0.5f);
	m_entityManager.AddComponent<LightComponent>(sun, sunComp);
	m_entityManager.AddComponent<TagComponent>(sun, TagComponent{ "Sun" });

	// flashlight entity - attached to player
	EntityID flashlight = m_entityManager.CreateEntity();
	m_entityManager.AddComponent<TransformComponent>(flashlight, TransformComponent{});
	LightComponent flashlightComp{};
	flashlightComp.type = LightComponent::Type::Spot;
	flashlightComp.innerCutOff = 12.5f;
	flashlightComp.outerCutOff = 15.0f;
	flashlightComp.ambient = glm::vec3(0.0f);
	flashlightComp.diffuse = glm::vec3(1.0f);
	flashlightComp.specular = glm::vec3(1.0f);
	m_entityManager.AddComponent<LightComponent>(flashlight, flashlightComp);
	m_entityManager.AddComponent<TagComponent>(flashlight, TagComponent{ "Flashlight" });

	// backpack entity
	EntityID backpack = m_entityManager.CreateEntity();
	TransformComponent backpackTransform{};
	backpackTransform.position = glm::vec3(0.0f, 0.0f, -3.0f);
	RenderComponent backpackRender{};
	backpackRender.meshID = 1;
	backpackRender.shaderID = 1;
	backpackRender.isVisible = true;
	m_entityManager.AddComponent<TransformComponent>(backpack, backpackTransform);
	m_entityManager.AddComponent<RenderComponent>(backpack, backpackRender);
}

void Engine::m_update(float deltaTime) {
	m_inputSystem.Update(m_entityManager, deltaTime);
	m_physicsSystem.Update(m_entityManager, deltaTime);
	m_collisionSystem.Update(m_entityManager);
	m_audioSystem.Update(m_entityManager);
}

void Engine::m_render() {
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // state-setting function
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state-using function
	m_renderSystem.Update(m_entityManager, m_deltaTime);
}

void Engine::m_processEvents()
{
}

void Engine::m_framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// setting viewport dimensions
	glViewport(0, 0, width, height);
}

void Engine::m_mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if (engine) {
		engine->m_inputSystem.ProcessMouseMovement(engine->m_entityManager,
			static_cast<float>(xpos), static_cast<float>(ypos));
	}
	// prevent sudden snap to center on first call
	//if (firstMouse)
	//{
	//	lastX = (float)xpos;
	//	lastY = (float)ypos;
	//	firstMouse = false;
	//}

	//float xoffset = static_cast<float>(xpos - lastX);
	//float yoffset = static_cast<float>(lastY - ypos); // reversed since y-coords are top to bottom
	//lastX = (float)xpos;
	//lastY = (float)ypos;

	//const float sensitivity = 0.1f;
	//xoffset *= sensitivity;
	//yoffset *= sensitivity;

	//camera.processMouseMovement(xoffset, yoffset);
}

void Engine::m_scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
	if (engine) {
		engine->m_inputSystem.ProcessScroll(engine->m_entityManager,
			static_cast<float>(yoffset));
	}
}
