/**
 * @file Camera.cpp
 * @brief Implementation of the Camera class.
 */

#include "Camera.h"
#include <GL/freeglut.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Camera::updateLook() {
    // Convert Euler angles (yaw, pitch) to a direction vector
    float lx = cos(pitch * M_PI / 180.0f) * cos(yaw * M_PI / 180.0f);
    float ly = sin(pitch * M_PI / 180.0f);
    float lz = cos(pitch * M_PI / 180.0f) * sin(yaw * M_PI / 180.0f);
    
    // Set the view matrix looking from (x,y,z) to (x+lx, y+ly, z+lz)
    gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0.0f, 1.0f, 0.0f);     
}

void Camera::mouseMove(int mx, int my) {
    // Static variables to store the previous mouse position
    static int lastX = mx, lastY = my;

    // Calculate mouse delta
    float xoffset = mx - lastX;
    float yoffset = lastY - my; // Reversed since Y-coordinates range from bottom to top
    lastX = mx; lastY = my;

    // Apply sensitivity
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    // Clamp pitch to prevent screen flipping at the zenith/nadir
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}
