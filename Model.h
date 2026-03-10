#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
using namespace std;

#include "Shader.h"
#include "Mesh.h"
#include "stb_image.h"

unsigned int textureFromFile(const char* path, const string& directory, bool gammma = false);

class Model
{
public: 
	Model(const char* path);
	void Draw(Shader& shader);
private:
	vector<Mesh> m_meshes;
	string m_directory;
	vector<Texture> m_textures_loaded;

	void m_loadModel(string path);
	void m_processNode(aiNode* node, const aiScene* scene);
	Mesh m_processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> m_loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};

