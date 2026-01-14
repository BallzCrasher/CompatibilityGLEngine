/**
 * @file GameObject.cpp
 * @brief Implementation of the GameObject class and basic primitives.
 */

#include "GameObject.h"
#include <cmath> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- GameObject Implementation ---

void GameObject::setPosition(float x, float y, float z) { position = { x, y, z }; }
void GameObject::setRotation(float x, float y, float z) { rotation = { x, y, z }; }
void GameObject::setScale(float x, float y, float z) { scale = { x, y, z }; }
void GameObject::setMaterial(const Material& m) { material = m; }

bool GameObject::isTransparent() const {
    return material.isTransparent();
}

void GameObject::setUpdateCallback(UpdateCallback action) {
    this->updateAction = action;
}

void GameObject::update(float deltaTime) {
    // If a behavior has been assigned, run it
    if (this->updateAction) {
        this->updateAction(this, deltaTime);
    }
}

void GameObject::setInteractCallback(InteractCallback action) {
    this->interactAction = action;
}

void GameObject::interact() {
	// 1. If this object has a specific action, run it.
    if (this->interactAction) {
        this->interactAction(this);
    } 
    // 2. If not, pass the click up to the parent (Bubbling)
    else if (this->parent != nullptr) {
        this->parent->interact();
    }
}

void GameObject::rotateAround(float px, float py, float pz, float ax, float ay, float az, float angle) {
    // 1. Convert Angle to Radians
    float rad = angle * (float)(M_PI / 180.0f);
    float c = cos(rad);
    float s = sin(rad);
    float t = 1.0f - c;

    // 2. Normalize the Axis Vector (ax, ay, az)
    float len = sqrt(ax * ax + ay * ay + az * az);
    if (len < 0.0001f) return; // Prevent divide by zero
    ax /= len; ay /= len; az /= len;

    // 3. Calculate Relative Position (Vector from Pivot to Object)
    float rx = position.x - px;
    float ry = position.y - py;
    float rz = position.z - pz;

    // 4. Apply Rodrigues' Rotation Formula
    // This rotates vector (rx,ry,rz) around axis (ax,ay,az)
    float newX = (t * (ax * rx + ay * ry + az * rz) * ax) + (c * rx) + (s * (ay * rz - az * ry));
    float newY = (t * (ax * rx + ay * ry + az * rz) * ay) + (c * ry) + (s * (az * rx - ax * rz));
    float newZ = (t * (ax * rx + ay * ry + az * rz) * az) + (c * rz) + (s * (ax * ry - ay * rx));

    // 5. Update Position
    position.x = px + newX;
    position.y = py + newY;
    position.z = pz + newZ;

    // 6. Update Orientation (Simple Euler addition)
    // This ensures the object faces the correct direction as it orbits.
    rotation.x += angle * ax;
    rotation.y += angle * ay;
    rotation.z += angle * az;
}

Vec3 GameObject::getRealRotation() const {
    // 1. If we have a parent, add our rotation to theirs
    if (parent) {
        Vec3 pRot = parent->getRealRotation();
        return { pRot.x + rotation.x, pRot.y + rotation.y, pRot.z + rotation.z };
    }
    // 2. Otherwise, our local rotation is the real rotation
    return rotation;
}

/**
 * @brief Helper to apply Euler rotation to a vector (XYZ order).
 */
static Vec3 applyRotation(Vec3 v, Vec3 rotDeg) {
    float radX = rotDeg.x * (float)(M_PI / 180.0f);
    float radY = rotDeg.y * (float)(M_PI / 180.0f);
    float radZ = rotDeg.z * (float)(M_PI / 180.0f);

    float x = v.x, y = v.y, z = v.z;

    // X-Axis
    float y1 = y * cos(radX) - z * sin(radX);
    float z1 = y * sin(radX) + z * cos(radX);
    float x1 = x;

    // Y-Axis
    float x2 = x1 * cos(radY) + z1 * sin(radY);
    float z2 = -x1 * sin(radY) + z1 * cos(radY);
    float y2 = y1;

    // Z-Axis
    float x3 = x2 * cos(radZ) - y2 * sin(radZ);
    float y3 = x2 * sin(radZ) + y2 * cos(radZ);
    float z3 = z2;

    return { x3, y3, z3 };
}

/**
 * @brief Helper to apply INVERSE Euler rotation to a vector.
 */
