/**
 * @file Model.h
 * @brief Defines the Model class for loading and rendering 3D assets.
 *
 * This header contains the Model class, which uses the Open Asset Import Library (Assimp)
 * to load external 3D models (OBJ, FBX, DAE, etc.). It handles mesh processing,
 * texture loading, and material extraction to render complex geometry within the
 * engine's GameObject framework.
 */

#pragma once
#include "GameObject.h"
#include <vector>
#include <string>
#include <assimp/scene.h>
#include <GL/freeglut.h> 

/**
 * @class Model
 * @brief A GameObject representing an imported 3D model.
 *
 * The Model class encapsulates the data required to render an external 3D model.
 * It manages a collection of sub-meshes, their associated textures, and materials.
 * It overrides the standard drawMesh method to render these sub-meshes with
 * the correct properties.
 */
class Model : public GameObject {
private:
    /**
     * @struct MeshEntry
     * @brief Internal structure holding geometry data for a single sub-mesh.
     */
    struct MeshEntry {
        std::vector<float> vertices;      /**< Flattened list of vertex positions (x, y, z). */
        std::vector<float> normals;       /**< Flattened list of vertex normals (x, y, z). */
        std::vector<float> texCoords;     /**< Flattened list of texture coordinates (u, v). */
        std::vector<unsigned int> indices; /**< Indices for indexed drawing. */
        unsigned int materialIndex;       /**< Index into the loadedMaterials and textures arrays. */
    };
    
    /** @brief Collection of meshes that make up the model. */
    std::vector<MeshEntry> meshes;
    
    /** @brief List of OpenGL texture IDs associated with the model's materials. */
    std::vector<GLuint> textures;
    
    /** @brief List of material properties extracted from the model file. */
    std::vector<Material> loadedMaterials; 
    
    /** @brief The root directory of the model file, used for loading relative texture paths. */
    std::string directory;

    /**
     * @brief Recursively processes Assimp nodes to extract mesh data.
     * @param node The current Assimp node being processed.
     * @param scene The root Assimp scene object.
     */
    void processNode(aiNode* node, const aiScene* scene);

    /**
     * @brief Extracts material properties (colors, textures) from the Assimp scene.
     * @param scene The Assimp scene containing material definitions.
     */
    void loadMaterials(const aiScene* scene); 

    /**
     * @brief Loads a texture from disk and generates an OpenGL texture ID.
     * @param path The relative or absolute path to the image file.
     * @param directory The directory context for relative paths.
     * @return The OpenGL ID of the generated texture, or 0 on failure.
     */
    GLuint loadTextureFromFile(const char* path, const std::string& directory);

public:
    /**
     * @brief Constructs a Model by loading a file from disk.
     * @param path The file path to the 3D model.
     */
    Model(const std::string& path);

    /**
     * @brief Renders the model.
     * * Iterates through all sub-meshes, binds their specific textures and materials,
     * and issues OpenGL drawing commands.
     */
    void drawMesh() override;

    /**
     * @brief Creates a deep copy of the model.
     * @return A pointer to the new Model instance.
     */
    GameObject* clone() const override { return new Model(*this); }
};
