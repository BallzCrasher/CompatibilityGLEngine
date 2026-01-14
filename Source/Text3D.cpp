/**
 * @file Text3D.cpp
 * @brief Implementation of the Text3D class.
 */

#include "Text3D.h"

Text3D::Text3D(const std::string& t) : text(t) {
    // Disable shadows for text as it is wireframe/line-based
    this->castsShadow = false; 
}

void Text3D::drawMesh() {
    // Calculate total width of the string in GLUT stroke units
    float rawWidth = glutStrokeLength(GLUT_STROKE_ROMAN, (const unsigned char*)text.c_str());
    
    glPushMatrix();
    
    // Scale down the default GLUT font (approx 120 units high) to ~1 unit
    float scaleFactor = 0.01f; 
    glScalef(scaleFactor, scaleFactor, scaleFactor);
    
    // Center the text horizontally and vertically relative to the object's origin
    glTranslatef(-rawWidth / 2.0f, -60.0f, 0.0f); 

    // Increase line width for better visibility (neon effect)
    glLineWidth(6.0f); 

    for (char c : text) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    }
    
    glLineWidth(1.0f); 

    glPopMatrix();
}