static Vec3 applyInverseRotation(Vec3 v, Vec3 rotDeg) {
    // Inverse Rotation Order: Z, then Y, then X (with negative angles)
    float radX = -rotDeg.x * (float)(M_PI / 180.0f);
    float radY = -rotDeg.y * (float)(M_PI / 180.0f);
    float radZ = -rotDeg.z * (float)(M_PI / 180.0f);

    float x = v.x, y = v.y, z = v.z;

    // 1. Z-Axis
    float x1 = x * cos(radZ) - y * sin(radZ);
    float y1 = x * sin(radZ) + y * cos(radZ);
    float z1 = z;

    // 2. Y-Axis
    float x2 = x1 * cos(radY) + z1 * sin(radY);
    float z2 = -x1 * sin(radY) + z1 * cos(radY);
    float y2 = y1;

    // 3. X-Axis
    float y3 = y2 * cos(radX) - z2 * sin(radX);
    float z3 = y2 * sin(radX) + z2 * cos(radX);
    float x3 = x2;

    return { x3, y3, z3 };
}

Vec3 GameObject::getRealPosition() const {
    // 1. If no parent, local position IS real position
    if (!parent) {
        return position;
    }
    // Note: getRealPosition is essentially getPointInWorldSpace({0,0,0})
    // But we implement it recursively here via parents.
    return parent->getPointInWorldSpace(position);
}

Vec3 GameObject::getPointInWorldSpace(Vec3 localPoint) const {
    // 1. Apply Local Scale
    Vec3 p = localPoint;
    p.x *= scale.x;
    p.y *= scale.y;
    p.z *= scale.z;

    // 2. Apply Local Rotation
    p = applyRotation(p, rotation);

    // 3. Apply Local Position
    p.x += position.x;
    p.y += position.y;
    p.z += position.z;

    // 4. Transform by Parent
    if (parent) {
        return parent->getPointInWorldSpace(p);
    }

    return p;
}

Vec3 GameObject::getPointInLocalSpace(Vec3 worldPoint) const {
    Vec3 p = worldPoint;

    // 1. Transform by Parent (Recursive Step: World -> ParentLocal)
    if (parent) {
        p = parent->getPointInLocalSpace(p);
    }

    // 2. Apply Inverse Local Position
    p.x -= position.x;
    p.y -= position.y;
    p.z -= position.z;

    // 3. Apply Inverse Local Rotation
    p = applyInverseRotation(p, rotation);

    // 4. Apply Inverse Local Scale
    if (std::abs(scale.x) > 0.0001f) p.x /= scale.x;
    if (std::abs(scale.y) > 0.0001f) p.y /= scale.y;
    if (std::abs(scale.z) > 0.0001f) p.z /= scale.z;

    return p;
}

void GameObject::draw() {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1, 0, 0);
    glRotatef(rotation.y, 0, 1, 0);
    glRotatef(rotation.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);
    
    material.apply();
    drawMesh();
    
    glPopMatrix();
}

// --- Primitive Shape Implementations ---

void Cube::drawMesh() { glutSolidCube(1.0); }

void Cylinder::drawMesh() { glutSolidCylinder(0.5, 1.0, 20, 20); }

void Plane::drawMesh() {
    // Determine the number of divisions (e.g., 20x20 grid)
    // More divisions = better lighting/fog quality but more processing
    int divisions = 20; 
    float step = 2.0f / divisions; // Total width is 2 (-1 to 1)

    glNormal3f(0, 1, 0); // Normal points up for the whole floor

    glBegin(GL_QUADS);
    for (int z = 0; z < divisions; ++z) {
        for (int x = 0; x < divisions; ++x) {
            // Calculate coordinates for this grid cell
            float x1 = -1.0f + x * step;
            float z1 = -1.0f + z * step;
            float x2 = x1 + step;
            float z2 = z1 + step;

            // Draw the cell
            glVertex3f(x1, 0, z1);
            glVertex3f(x1, 0, z2);
            glVertex3f(x2, 0, z2);
            glVertex3f(x2, 0, z1);
        }
    }
    glEnd();
}

// --- Collision Box Implementation ---

CollisionBox::CollisionBox(float w, float h, float d) 
    : width(w), height(h), depth(d) {
    castsShadow = false; 
}

void CollisionBox::drawMesh() {
#ifdef SHOW_COLLISION_BOXES
    // 1. Check if global lighting is currently enabled.
    // In your engine (main.cpp), GL_LIGHTING is disabled ONLY during the Shadow Pass.
    GLboolean lightingIsOn;
    glGetBooleanv(GL_LIGHTING, &lightingIsOn);

    // 2. If Lighting is OFF, we are rendering shadows. 
    // Do NOT draw the debug wires, or they will be flattened onto the floor.
    if (!lightingIsOn) {
        return; 
    }

    // 3. Normal Drawing (Use PushAttrib to prevent flickering)
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT); 

    glDisable(GL_LIGHTING);      // Force lighting off for the wireframe (so it's bright magenta)
    glDisable(GL_TEXTURE_2D);    
    glColor3f(1.0f, 0.0f, 1.0f); // Magenta
    
    glPushMatrix();
    glScalef(width, height, depth);
    glutWireCube(1.0);
    glPopMatrix();
    
    glPopAttrib(); 
#endif
}
