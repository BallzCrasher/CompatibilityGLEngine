/**
 * @file Lighting.h
 * @brief Defines classes for managing OpenGL light sources.
 *
 * This header contains the DirectionalLight class for global lighting (e.g., the sun)
 * and the PointLight class for local, positioned light sources (e.g., lamps, fire).
 * PointLights inherit from GameObject to allow them to be positioned and transformed
 * within the scene graph.
 */

#pragma once
#include <GL/freeglut.h>
#include "GameObject.h"

/**
 * @class DirectionalLight
 * @brief Manages a global directional light source (infinite distance).
 *
 * Typically represents the sun or moon. The light rays are parallel,
 * meaning the position vector acts as a direction vector (w=0).
 */
class DirectionalLight {
public:
    /**
     * @brief Enables and configures the directional light (GL_LIGHT0).
     * * Sets the light position (direction), diffuse color, and specular color.
     */
    void enable();
};

/**
 * @class PointLight
 * @brief Manages a local positional light source.
 *
 * Point lights radiate outward from a specific position in 3D space (w=1).
 * They inherit from GameObject, allowing them to be attached to other objects
 * or moved around the scene easily.
 */
class PointLight : public GameObject { 
private:
    /** @brief The OpenGL light identifier (e.g., GL_LIGHT1, GL_LIGHT2). */
    int lightId;
    
    /** @brief Red color component (0.0 - 1.0). */
    float r;
    /** @brief Green color component (0.0 - 1.0). */
    float g;
    /** @brief Blue color component (0.0 - 1.0). */
    float b;
    
    /** @brief Brightness multiplier for the light. */
    float intensity;

public:
    /**
     * @brief Constructs a new PointLight.
     * @param id The internal ID offset (0 for GL_LIGHT1, 1 for GL_LIGHT2, etc.).
     * @param _x X coordinate in world space.
     * @param _y Y coordinate in world space.
     * @param _z Z coordinate in world space.
     * @param _r Red color component.
     * @param _g Green color component.
     * @param _b Blue color component.
     * @param _intensity Brightness multiplier.
     */
    PointLight(int id, float _x, float _y, float _z, float _r, float _g, float _b, float _intensity);
    
    /**
     * @brief Enables and configures the point light in OpenGL.
     * * Sets the position based on the GameObject's current location.
     * * Calculates diffuse and specular components based on color and intensity.
     * * Applies linear attenuation to simulate falloff.
     */
    void enable();

    /**
     * @brief Overrides the mesh drawing method.
     * * PointLights are generally invisible in the rendered scene (unless a debug mesh is added),
     * so this implementation is intentionally empty.
     */
    void drawMesh() override; 
    
    /**
     * @brief Creates a copy of this light source.
     * @return A pointer to the new PointLight instance.
     */
    GameObject* clone() const override;
};
