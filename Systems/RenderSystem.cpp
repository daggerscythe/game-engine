#include "RenderSystem.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void RenderSystem::Init(int screenWidth, int screenHeight) {
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

void RenderSystem::LoadModel(uint32_t id, const std::string& path) {
	if (m_models.find(id) != m_models.end()) {
		std::cout << "Model ID " << id << " already loaded" << std::endl;
		return;
	}
	m_models[id] = new Model(path.c_str());
}

void RenderSystem::LoadShader(uint32_t id, const std::string& vertexPath, const std::string& fragmentPath) {
	if (m_shaders.find(id) != m_shaders.end()) {
		std::cout << "Shader ID " << id << " already loaded" << std::endl;
		return;
	}
	m_shaders[id] = new Shader(vertexPath.c_str(), fragmentPath.c_str());
}

void RenderSystem::Update(EntityManager& entityManager, float deltaTime) {
	m_updateCamera(entityManager, deltaTime);
	m_drawEntities(entityManager);
}

void RenderSystem::Shutdown() {
	for (auto& [id, model] : m_models) delete model;
	for (auto& [id, shader] : m_shaders) delete shader;
	m_models.clear();
	m_shaders.clear();
}

void RenderSystem::m_updateCamera(EntityManager& entityManager, float deltaTime) {
	// get all camera entities
	auto entities = entityManager.GetEntitiesWith<CameraComponent, InputComponent>();

	for (EntityID entity : entities) {
		CameraComponent& camera = entityManager.GetComponent<CameraComponent>(entity);
		InputComponent& input = entityManager.GetComponent<InputComponent>(entity);
	
		// apply mouse delta to yaw and pitch
		camera.yaw += input.mouseDeltaX * camera.sensitivity;
		camera.pitch += input.mouseDeltaY * camera.sensitivity;

		// clamp pitch
		if (camera.pitch > 89.0f) camera.pitch = 89.0f;
		if (camera.pitch < -89.0f) camera.pitch = -89.0f;

		// recalculate the big 3
		m_updateCameraVectors(camera);

		// process zoom
		camera.zoom -= input.scrollDelta;
		if (camera.zoom < 1.0f) camera.zoom = 1.0f;
		if (camera.zoom > 45.0f) camera.zoom = 45.0f;

		// process keyboard
		float velocity = camera.speed * deltaTime;
		if (input.moveForward) camera.position += camera.front * velocity;
		if (input.moveBackward) camera.position -= camera.front * velocity;
		if (input.moveLeft) camera.position -= camera.right * velocity;
		if (input.moveRight) camera.position += camera.right * velocity;

		// cannot fly around
		// camera.position.y = 0.0f;

		// build matrices
		m_view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
		m_projection = glm::perspective(
			glm::radians(camera.zoom), 
			(float)m_screenWidth / (float)m_screenHeight,
			0.1f,
			100.0f
		);
	}
}

void RenderSystem::m_setLightUniforms(EntityManager& entityManager, Shader& shader) {
	auto lights = entityManager.GetEntitiesWith<LightComponent, TransformComponent>();

	int pointLightIndex = 0;

	for (EntityID entity : lights) {
		LightComponent& lc = entityManager.GetComponent<LightComponent>(entity);
		TransformComponent& tc = entityManager.GetComponent<TransformComponent>(entity);

		if (!lc.isOn) continue;

		if (lc.type == LightComponent::Type::Directional) {
			shader.setVec3("dirLight.direction", lc.direction);
			shader.setVec3("dirLight.ambient", lc.ambient);
			shader.setVec3("dirLight.diffuse", lc.diffuse);
			shader.setVec3("dirLight.specular", lc.specular);
		}
		else if (lc.type == LightComponent::Type::Point) {
			if (pointLightIndex >= 16) continue; // max # of point lights reached
			std::string base = "pointLights[" + std::to_string(pointLightIndex) + "]";
			shader.setVec3(base + ".position", tc.position);
			shader.setVec3(base + ".ambient", lc.ambient);
			shader.setVec3(base + ".diffuse", lc.diffuse);
			shader.setVec3(base + ".specular", lc.specular);
			shader.setFloat(base + ".constant", lc.constant);
			shader.setFloat(base + ".linear", lc.linear);
			shader.setFloat(base + ".linear", lc.linear);
			shader.setFloat(base + ".quadratic", lc.quadratic);
			pointLightIndex++;
		}
		else if (lc.type == LightComponent::Type::Spot) {
			shader.setVec3("spotLight.position", tc.position);
			shader.setVec3("spotLight.direction", lc.direction);
			shader.setVec3("spotLight.ambient", lc.ambient);
			shader.setVec3("spotLight.diffuse", lc.diffuse);
			shader.setVec3("spotLight.specular", lc.specular);
			shader.setFloat("spotLight.constant", lc.constant);
			shader.setFloat("spotLight.linear", lc.linear);
			shader.setFloat("spotLight.quadratic", lc.quadratic);
			shader.setFloat("spotLight.innerCutOff", glm::cos(glm::radians(lc.innerCutOff)));
			shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(lc.outerCutOff)));
		}
	}
	shader.setInt("numPointLigths", pointLightIndex);
}

void RenderSystem::m_drawEntities(EntityManager& entityManager) {
	auto entities = entityManager.GetEntitiesWith<RenderComponent, TransformComponent>();

	for (EntityID entity : entities) {
		RenderComponent& rc = entityManager.GetComponent<RenderComponent>(entity);
		TransformComponent& tc = entityManager.GetComponent<TransformComponent>(entity);

		if (!rc.isVisible) continue;

		// look up shader and model
		if (m_shaders.find(rc.shaderID) == m_shaders.end()) continue;
		if (m_models.find(rc.meshID) == m_models.end()) continue;

		Shader& shader = *m_shaders[rc.shaderID];
		Model& model = *m_models[rc.meshID];

		shader.use();

		// set matrices
		shader.setMat4("view", m_view);
		shader.setMat4("projection", m_projection);

		// build & set model matrix
		glm::mat4 m_model = glm::mat4(1.0f);
		m_model = glm::translate(m_model, tc.position);
		m_model = glm::rotate(m_model, glm::radians(tc.rotation.x), glm::vec3(1, 0, 0));
		m_model = glm::rotate(m_model, glm::radians(tc.rotation.y), glm::vec3(0, 1, 0));
		m_model = glm::rotate(m_model, glm::radians(tc.rotation.z), glm::vec3(0, 0, 1));
		m_model - glm::scale(m_model, tc.scale);
		shader.setMat4("model", m_model);

		// set light unifroms
		m_setLightUniforms(entityManager, shader);

		// draw
		model.Draw(shader);
	}
}

void RenderSystem::m_updateCameraVectors(CameraComponent& camera) {
	// calculate new front
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	newFront.y = sin(glm::radians(camera.pitch));
	newFront.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.front = glm::normalize(newFront);

	// re-calculate right and up
	camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
	camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}
