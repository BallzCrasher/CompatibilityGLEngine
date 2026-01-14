/**
 * @file Container.h
 * @brief Defines the Container class for managing hierarchies of GameObjects.
 *
 * This header contains the Container class, which implements the Composite design pattern.
 * It allows multiple GameObjects to be grouped together, transformed relative to a parent,
 * and managed as a single entity.
 */

#pragma once
#include "GameObject.h"
#include <vector>

/**
 * @class Container
 * @brief A composite GameObject that holds and manages a collection of child objects.
 *
 * The Container class is responsible for propagating updates, rendering calls, and 
 * transformations to its children. It is essential for constructing complex scenes
 * or compound objects (e.g., a car consisting of a chassis and four wheels).
 */
class Container : public GameObject {
private:
    /** @brief The list of child objects managed by this container. */
    std::vector<GameObject*> children;

public:
    /** @brief Default constructor. */
    Container();

    /** * @brief Destructor.
     * * Deletes all child objects to prevent memory leaks.
     */
    ~Container(); 

    /**
     * @brief Adds a child object to the container.
     * * The container takes ownership of the child. The child's parent pointer
     * is updated to point to this container.
     * * @param child Pointer to the GameObject to add.
     */
    void addChild(GameObject* child);

    /**
     * @brief Renders the container and all its children.
     * * Applies the container's local transformation matrix and then iterates
     * through children to draw them.
     */
    void draw() override;

    /**
     * @brief Overrides the mesh drawing method.
     * * Containers generally do not have a mesh of their own, so this implementation
     * is empty.
     */
    void drawMesh() override {} 

    /**
     * @brief Creates a deep copy of the container.
     * * Recursively clones the container and all its children.
     * @return A pointer to the new Container instance.
     */
	GameObject* clone() const override;

    /**
     * @brief Checks if any child (or descendant) is transparent.
     * @return True if a transparent object exists in the hierarchy, false otherwise.
     */
    bool hasTransparentChildren() const;

    /**
     * @brief Updates the container and propagates the update to all children.
     * @param deltaTime Time elapsed since the last frame.
     */
    void update(float deltaTime) override;

    /**
     * @brief Retrieves the list of children.
     * @return A constant reference to the vector of child pointers.
     */
    const std::vector<GameObject*>& getChildren() const { return children; }

    /**
     * @brief Renders only the opaque children in the hierarchy.
     * * Helper method for multi-pass rendering. Recursively calls itself for
     * child Containers and calls draw() on non-transparent leaf nodes.
     */
    void drawOpaqueChildren();

    /**
     * @brief Renders only the transparent children in the hierarchy.
     * * Helper method for multi-pass rendering. Recursively calls itself for
     * child Containers and calls draw() on transparent leaf nodes.
     */
    void drawTransparentChildren();
};
