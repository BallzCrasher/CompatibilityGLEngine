/**
 * @file Container.cpp
 * @brief Implementation of the Container class.
 */

#include "Container.h"
#include <algorithm> 

Container::Container() {}

Container::~Container() {
    // When the container is destroyed, delete all children to prevent memory leaks
    for (auto* child : children) {
        delete child;
    }
    children.clear();
}

void Container::update(float deltaTime) {
    // Run the container's own behavior
    GameObject::update(deltaTime);

    // Propagate update to all children
    for (auto* child : children) {
        child->update(deltaTime);
    }
}

void Container::addChild(GameObject* child) {
    // Set the parent relationship
    child->setParent(this);
    children.push_back(child);
}

void Container::draw() {
    glPushMatrix();

    // Apply local transformation
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1, 0, 0);
    glRotatef(rotation.y, 0, 1, 0);
    glRotatef(rotation.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);

    // Draw all children relative to this container
    for (auto* child : children) {
        child->draw();
    }

    glPopMatrix();
}

void Container::drawOpaqueChildren() {
    glPushMatrix();
    
    // Apply Local Transform
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1, 0, 0);
    glRotatef(rotation.y, 0, 1, 0);
    glRotatef(rotation.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);

    // Iterate Children
    for (auto* child : children) {
        Container* subContainer = dynamic_cast<Container*>(child);
        
        if (subContainer) {
            // Recurse into sub-containers
            subContainer->drawOpaqueChildren();
        } else {
            // Only draw if the object is opaque
            if (!child->isTransparent()) {
                child->draw();
            }
        }
    }
    glPopMatrix();
}

void Container::drawTransparentChildren() {
    glPushMatrix();
    
    // Apply Local Transform
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1, 0, 0);
    glRotatef(rotation.y, 0, 1, 0);
    glRotatef(rotation.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);

    // Iterate Children
    for (auto* child : children) {
        Container* subContainer = dynamic_cast<Container*>(child);
        
        if (subContainer) {
            // Recurse into sub-containers
            subContainer->drawTransparentChildren();
        } else {
            // Only draw if the object is transparent
            if (child->isTransparent()) {
                child->draw();
            }
        }
    }
    glPopMatrix();
}

GameObject* Container::clone() const {
    Container* newC = new Container();
    
    // Copy Properties
    newC->setPosition(position.x, position.y, position.z);
    newC->setRotation(rotation.x, rotation.y, rotation.z);
    newC->setScale(scale.x, scale.y, scale.z);
    newC->setMaterial(material);
    newC->castsShadow = castsShadow;

    // Deep Copy Children
    for (auto* child : children) {
        // Recursively clone the hierarchy
        newC->addChild(child->clone());
    }
    
    return newC;
}

bool Container::hasTransparentChildren() const {
    for (auto* child : children) {
        if (child->isTransparent()) return true;
        
        // If the child is also a container, check its children recursively
        Container* subContainer = dynamic_cast<Container*>(child);
        if (subContainer && subContainer->hasTransparentChildren()) return true;
    }
    return false;
}
