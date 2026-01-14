/**
 * @file GameObject.h
 * @brief Defines the base abstract class for all entities in the scene.
 *
 * This header contains the GameObject class, which provides core functionality
 * for positioning, rotation, scaling, hierarchy management (parenting), 
 * material properties, and update/interaction callbacks. It also defines
 * basic primitive shapes (Cube, Cylinder, Plane) and a utility CollisionBox.
 */

#pragma once
#include "Common.h"
#include <functional>

/**
 * @class GameObject
 * @brief The abstract base class for all 3D objects in the engine.
 *
 * GameObject handles the "Transform" (Position, Rotation, Scale), the material state,
 * and the scene graph hierarchy (parent-child relationships). It also supports
 * functional callbacks for per-frame updates and user interactions.
 */
class GameObject {
public:
    /** @brief Function signature for per-frame update logic. */
    using UpdateCallback = std::function<void(GameObject*, float)>;
    
    /** @brief Function signature for user interaction logic. */
    using InteractCallback = std::function<void(GameObject*)>;

protected:
    /** @brief Local position (X, Y, Z). */
    Vec3 position = { 0, 0, 0 };
    /** @brief Local rotation in Euler degrees (X, Y, Z). */
    Vec3 rotation = { 0, 0, 0 };
    /** @brief Local scale factors (X, Y, Z). */
    Vec3 scale = { 1, 1, 1 };
    
    /** @brief The material properties used for rendering. */
    Material material;

    /** @brief Pointer to the parent object (nullptr if this is a root object). */
    GameObject* parent = nullptr;

    /** @brief Optional callback executed during the update loop. */
    UpdateCallback updateAction = nullptr;
    
    /** @brief Optional callback executed when the object is interacted with. */
    InteractCallback interactAction = nullptr;

public:
    /** @brief Default constructor. */
    GameObject() {}
    
    /** @brief Virtual destructor. */
    virtual ~GameObject() {}

    /** @brief Sets the object's local position. */
    void setPosition(float x, float y, float z);
    
    /** @brief Sets the object's local rotation (in degrees). */
    void setRotation(float x, float y, float z);
    
    /** @brief Sets the object's local scale. */
    void setScale(float x, float y, float z);
    
    /** @brief Assigns a material to the object. */
    void setMaterial(const Material& m);
    
    /** * @brief Checks if the object uses a transparent material.
     * @return True if the material's alpha is < 1.0. 
     */
    bool isTransparent() const;

    /** @brief Flag indicating if this object should cast shadows (default: true). */
    bool castsShadow = true; 

    /** * @brief Sets the parent of this object.
     * @param p Pointer to the new parent GameObject.
     */
    void setParent(GameObject* p) { parent = p; }
    
    /** @brief Gets the current parent object. */
    GameObject* getParent() const { return parent; }

    /** @brief Gets the local position. */
    Vec3 getPosition() const { return position; }
    /** @brief Gets the local rotation. */
    Vec3 getRotation() const { return rotation; }
    /** @brief Gets the local scale. */
    Vec3 getScale() const { return scale; }

    /** * @brief Calculates the absolute world position.
     * * Recursively adds parent positions and applies parent transformations.
     * @return The global position in world space.
     */
    Vec3 getRealPosition() const;

    /** * @brief Calculates the absolute world rotation.
     * * Sums the local rotation with all ancestor rotations.
     * @return The global rotation in world space (Euler degrees).
     */
    Vec3 getRealRotation() const;

    /**
     * @brief Transforms a point from Local Space to World Space.
     * * Applies the object's Scale, Rotation, and Position, then recursively
     * applies parent transformations up to the root.
     * @param localPoint The point relative to the object's origin.
     * @return The point's coordinates in World Space.
     */
    Vec3 getPointInWorldSpace(Vec3 localPoint) const;

    /**
     * @brief Transforms a point from World Space to Local Space.
     * * Inverse operation of getPointInWorldSpace. useful for collision detection
     * or determining where something is relative to this object.
     * @param worldPoint The point in World Space.
     * @return The point's coordinates relative to this object's origin.
     */
    Vec3 getPointInLocalSpace(Vec3 worldPoint) const;

    /** * @brief Assigns a custom behavior to run every frame.
     * @param action A lambda or function matching UpdateCallback.
     */
    void setUpdateCallback(UpdateCallback action);
    
    /**
     * @brief Updates the object state. called once per frame.
     * * Executes the assigned updateAction if one exists.
     * @param deltaTime Time elapsed since the last frame (in seconds).
     */
    virtual void update(float deltaTime);

    /** * @brief Assigns a custom behavior for user interaction (e.g., clicking).
     * @param action A lambda or function matching InteractCallback.
     */
    void setInteractCallback(InteractCallback action);
    
    /**
     * @brief Triggers the interaction logic.
     * * If an InteractCallback is defined, it runs. Otherwise, the event
     * bubbles up to the parent object.
     */
    virtual void interact();

    /**
     * @brief Orbits the object around a specific pivot point in space.
     * * Updates both position and rotation to maintain the facing direction relative to the pivot.
     * * @param px Pivot X.
     * @param py Pivot Y.
     * @param pz Pivot Z.
     * @param ax Axis X component.
     * @param ay Axis Y component.
     * @param az Axis Z component.
     * @param angle Rotation angle in degrees.
     */
    void rotateAround(float px, float py, float pz, float ax, float ay, float az, float angle);

    /**
     * @brief Renders the object.
     * * Sets up the OpenGL matrix (translation, rotation, scaling) and material,
     * then calls drawMesh().
     */
    virtual void draw();
    
    /** * @brief Pure virtual method to draw the specific geometry.
     * * Derived classes must implement this to define their shape (e.g., glutSolidCube).
     */
    virtual void drawMesh() = 0; 
    
    /** * @brief Pure virtual method to clone the object.
     * * Derived classes must implement this to return a deep copy of themselves.
     */
    virtual GameObject* clone() const = 0; 
};

/** @brief A simple cube primitive. */
class Cube : public GameObject {
public:
    void drawMesh() override;
    GameObject* clone() const override { return new Cube(*this); }
};

/** @brief A simple cylinder primitive. */
class Cylinder : public GameObject {
public:
    void drawMesh() override;
    GameObject* clone() const override { return new Cylinder(*this); }
};

/** @brief A flat plane primitive, typically used for floors. */
class Plane : public GameObject {
public:
    void drawMesh() override;
    GameObject* clone() const override { return new Plane(*this); }
};

/** * @brief An invisible box used for collision or triggers.
 * * It is only visible when debug flags are enabled.
 */
class CollisionBox : public GameObject {
public:
    float width, height, depth;
    
    /**
     * @brief Constructs a CollisionBox with specified dimensions.
     * @param w Width (X-axis).
     * @param h Height (Y-axis).
     * @param d Depth (Z-axis).
     */
    CollisionBox(float w, float h, float d);
    
    void drawMesh() override;
    GameObject* clone() const override { return new CollisionBox(*this); }
};
