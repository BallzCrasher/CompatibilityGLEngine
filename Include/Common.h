/**
 * @file Common.h
 * @brief Defines common helper structures and the Material system for the graphics engine.
 *
 * This header contains utility structures for 3D math (Vec3) and a comprehensive
 * Material structure that wraps OpenGL material properties and provides factory
 * methods for common surface types.
 */

#pragma once
#include <GL/freeglut.h>

/**
 * @struct Vec3
 * @brief A simple structure representing a 3-dimensional vector or point.
 */
struct Vec3 { 
    float x; /**< The X coordinate. */
    float y; /**< The Y coordinate. */
    float z; /**< The Z coordinate. */
};

/**
 * @struct Material
 * @brief Encapsulates standard OpenGL material properties.
 *
 * The Material struct manages Ambient, Diffuse, Specular, and Emission components,
 * as well as shininess. It includes helper functions to apply these settings to
 * the OpenGL state machine.
 */
struct Material {
    /** @brief Ambient color (RGBA). Default is dark grey. */
    float ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

    /** @brief Diffuse color (RGBA). Default is light grey. */
    float diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

    /** @brief Specular highlight color (RGBA). Default is black (no highlight). */
    float specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    /** @brief Emissive color (RGBA). Default is black (no glow). */
    float emission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    /** @brief Specular shininess factor. Higher values create smaller, sharper highlights. */
    float shininess = 0.0f;

    /**
     * @brief Applies the material properties to the current OpenGL state.
     * * Handles `glMaterialfv` calls for all components and automatically enables
     * or disables GL_BLEND based on the alpha value of the diffuse component.
     */
    void apply() const;

    /**
     * @brief Checks if the material is transparent.
     * @return True if the diffuse alpha component is less than 1.0, false otherwise.
     */
    bool isTransparent() const;

    // --- Factory methods ---

    /**
     * @brief Creates a transparent glass material.
     * @return A Material configured with low alpha, dark tint, and high shininess.
     */
    static Material CreateGlass();

    /**
     * @brief Creates a glowing neon material.
     * @param r Red component (0.0 - 1.0).
     * @param g Green component (0.0 - 1.0).
     * @param b Blue component (0.0 - 1.0).
     * @return A Material with high emission and zero diffuse/ambient reflection.
     */
    static Material CreateNeon(float r, float g, float b);
    
    /**
     * @brief Creates a highly reflective chrome material.
     * @return A Material with grey base and bright, sharp specular highlights.
     */
    static Material CreateChrome();

    /**
     * @brief Creates a metallic gold material.
     * @return A Material calibrated to resemble physical gold.
     */
    static Material CreateGold();

    /**
     * @brief Creates a shiny plastic material.
     * @param r Red component (0.0 - 1.0).
     * @param g Green component (0.0 - 1.0).
     * @param b Blue component (0.0 - 1.0).
     * @return A Material with the specified color and white specular highlights.
     */
    static Material CreatePlastic(float r, float g, float b);

    /**
     * @brief Creates a flat matte material.
     * @param r Red component (0.0 - 1.0).
     * @param g Green component (0.0 - 1.0).
     * @param b Blue component (0.0 - 1.0).
     * @return A Material with the specified color and no specular highlights.
     */
    static Material CreateMatte(float r, float g, float b);
};
