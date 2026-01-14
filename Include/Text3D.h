/**
 * @file Text3D.h
 * @brief Defines a GameObject for rendering 3D text strings.
 *
 * This header contains the Text3D class, which inherits from GameObject.
 * It provides functionality to render string data as 3D stroke characters
 * within the scene graph.
 */

#pragma once
#include "GameObject.h"
#include <string>
#include <GL/freeglut.h>

/**
 * @class Text3D
 * @brief A GameObject that displays a text string in 3D space.
 *
 * This class wraps GLUT's stroke font capabilities to render text that allows
 * the background to show through. It automatically centers the text and 
 * scales it to be consistent with the engine's unit system.
 */
class Text3D : public GameObject {
public:
    /** @brief The string content to display. */
    std::string text;

    /**
     * @brief Constructs a new Text3D object.
     * @param t The string to render.
     */
    Text3D(const std::string& t);
    
    /**
     * @brief Renders the text as a 3D wireframe mesh.
     * * Calculates the string width to center it, scales the large default GLUT
     * stroke font down to a manageable size, and renders it with increased line width.
     */
    void drawMesh() override;

    /**
     * @brief Creates a deep copy of the text object.
     * @return A pointer to the new Text3D instance.
     */
	GameObject* clone() const override { return new Text3D(*this); }
};
