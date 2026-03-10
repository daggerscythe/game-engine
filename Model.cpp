#include "Model.h"

Model::Model(const char* path) 
{
	m_loadModel(path);
}
void Model::Draw(Shader& shader)
{
	for (unsigned int i = 0; i < m_meshes.size(); i++)
		m_meshes[i].Draw(shader);
}

void Model::m_loadModel(string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		return;
	}
	m_directory = path.substr(0, path.find_last_of('/'));

	m_processNode(scene->mRootNode, scene);
}
void Model::m_processNode(aiNode* node, const aiScene* scene)
{
	// process all node's meshes
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(m_processMesh(mesh, scene));
	}
	// process node's children's meshes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		m_processNode(node->mChildren[i], scene);
}
Mesh Model::m_processMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// vertex position
		float x = mesh->mVertices[i].x;
		float y = mesh->mVertices[i].y;
		float z = mesh->mVertices[i].z;
		vertex.Position = glm::vec3(x, y, z);
		// vertex normal
		x = mesh->mNormals[i].x;
		y = mesh->mNormals[i].y;
		z = mesh->mNormals[i].z;
		vertex.Normal = glm::vec3(x, y, z);
		// vertex texture coordinates
		if (mesh->mTextureCoords[0])
		{
			x = mesh->mTextureCoords[0][i].x;
			y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = glm::vec2(x, y);
		}
		else
			vertex.TexCoords = glm::vec2(0.0f);
		// add vertex to vertices
		vertices.push_back(vertex);
	}
	// process indeces
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// diffuse maps
		vector<Texture> diffuseMaps = m_loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// specular maps
		vector<Texture> specularMaps = m_loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
	return Mesh(vertices, indices, textures);
}
vector<Texture> Model::m_loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString filePath;
		mat->GetTexture(type, i, &filePath);
		bool skip = false;
		for (unsigned int j = 0; j < m_textures_loaded.size(); j++)
		{
			if (std::strcmp(m_textures_loaded[j].path.data(), filePath.C_Str()) == 0)
			{
				textures.push_back(m_textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			Texture texture;
			texture.id = textureFromFile(filePath.C_Str(), m_directory);
			texture.type = typeName;
			texture.path = filePath.C_Str();
			textures.push_back(texture);
			m_textures_loaded.push_back(texture);
		}
	}
	return textures;
}

unsigned int textureFromFile(const char* path, const string& directory, bool gammma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	// generate a texture object
	unsigned int textureID;
	glGenTextures(1, &textureID);

	// load and link the image to the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// clean up
	stbi_image_free(data);

	return textureID;
}