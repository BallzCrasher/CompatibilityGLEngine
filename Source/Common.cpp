/**
 * @file Common.cpp
 * @brief Implementation of the Material system.
 */

#include "Common.h"

void Material::apply() const {
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    if (diffuse[3] < 1.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

bool Material::isTransparent() const {
    return diffuse[3] < 1.0f;
}

Material Material::CreateGlass() {
    Material m;
    // Dark diffuse color, very low Alpha (0.2)
    m.diffuse[0] = 0.0f; m.diffuse[1] = 0.0f; m.diffuse[2] = 0.1f; m.diffuse[3] = 0.2f; 
    
    // Sync Ambient alpha with Diffuse alpha
    m.ambient[0] = 0.0f; m.ambient[1] = 0.0f; m.ambient[2] = 0.1f; m.ambient[3] = 0.2f; 

    // Pure white specular
    m.specular[0] = 1.0f; m.specular[1] = 1.0f; m.specular[2] = 1.0f; m.specular[3] = 1.0f;
    m.shininess = 120.0f; 
    return m;
}

Material Material::CreateNeon(float r, float g, float b) {
    Material m;
    m.emission[0] = r; m.emission[1] = g; m.emission[2] = b; m.emission[3] = 1.0f;
    m.diffuse[0] = 0.0f; m.diffuse[1] = 0.0f; m.diffuse[2] = 0.0f; m.diffuse[3] = 1.0f;
    return m;
}

Material Material::CreateChrome() {
    Material m;
    // Silver/Grey base
    m.ambient[0] = 0.25f; m.ambient[1] = 0.25f; m.ambient[2] = 0.25f; m.ambient[3] = 1.0f;
    m.diffuse[0] = 0.4f;  m.diffuse[1] = 0.4f;  m.diffuse[2] = 0.4f;  m.diffuse[3] = 1.0f;
    // Very bright, sharp specular
    m.specular[0] = 0.77f; m.specular[1] = 0.77f; m.specular[2] = 0.77f; m.specular[3] = 1.0f;
    m.shininess = 76.8f;
    return m;
}

Material Material::CreateGold() {
    Material m;
    // Golden Ambient/Diffuse
    m.ambient[0] = 0.247f; m.ambient[1] = 0.199f; m.ambient[2] = 0.074f; m.ambient[3] = 1.0f;
    m.diffuse[0] = 0.751f; m.diffuse[1] = 0.606f; m.diffuse[2] = 0.226f; m.diffuse[3] = 1.0f;
    // Golden Specular
    m.specular[0] = 0.628f; m.specular[1] = 0.555f; m.specular[2] = 0.366f; m.specular[3] = 1.0f;
    m.shininess = 51.2f;
    return m;
}

Material Material::CreatePlastic(float r, float g, float b) {
    Material m;
    m.ambient[0] = r * 0.2f; m.ambient[1] = g * 0.2f; m.ambient[2] = b * 0.2f; m.ambient[3] = 1.0f;
    m.diffuse[0] = r;        m.diffuse[1] = g;        m.diffuse[2] = b;        m.diffuse[3] = 1.0f;
    // White specular for plastic look
    m.specular[0] = 1.0f;    m.specular[1] = 1.0f;    m.specular[2] = 1.0f;    m.specular[3] = 1.0f;
    m.shininess = 32.0f;
    return m;
}

Material Material::CreateMatte(float r, float g, float b) {
    Material m;
    m.ambient[0] = r * 0.2f; m.ambient[1] = g * 0.2f; m.ambient[2] = b * 0.2f; m.ambient[3] = 1.0f;
    m.diffuse[0] = r;        m.diffuse[1] = g;        m.diffuse[2] = b;        m.diffuse[3] = 1.0f;
    // No specular
    m.specular[0] = 0.0f;    m.specular[1] = 0.0f;    m.specular[2] = 0.0f;    m.specular[3] = 1.0f;
    m.shininess = 0.0f;
    return m;
}
