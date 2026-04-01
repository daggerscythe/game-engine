#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <string>
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"
#include "../Shader.h"
#include "../Model.h"

class RenderSystem {
public:
	// called once before game loop
	void Init(int screenWidth, int screenHeight);
	void LoadModel(uint32_t id, const std::string& path);
	void LoadShader(uint32_t id, const std::string& vertexPath, const std::string& fragmentPath);
	
	// create simple procedural meshes
	void CreateBoxModel(uint32_t id);

	// called every frame
	void Update(EntityManager& entityManager, float deltatTime);

	// cleanup models and shaders
	void Shutdown();


private:
	// asset storage
	std::unordered_map<uint32_t, Model*> m_models;
	std::unordered_map<uint32_t, Shader*> m_shaders;

	// screen size for projection matrix
	int m_screenWidth = 0;
	int m_screenHeight = 0;

	// internal methods
	void m_updateCamera(EntityManager& entityManager, float deltatTime);
	void m_setLightUniforms(EntityManager& entityManager, Shader& shader);
	void m_drawEntities(EntityManager& entityManager, const glm::vec3& cameraPos);
	void m_updateCameraVectors(CameraComponent& camera);
	
	glm::mat4 m_view = glm::mat4(1.0f);
	glm::mat4 m_projection = glm::mat4(1.0f);
};