#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "stb_image.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow*, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool firstMouse = true; // intitial mouse_callback call

float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;

// timing
float deltaTime = 0.0f; // last frame exec time
float lastFrame = 0.0f; // time of last frame

// misc 
bool flashLight = true;

int main()
{
	glfwInit(); // initialized GLFW

	// configuring GLFW for version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creating a window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "openGL demo", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// tell GLFW we want to call resize function by registering
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// tell GLFW we want to capture the mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// call the function each time mouse moves
	glfwSetCursorPosCallback(window, mouse_callback);
	// call the function each time mouse scrolls
	glfwSetScrollCallback(window, scroll_callback);

	// initialize GLAD before calling any OpenGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	// flip loaded textures
	stbi_set_flip_vertically_on_load(true);

	// shader program
	//Shader lightingShader("ObjectVertexShader.glsl", "ObjectFragmentShader.glsl");
	//Shader lightSourceShader("LightingVertexShader.glsl", "LightingFragmentShader.glsl");
	Shader modelShader("ModelVertexShader.glsl", "ModelFragmentShader.glsl");

	std::string modelPath = "models/backpack/backpack.obj";
	Model backpack(&modelPath[0]);

	float vertices[] = {
		// positions       // TexCoordss  // normals
		-0.5f,-0.5f,-0.5f,  0.0f,0.0f,   0.0f,0.0f,-1.0f,
		 0.5f,-0.5f,-0.5f,  1.0f,0.0f,   0.0f,0.0f,-1.0f,
		 0.5f, 0.5f,-0.5f,  1.0f,1.0f,   0.0f,0.0f,-1.0f,
		 0.5f, 0.5f,-0.5f,  1.0f,1.0f,   0.0f,0.0f,-1.0f,
		-0.5f, 0.5f,-0.5f,  0.0f,1.0f,   0.0f,0.0f,-1.0f,
		-0.5f,-0.5f,-0.5f,  0.0f,0.0f,   0.0f,0.0f,-1.0f,

		-0.5f,-0.5f, 0.5f,  0.0f,0.0f,   0.0f,0.0f, 1.0f,
		 0.5f,-0.5f, 0.5f,  1.0f,0.0f,   0.0f,0.0f, 1.0f,
		 0.5f, 0.5f, 0.5f,  1.0f,1.0f,   0.0f,0.0f, 1.0f,
		 0.5f, 0.5f, 0.5f,  1.0f,1.0f,   0.0f,0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f,  0.0f,1.0f,   0.0f,0.0f, 1.0f,
		-0.5f,-0.5f, 0.5f,  0.0f,0.0f,   0.0f,0.0f, 1.0f,

		-0.5f, 0.5f, 0.5f,  1.0f,0.0f,  -1.0f,0.0f,0.0f,
		-0.5f, 0.5f,-0.5f,  1.0f,1.0f,  -1.0f,0.0f,0.0f,
		-0.5f,-0.5f,-0.5f,  0.0f,1.0f,  -1.0f,0.0f,0.0f,
		-0.5f,-0.5f,-0.5f,  0.0f,1.0f,  -1.0f,0.0f,0.0f,
		-0.5f,-0.5f, 0.5f,  0.0f,0.0f,  -1.0f,0.0f,0.0f,
		-0.5f, 0.5f, 0.5f,  1.0f,0.0f,  -1.0f,0.0f,0.0f,

		 0.5f, 0.5f, 0.5f,  1.0f,0.0f,   1.0f,0.0f,0.0f,
		 0.5f, 0.5f,-0.5f,  1.0f,1.0f,   1.0f,0.0f,0.0f,
		 0.5f,-0.5f,-0.5f,  0.0f,1.0f,   1.0f,0.0f,0.0f,
		 0.5f,-0.5f,-0.5f,  0.0f,1.0f,   1.0f,0.0f,0.0f,
		 0.5f,-0.5f, 0.5f,  0.0f,0.0f,   1.0f,0.0f,0.0f,
		 0.5f, 0.5f, 0.5f,  1.0f,0.0f,   1.0f,0.0f,0.0f,

		-0.5f,-0.5f,-0.5f,  0.0f,1.0f,   0.0f,-1.0f,0.0f,
		 0.5f,-0.5f,-0.5f,  1.0f,1.0f,   0.0f,-1.0f,0.0f,
		 0.5f,-0.5f, 0.5f,  1.0f,0.0f,   0.0f,-1.0f,0.0f,
		 0.5f,-0.5f, 0.5f,  1.0f,0.0f,   0.0f,-1.0f,0.0f,
		-0.5f,-0.5f, 0.5f,  0.0f,0.0f,   0.0f,-1.0f,0.0f,
		-0.5f,-0.5f,-0.5f,  0.0f,1.0f,   0.0f,-1.0f,0.0f,

		-0.5f, 0.5f,-0.5f,  0.0f,1.0f,   0.0f, 1.0f,0.0f,
		 0.5f, 0.5f,-0.5f,  1.0f,1.0f,   0.0f, 1.0f,0.0f,
		 0.5f, 0.5f, 0.5f,  1.0f,0.0f,   0.0f, 1.0f,0.0f,
		 0.5f, 0.5f, 0.5f,  1.0f,0.0f,   0.0f, 1.0f,0.0f,
		-0.5f, 0.5f, 0.5f,  0.0f,0.0f,   0.0f, 1.0f,0.0f,
		-0.5f, 0.5f,-0.5f,  0.0f,1.0f,   0.0f, 1.0f,0.0f
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(2.0f, 5.0f, -15.0f), 
		glm::vec3(-1.5f, -2.2f, -2.5f), 
		glm::vec3(-3.8f, -2.0f, -12.3f), 
		glm::vec3(2.4f, -0.4f, -3.5f), 
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f), 
		glm::vec3(1.5f, 2.0f, -2.5f), 
		glm::vec3(1.5f, 0.2f, -1.5f), 
		glm::vec3(-1.3f, 1.0f, -1.5f)
	};

	glm::vec3 lightPositions[] = {
		glm::vec3(0.7f, 0.2f, 2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f, 2.0f, -12.0f),
		glm::vec3(0.0f, 0.0f, -3.0f)
	};

	// generate a vertex array & vertex buffer & element buffer objects
	//unsigned int VBO, objectVAO, lightVAO;
	//glGenVertexArrays(1, &objectVAO);
	//glGenVertexArrays(1, &lightVAO);
	//glGenBuffers(1, &VBO);

	//// bind VAO first, then bind VBO, then configure attribs
	//glBindVertexArray(objectVAO);

	//// GL_STREAM_DRAW - data set once and used a few times
	//// GL_STATIC_DRAW - data set once and used many times
	//// GL_DYNAMIC_DRAW - data is changed and used many times

	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//
	//// tell OpenGL how to interpret the vertex data
	//// position attrbiute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//// texture coord attribute
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
	//// normal attribute
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	//glEnableVertexAttribArray(2);
	//
	//// configuring the light source
	//glBindVertexArray(lightVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//// set vertex attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);

	//unsigned int diffuseMap = loadTexture("container2.png");
	//unsigned int specularMap = loadTexture("container2_specular.png");
	////unsigned int specularMap = loadTexture("container2_color_specular.png");

	//// pass in static uniforms
	//lightingShader.use();

	//// directional light
	//lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, 0.3f);
	//lightingShader.setVec3("dirLight.ambient", glm::vec3(0.1f)); // gray dim ambient light
	//lightingShader.setVec3("dirLight.diffuse", glm::vec3(1.0f)); // brightness of the light
	//lightingShader.setVec3("dirLight.specular", glm::vec3(1.0f)); // intensity of the highlights

	//// point lights
	//for (int i = 0; i < 4; i++)
	//{
	//	std::string curPosition = "pointLights[" + std::to_string(i) + "].position";
	//	lightingShader.setVec3(curPosition, lightPositions[i]);
	//	std::string curConst= "pointLights[" + std::to_string(i) + "].constant";
	//	lightingShader.setFloat(curConst, 1.0f);
	//	std::string curLinear = "pointLights[" + std::to_string(i) + "].linear";
	//	lightingShader.setFloat(curLinear, 0.09f);
	//	std::string curQuad = "pointLights[" + std::to_string(i) + "].quadratic";
	//	lightingShader.setFloat(curQuad, 0.032f);
	//	std::string curAmbient = "pointLights[" + std::to_string(i) + "].ambient";
	//	lightingShader.setVec3(curAmbient, glm::vec3(0.2f));
	//	std::string curDiffuse = "pointLights[" + std::to_string(i) + "].diffuse";
	//	lightingShader.setVec3(curDiffuse, glm::vec3(1.0f));
	//	std::string curSpecular = "pointLights[" + std::to_string(i) + "].specular";
	//	lightingShader.setVec3(curSpecular, glm::vec3(1.0f));
	//}

	//// spot light
	//lightingShader.setVec3("spotLight.position", camera.position);
	//lightingShader.setVec3("spotLight.direction", camera.front);
	//lightingShader.setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
	//lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
	//lightingShader.setVec3("spotLight.ambient", glm::vec3(0.0f)); // gray dim ambient light
	//lightingShader.setVec3("spotLight.diffuse", glm::vec3(1.0f)); // brightness of the light
	//lightingShader.setVec3("spotLight.specular", glm::vec3(1.0f)); // intensity of the highlights

	//// material
	//lightingShader.setInt("material.diffuse", 0); // basically the color/texture
	//lightingShader.setInt("material.specular", 1); // color of the highlights/parts that reflect light
	//lightingShader.setFloat("material.shininess", 64.0f); // radius of the highlight

	//// unbinding, not necessary, but good practice
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//// draw in wireframe polygons
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// pre-frame logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// rendering commands
		// clear the buffer
		//glClearColor(0.69f, 0.94f, 1.0f, 1.0f); // state-setting function
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state-using function

		//lightingShader.use();
		//lightingShader.setVec3("viewPos", camera.position);
		//lightingShader.setBool("flashLight", flashLight);

		modelShader.use();
		// ---------------------------------------------
		// CAMERA STUFF
		// View Matrix - transforms global to view coordinates
		// LookAt matrix - tansforms any vector to camera's coordinate space
		glm::mat4 view = camera.getViewMatrix();
		//lightingShader.setMat4("view", view);
		modelShader.setMat4("view", view);

		// Projection Matrix - adds perspective
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//lightingShader.setMat4("projection", projection);
		modelShader.setMat4("projection", projection);

		// ---------------------------------------------	
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // center of scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		modelShader.setMat4("model", model);
		backpack.Draw(modelShader);

		// bind textures
		//glActiveTexture(GL_TEXTURE0); // activate texture unit
		//glBindTexture(GL_TEXTURE_2D, diffuseMap);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, specularMap);

		// draw objects
		//glBindVertexArray(objectVAO);
		//for (unsigned int i = 0; i < 10; i++)
		//{			
		//	// Model Matrix - transforms local to global coordinates
		//	glm::mat4 model = glm::mat4(1.0f);
		//	model = glm::translate(model, cubePositions[i]);
		//	float angle = 20.0f * i;
		//	model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		//	lightingShader.setMat4("model", model);
		//	
		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//}

		//lightingShader.setVec3("spotLight.position", camera.position);
		//lightingShader.setVec3("spotLight.direction", camera.front);

		//// draw light source
		//// model matrix
		//for (int i = 0; i < 4; i++)
		//{
		//	glm::mat4 lightModel = glm::mat4(1.0f);
		//	lightModel = glm::translate(lightModel, lightPositions[i]);
		//	lightModel = glm::scale(lightModel, glm::vec3(0.5f));

		//	lightSourceShader.use();
		//	lightSourceShader.setMat4("model", lightModel);
		//	lightSourceShader.setMat4("view", view);
		//	lightSourceShader.setMat4("projection", projection);

		//	glBindVertexArray(lightVAO);
		//	glDrawArrays(GL_TRIANGLES, 0, 36);
		//}

		// make the light move around
		//lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
		//lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;

		// check and call events and swap buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// de-allocate resources
	//glDeleteVertexArrays(1, &objectVAO);
	//glDeleteVertexArrays(1, &lightVAO);
	//glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// setting viewport dimensions
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	// ESC
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// WASD
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, deltaTime);

	// flashlight toggle
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		flashLight = !flashLight;

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// prevent sudden snap to center on first call
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos - lastX);
	float yoffset= static_cast<float>(lastY - ypos); // reversed since y-coords are top to bottom
	lastX = (float)xpos;
	lastY = (float)ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset*= sensitivity;

	camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow*, double xoffset, double yoffset)
{
	camera.processMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
	// generate a texture object
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// load and link the image to the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1) format = GL_RED;
		if (nrChannels == 3) format = GL_RGB;
		if (nrChannels == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set wrapping/filtering options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	
	// clean up
	stbi_image_free(data);

	return textureID;
}

// graphics pipeline:
// 1. Vertex Shader - REQUIRED in GL
// takes a single vertex as input and transforms 3D coordinates
// 2. Geometry Shader - OPTIONAL, usually DEFAULT
// generates a collection of vertices into other shapes
// 3. Shape assembly
// assembles all the points into a primitive shape
// 4. Rasterization
// maps primitives to corresponding pixels on screen, resulting in fragments
// before fragment shading, clipping discards 
// all fragments outside your view for better performance
// 5. Fragment Shader - REQUIRED in GL
// Fragment - all data required to render a single pixel
// Calculates the final color of a pixel and all the effects occur
// 6. Alpha tests and Blending
// checks corresponding depth and stencil values to check if
// a fragment is in front or behind other objects and should be dsiacrded
// also checks alpha values and blends the objects together