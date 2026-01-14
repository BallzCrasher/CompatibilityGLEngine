/**
 * @file Lighting.cpp
 * @brief Implementation of the lighting system.
 */

#include "Lighting.h"

void DirectionalLight::enable() {
    glEnable(GL_LIGHT0);
    
    // Directional light indicated by w=0.0
    GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    GLfloat diffuse_light[] = { 1.0f, 0.95f, 0.8f, 1.0f }; // Warm Yellow/Orange
    GLfloat specular_light[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light);
}

PointLight::PointLight(int id, float _x, float _y, float _z, float _r, float _g, float _b, float _intensity) 
    : lightId(GL_LIGHT1 + id), r(_r), g(_g), b(_b), intensity(_intensity) 
{
    this->setPosition(_x, _y, _z);
}

void PointLight::enable() {
    // Prevent accessing invalid light IDs
    if (lightId > GL_LIGHT7) return; 
    glEnable(lightId);
    
    // Positional light indicated by w=1.0
    GLfloat light_position[] = { position.x, position.y, position.z, 1.0f }; 
    
    // Multiply color by intensity
    GLfloat light_diffuse[] = { r * intensity, g * intensity, b * intensity, 1.0f };
    GLfloat light_specular[] = { r * intensity, g * intensity, b * intensity, 1.0f };

    // Standard attenuation factor
    GLfloat attenuation = 0.05f;

    glLightfv(lightId, GL_POSITION, light_position);
    glLightfv(lightId, GL_DIFFUSE, light_diffuse);
    glLightfv(lightId, GL_SPECULAR, light_specular);
    glLightf(lightId, GL_LINEAR_ATTENUATION, attenuation);
}

void PointLight::drawMesh() {
    // Intentionally empty
}

GameObject* PointLight::clone() const {
    // Uses the default Copy Constructor (copies position, color, AND lightId)
    return new PointLight(*this);
}
