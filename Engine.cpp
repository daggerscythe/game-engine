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

	// DEBUG
	std::cout << "DEBUG: Created Engine instance" << std::endl;
}

Engine::~Engine() {
	m_renderSystem.Shutdown();
	m_audioSystem.Shutdown();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Engine::Run() {
	// DEBUG
	std::cout << "DEBUG: Running Engine..." << std::endl;

	m_initScene();

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
	// DEBUG
	std::cout << "DEBUG: Initializing Window...." << std::endl;

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

	// initialize audio system
	m_audioSystem.Init();
	// DEBUG
	std::cout << "DEBUG: Initialized Audio System" << std::endl;

	// pass in reference to the input system
	m_inputSystem.Init(m_window);
	// DEBUG
	std::cout << "DEBUG: Initialized Input System" << std::endl;

	return true;
}

void Engine::m_initScene() {
	// DEBUG
	std::cout << "DEBUG: Initializing Scene..." << std::endl;

	// initialize render system
	m_renderSystem.Init(SCR_WIDTH, SCR_HEIGHT);
	// DEBUG
	std::cout << "DEBUG: Initialized Render System" << std::endl;

	// load shaders and models
	m_renderSystem.LoadShader(1, "ModelVertexShader.glsl", "ModelFragmentShader.glsl");
	m_renderSystem.LoadShader(2, "LightingVertexShader.glsl", "LightingFragmentShader.glsl");
	m_renderSystem.LoadModel(1, "models/backpack/backpack.obj");
	
	// create a box for floor
	m_renderSystem.CreateBoxModel(2);

	// add sounds
	// DEBUG
	std::cout << "DEBUG: Loading sounds..." << std::endl;
	
	uint32_t bgmSound = m_audioSystem.LoadSound("audio/a_moments_peace.wav");
	uint32_t bounceSound = m_audioSystem.LoadSound("audio/bounceCutMono.wav");

	// DEBUG
	std::cout << "DEBUG: Loading entities..." << std::endl;

	// background music
	EntityID bgm = m_entityManager.CreateEntity(); // ID = 0
    AudioSourceComponent bgmASC{};
	bgmASC.isLooping = true;
	bgmASC.playState = AudioSourceComponent::PlayState::RequestPlay;
	bgmASC.spatial = false;
	m_entityManager.AddComponent<AudioSourceComponent>(bgm, bgmASC);
	m_audioSystem.RegisterSource(bgm, bgmSound, bgmASC.spatial, bgmASC.isLooping);

	// DEBUG
	std::cout << "DEBUG: Created background music entity" << std::endl;

	// sun - directional light
	EntityID sun = m_entityManager.CreateEntity(); // ID = 1
	m_entityManager.AddComponent<TransformComponent>(sun, TransformComponent{});
	LightComponent sunComp{};
	sunComp.type = LightComponent::Type::Directional;
	sunComp.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	sunComp.ambient = glm::vec3(0.3f);
	sunComp.diffuse = glm::vec3(0.8f);
	sunComp.specular = glm::vec3(0.5f);
	m_entityManager.AddComponent<LightComponent>(sun, sunComp);
	m_entityManager.AddComponent<TagComponent>(sun, TagComponent{ "Sun" });

	// DEBUG
	std::cout << "DEBUG: Created Sun entity" << std::endl;

	// flashlight - attached to player
	EntityID flashlight = m_entityManager.CreateEntity(); // ID = 2
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

	// DEBUG
	std::cout << "DEBUG: Created Flashlight entity" << std::endl;

	// floor 
	EntityID floor = m_entityManager.CreateEntity(); // ID = 3
	TransformComponent floorTransform{};
	floorTransform.position = glm::vec3(0.0f, -2.0f, -3.0f);
	floorTransform.scale = glm::vec3(30.0f, 0.1f, 30.0f);
	m_entityManager.AddComponent<TransformComponent>(floor, floorTransform);
    RigidBodyComponent floorRB{};
	floorRB.isStatic = true;
	m_entityManager.AddComponent<RigidBodyComponent>(floor, floorRB);
	ColliderComponent floorCollider{};
	floorCollider.shape = ColliderComponent::Shape::AABB;
	floorCollider.size = floorTransform.scale * 0.5f;
	m_entityManager.AddComponent<ColliderComponent>(floor, floorCollider);
	RenderComponent floorRender{};
	floorRender.meshID = 2;
	floorRender.shaderID = 2;
	floorRender.isVisible = true;
	m_entityManager.AddComponent<RenderComponent>(floor, floorRender);

	// DEBUG
	std::cout << "DEBUG: Created Floor entity" << std::endl;

	// create player entity
	EntityID player = m_entityManager.CreateEntity(); // ID = 4
	//m_entityManager.AddComponent<TransformComponent>(player, TransformComponent{});
	TransformComponent playerTransform{};
	playerTransform.position = glm::vec3(0.0f, 5.0f, 3.0f);
	m_entityManager.AddComponent<TransformComponent>(player, playerTransform);
	//m_entityManager.AddComponent<RigidBodyComponent>(player, RigidBodyComponent{});
	RigidBodyComponent playerRB{};
	playerRB.mass = 1.0f;
	playerRB.isStatic = false;
	playerRB.restitution = 0.0f;
	m_entityManager.AddComponent<RigidBodyComponent>(player, playerRB);
	//m_entityManager.AddComponent<ColliderComponent>(player, ColliderComponent{});
	ColliderComponent playerCollider{};
	playerCollider.shape = ColliderComponent::Shape::Sphere;
	playerCollider.size = glm::vec3(1.0f);
	m_entityManager.AddComponent<ColliderComponent>(player, playerCollider);
	m_entityManager.AddComponent<SpawnpointComponent>(player, SpawnpointComponent{ playerTransform.position, false });
	m_entityManager.AddComponent<CameraComponent>(player, CameraComponent{});
	m_entityManager.AddComponent<InputComponent>(player, InputComponent{});
	m_entityManager.AddComponent<TagComponent>(player, TagComponent{ "Player" });
	// for footsteps:
	//AudioSourceComponent playerASC{};
	//playerASC.isLooping = false;
	//playerASC.playState = AudioSourceComponent::PlayState::Idle;
	//playerASC.spatial = true;
	//m_entityManager.AddComponent<AudioSourceComponent>(player, playerASC);
	//m_audioSystem.RegisterSource(player, bounceSound, playerASC.spatial, playerASC.isLooping);


	// DEBUG
	std::cout << "DEBUG: Created Player entity" << std::endl;

	// ball 
	EntityID ball = m_entityManager.CreateEntity(); // ID = 5
	TransformComponent ballTransform{};
	ballTransform.position = glm::vec3(0.0f, 5.0f, -3.0f);
	m_entityManager.AddComponent<TransformComponent>(ball, ballTransform);
	RigidBodyComponent ballRB{};
	ballRB.mass = 2.0f;
	ballRB.isStatic = false;
	ballRB.restitution = 0.8f;
	m_entityManager.AddComponent<RigidBodyComponent>(ball, ballRB);
	ColliderComponent ballCollider{};
	ballCollider.shape = ColliderComponent::Shape::Sphere;
	ballCollider.size = glm::vec3(1.7f);
	m_entityManager.AddComponent<ColliderComponent>(ball, ballCollider);
	RenderComponent ballRender{};
	ballRender.meshID = 1;
	ballRender.shaderID = 1;
	ballRender.isVisible = true;
	m_entityManager.AddComponent<RenderComponent>(ball, ballRender);
	m_entityManager.AddComponent<SpawnpointComponent>(ball, SpawnpointComponent{ ballTransform.position, false });
    AudioSourceComponent ballASC{};
	ballASC.isLooping = false;
	ballASC.playState = AudioSourceComponent::PlayState::Idle;
	ballASC.spatial = true;
	m_entityManager.AddComponent<AudioSourceComponent>(ball, ballASC);
	m_audioSystem.RegisterSource(ball, bounceSound, ballASC.spatial, ballASC.isLooping);

	// DEBUG
	std::cout << "DEBUG: Created Ball (Backpack) Entity" << std::endl;

}

void Engine::m_update(float deltaTime) {
	m_inputSystem.Update(m_entityManager, deltaTime);
	m_physicsSystem.Update(m_entityManager, deltaTime);
	m_collisionSystem.Update(m_entityManager);
	m_audioSystem.Update(m_entityManager);
}

void Engine::m_render() {
	glClearColor(0.57f, 0.95f, 0.98f, 1.0f); // state-setting function
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
