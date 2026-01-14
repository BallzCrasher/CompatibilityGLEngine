/**
 * @file Model.cpp
 * @brief Implementation of the Model class using Assimp and stb_image.
 */

#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::Model(const std::string& path) {
    Assimp::Importer importer;
    
    // Load scene with flags for triangulation, smoothing, UV flipping, and pre-transforming vertices
	const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_FlipUVs | 
        aiProcess_PreTransformVertices 
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    // 1. Load Materials (Textures + Properties)
    loadMaterials(scene);

    // 2. Process Geometry
    processNode(scene->mRootNode, scene);
}

void Model::loadMaterials(const aiScene* scene) {
    // Resize the materials vector to match the scene's material count
    loadedMaterials.resize(scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* aiMat = scene->mMaterials[i];
        Material& myMat = loadedMaterials[i]; 

        // --- Load Material Properties (Colors) ---
        aiColor3D color(0.f, 0.f, 0.f);
        float shininess = 0.0f;

        // Diffuse
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            myMat.diffuse[0] = color.r; myMat.diffuse[1] = color.g; myMat.diffuse[2] = color.b;
            myMat.diffuse[3] = 1.0f; 
        }

        // Specular
        if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            myMat.specular[0] = color.r; myMat.specular[1] = color.g; myMat.specular[2] = color.b;
        }

        // Ambient
        if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
            myMat.ambient[0] = color.r; myMat.ambient[1] = color.g; myMat.ambient[2] = color.b;
        }

        // Emission
        if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
            myMat.emission[0] = color.r; myMat.emission[1] = color.g; myMat.emission[2] = color.b;
        }

        // Shininess
        if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            myMat.shininess = shininess;
        }

        // --- Load Textures ---
        if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString str;
            aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            GLuint textureId = loadTextureFromFile(str.C_Str(), this->directory);
            textures.push_back(textureId);
        } else {
            textures.push_back(0); 
        }
    }
}

GLuint Model::loadTextureFromFile(const char* path, const std::string& dir) {
	std::cerr << "Loading texture: " << path << std::endl;
    std::string filename = std::string(path);
	
	std::filesystem::path modelDir(directory);
	std::filesystem::path texPath(path); 

	// Handle absolute vs relative paths
	std::filesystem::path fullPath;
	if (texPath.is_absolute()) {
		fullPath = texPath;
	} else {
		fullPath = modelDir / texPath;
	}

	std::cerr << "Loading texture: " << fullPath << std::endl;
	
	if (!std::filesystem::exists(fullPath)) {
		std::cerr << "Texture missing: " << fullPath << std::endl;
		return 0;
	}

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << fullPath << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshEntry myMesh;
        myMesh.materialIndex = mesh->mMaterialIndex;

        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            myMesh.vertices.push_back(mesh->mVertices[j].x);
            myMesh.vertices.push_back(mesh->mVertices[j].y);
            myMesh.vertices.push_back(mesh->mVertices[j].z);
            
            if (mesh->HasNormals()) {
                myMesh.normals.push_back(mesh->mNormals[j].x);
                myMesh.normals.push_back(mesh->mNormals[j].y);
                myMesh.normals.push_back(mesh->mNormals[j].z);
            }
            
            if (mesh->HasTextureCoords(0)) {
                myMesh.texCoords.push_back(mesh->mTextureCoords[0][j].x);
                myMesh.texCoords.push_back(mesh->mTextureCoords[0][j].y);
            } else {
                myMesh.texCoords.push_back(0.0f);
                myMesh.texCoords.push_back(0.0f);
            }
        }
        
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
                myMesh.indices.push_back(face.mIndices[k]);
        }
        meshes.push_back(myMesh);
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

void Model::drawMesh() {
    glEnable(GL_TEXTURE_2D);

    for (auto& mesh : meshes) {
        
        // Apply extracted material properties
        if (mesh.materialIndex < loadedMaterials.size()) {
            loadedMaterials[mesh.materialIndex].apply();
        }

        // Apply texture if available
        if (mesh.materialIndex < textures.size() && textures[mesh.materialIndex] != 0) {
            glBindTexture(GL_TEXTURE_2D, textures[mesh.materialIndex]);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0); 
        }

        glBegin(GL_TRIANGLES);
        for (unsigned int idx : mesh.indices) {
            if (!mesh.normals.empty())
                glNormal3f(mesh.normals[idx * 3], mesh.normals[idx * 3 + 1], mesh.normals[idx * 3 + 2]);
            
            if (!mesh.texCoords.empty())
                glTexCoord2f(mesh.texCoords[idx * 2], mesh.texCoords[idx * 2 + 1]);

            glVertex3f(mesh.vertices[idx * 3], mesh.vertices[idx * 3 + 1], mesh.vertices[idx * 3 + 2]);
        }
        glEnd();
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}
