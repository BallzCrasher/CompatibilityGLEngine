/**
 * @file main.cpp
 * @brief Entry point and scene management for the OpenGL engine.
 *
 * This file contains the main application loop, scene initialization, input handling,
 * physics integration, and rendering pipeline (Opaque -> Shadows -> Transparent).
 * It constructs the game world using the GameObject and Container systems.
 */

#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <vector>

#include "Common.h"
#include "GameObject.h"
#include "Lighting.h"
#include "Camera.h"
#include "Container.h"
#include "Model.h"
#include "Text3D.h"

// --- GLOBAL ENGINE STATE ---

/** @brief List of all renderable objects in the scene. */
std::vector<GameObject*> objects;

/** @brief List of objects that possess collision properties. */
std::vector<GameObject*> physicsObjects;

/** @brief List of active point lights. */
std::vector<PointLight> pointLights;

/** @brief The global directional light (Sun). */
DirectionalLight sun;

/** @brief The player camera. */
Camera camera;

/** @brief Array to track the state of keyboard keys (pressed/released). */
bool keys[256];

// --- TIME ---
/** @brief Timestamp of the previous frame for delta time calculation. */
int lastTime = 0; 

// --- MOUSE VARIABLES ---
int windowWidth = 800;
int windowHeight = 600;
float virtualMouseX = 400.0f; 
float virtualMouseY = 300.0f; 
bool isWarping = false;       

// --- PHYSICS GLOBALS ---
float playerVelocityY = 0.0f;
const float GRAVITY = 9.8f;
const float PLAYER_HEIGHT = 1.5f; /**< Eye level from feet. */
const float PLAYER_RADIUS = 0.3f; /**< Collision radius of the player. */

/**
 * @brief Helper to build a procedural glass wall with pillars.
 *
 * Generates a Container holding alternating pillars and glass panes.
 *
 * @param length Approximate total length of the wall.
 * @param height Height of the pillars and glass.
 * @param paneWidth The width of a single glass panel.
 * @param pillarThickness The width/depth of the pillars.
 * @return A pointer to the constructed Container.
 */
Container* createGlassWall(float length, float height, float paneWidth, float pillarThickness = 0.2f) {
    Container* wall = new Container();

    Material matGlass = Material::CreateGlass();
    Material matPillar = Material::CreateChrome(); 

    // Calculate Layout
    // Pattern: Pillar -> Glass -> Pillar -> Glass -> Pillar
    // Solve for N (number of panes): 
    // Total = N*paneWidth + (N+1)*pillarThickness
    int numPanes = (int)((length - pillarThickness) / (paneWidth + pillarThickness));
    if (numPanes < 1) numPanes = 1; // Ensure at least one section

    // Recalculate exact total length to center it perfectly
    float totalActualLength = (numPanes * paneWidth) + ((numPanes + 1) * pillarThickness);
    float startX = -totalActualLength / 2.0f;
    float currentX = startX;
    float glassDepth = 0.05f; 

    // Build the Wall
    for (int i = 0; i <= numPanes; i++) {
        // A. Create Pillar
        Cube* pillar = new Cube();
        pillar->setScale(pillarThickness, height, pillarThickness);
        // Position is relative to the Container center. 
        // Y is moved up by height/2 so the base sits on y=0
        pillar->setPosition(currentX + pillarThickness / 2.0f, height / 2.0f, 0.0f);
        pillar->setMaterial(matPillar);
        wall->addChild(pillar);

        currentX += pillarThickness;

        // B. Create Glass Pane (between pillars)
        if (i < numPanes) {
            Cube* glass = new Cube();
            glass->setScale(paneWidth, height, glassDepth);
            glass->setPosition(currentX + paneWidth / 2.0f, height / 2.0f, 0.0f);
            glass->setMaterial(matGlass);
			glass->castsShadow = false;
            wall->addChild(glass);
            
            currentX += paneWidth;
        }
    }

    return wall;
}

/**
 * @brief Helper to create a glass table with metal legs.
 * @param width Table width.
 * @param height Table height.
 * @param depth Table depth.
 * @return A pointer to the constructed Container.
 */
Container* createGlassTable(float width, float height, float depth) {
    Container* table = new Container();

    // 1. Define Materials
    // Metal: Dark grey with high specular shine
    Material matMetal;
    matMetal.ambient[0] = 0.2f; matMetal.ambient[1] = 0.2f; matMetal.ambient[2] = 0.2f; matMetal.ambient[3] = 1.0f;
    matMetal.diffuse[0] = 0.3f; matMetal.diffuse[1] = 0.3f; matMetal.diffuse[2] = 0.35f; matMetal.diffuse[3] = 1.0f;
    matMetal.specular[0] = 0.9f; matMetal.specular[1] = 0.9f; matMetal.specular[2] = 0.9f; matMetal.specular[3] = 1.0f;
    matMetal.shininess = 60.0f;

    Material matGlass = Material::CreateGlass();

    float legThick = 0.1f;
    float frameThick = 0.1f; 
    float glassThick = 0.05f;

    // 2. Create Legs (4 Corners)
    float legX = width / 2.0f - legThick / 2.0f;
    float legZ = depth / 2.0f - legThick / 2.0f;
    
    for (int xDir = -1; xDir <= 1; xDir += 2) {
        for (int zDir = -1; zDir <= 1; zDir += 2) {
            Cube* leg = new Cube();
            leg->setScale(legThick, height, legThick);
            // Legs sit on the floor (y=0), so their center is at height/2
            leg->setPosition(xDir * legX, height / 2.0f, zDir * legZ);
            leg->setMaterial(matMetal);
            table->addChild(leg);
        }
    }

    // 3. Create Metal Frame
    
    // Long Bars (Front/Back)
    float longBarLen = width;
    Cube* barFront = new Cube();
    barFront->setScale(longBarLen, frameThick, legThick);
    barFront->setPosition(0.0f, height - frameThick/2.0f, legZ); 
    barFront->setMaterial(matMetal);
    table->addChild(barFront);

    Cube* barBack = dynamic_cast<Cube*>(barFront->clone());
    barBack->setPosition(0.0f, height - frameThick/2.0f, -legZ);
    table->addChild(barBack);

    // Short Bars (Left/Right)
    float shortBarLen = depth - (2 * legThick); 
    Cube* barLeft = new Cube();
    barLeft->setScale(legThick, frameThick, shortBarLen);
    barLeft->setPosition(-legX, height - frameThick/2.0f, 0.0f);
    barLeft->setMaterial(matMetal);
    table->addChild(barLeft);

    Cube* barRight = dynamic_cast<Cube*>(barLeft->clone());
    barRight->setPosition(legX, height - frameThick/2.0f, 0.0f);
    table->addChild(barRight);

    // 4. Create Glass Top
    Cube* glass = new Cube();
    glass->setScale(width - legThick, glassThick, depth - legThick);
    glass->setPosition(0.0f, height - frameThick/2.0f, 0.0f);
    glass->setMaterial(matGlass);
    glass->castsShadow = false; 
    table->addChild(glass);

    // 5. Collision Box
    CollisionBox* collider = new CollisionBox(width, height, depth);
    collider->setPosition(0.0f, height / 2.0f, 0.0f);
    table->addChild(collider);
	physicsObjects.push_back(collider);

    return table;
}

/**
 * @brief Helper to create a detailed modern chair.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return A pointer to the constructed Container.
 */
Container* createModernChair(float r, float g, float b) {
    Container* chair = new Container();

    // 1. Refined Material (Less Bright)
    float darkR = r * 0.6f;
    float darkG = g * 0.6f;
    float darkB = b * 0.6f;

    Material matPlastic;
    matPlastic.ambient[0] = darkR * 0.4f; matPlastic.ambient[1] = darkG * 0.4f; matPlastic.ambient[2] = darkB * 0.4f; matPlastic.ambient[3] = 1.0f;
    matPlastic.diffuse[0] = darkR;        matPlastic.diffuse[1] = darkG;        matPlastic.diffuse[2] = darkB;        matPlastic.diffuse[3] = 1.0f;
    matPlastic.specular[0] = 0.3f;        matPlastic.specular[1] = 0.3f;        matPlastic.specular[2] = 0.3f;        matPlastic.specular[3] = 1.0f;
    matPlastic.shininess = 20.0f;

    // Dimensions
    float seatHeight = 0.45f;
    float seatWidth = 0.45f;
    float seatDepth = 0.45f;
    float legThick = 0.04f;
    
    // 2. Legs
    float legOffset = seatWidth / 2.0f - legThick; 
    
    for (int xDir = -1; xDir <= 1; xDir += 2) {
        for (int zDir = -1; zDir <= 1; zDir += 2) {
            Cube* leg = new Cube();
            leg->setScale(legThick, seatHeight, legThick);
            leg->setPosition(xDir * legOffset, seatHeight / 2.0f, zDir * legOffset);
            leg->setMaterial(matPlastic);
            chair->addChild(leg);
        }
    }

    // 3. Seat
    Cube* seat = new Cube();
    seat->setScale(seatWidth, 0.06f, seatDepth);
    seat->setPosition(0.0f, seatHeight, 0.0f);
    seat->setMaterial(matPlastic);
    chair->addChild(seat);

    // 4. Angled Backrest Container
    Container* backrestGroup = new Container();
    
    // Pivot point for the backrest is at the back of the seat
    backrestGroup->setPosition(0.0f, seatHeight, -seatDepth / 2.0f + 0.05f);
    backrestGroup->setRotation(-15.0f, 0.0f, 0.0f); 

    // A. Vertical Supports
    float supportHeight = 0.5f;
    for (int xDir = -1; xDir <= 1; xDir += 2) {
        Cube* support = new Cube();
        support->setScale(legThick, supportHeight, legThick);
        support->setPosition(xDir * legOffset, supportHeight / 2.0f, 0.0f);
        support->setMaterial(matPlastic);
        backrestGroup->addChild(support);
    }

    // B. Horizontal Slats
    float slatCount = 3;
    float startY = 0.2f; 
    float gap = (supportHeight - startY) / slatCount;

    for (int i = 0; i < slatCount; i++) {
        Cube* slat = new Cube();
        slat->setScale(seatWidth, 0.03f, 0.02f);
        // Position slats up the support
        slat->setPosition(0.0f, startY + (i * gap) + 0.05f, 0.0f);
        slat->setMaterial(matPlastic);
        backrestGroup->addChild(slat);
    }

    chair->addChild(backrestGroup);

    // 5. Collision Box
    CollisionBox* box = new CollisionBox(seatWidth, seatHeight + supportHeight, seatDepth);
    box->setPosition(0.0f, (seatHeight + supportHeight) / 2.0f, 0.0f);
    chair->addChild(box);
	physicsObjects.push_back(box);

    return chair;
}

/**
 * @brief Renders all opaque objects in the scene.
 * * This pass is typically performed first.
 */
void drawOpaqueObjects() {
    for (auto* obj : objects) {
        Container* container = dynamic_cast<Container*>(obj);
        
        if (container) {
            container->drawOpaqueChildren();
        } else {
            if (!obj->isTransparent()) {
                obj->draw();
            }
        }
    }
}

/**
 * @brief Renders all transparent objects in the scene.
 * * Performs a two-pass rendering (back faces then front faces) and handles
 * depth masking to ensure proper alpha blending.
 */
void drawTransparentObjects() {
    // Disable writing to the depth buffer so glass can layer correctly
    glDepthMask(GL_FALSE); 

    // 1. Back Faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); 
    
    for (auto* obj : objects) {
        Container* container = dynamic_cast<Container*>(obj);
        
        if (container) {
            container->drawTransparentChildren();
        } else {
            if (obj->isTransparent()) {
                obj->draw();
            }
        }
    }

    // 2. Front Faces
    glCullFace(GL_BACK);
    
    for (auto* obj : objects) {
        Container* container = dynamic_cast<Container*>(obj);

        if (container) {
            container->drawTransparentChildren();
        } else {
            if (obj->isTransparent()) {
                obj->draw();
            }
        }
    }

    glDisable(GL_CULL_FACE);
    
    // Re-enable depth writing
    glDepthMask(GL_TRUE); 
}

/**
 * @brief Constructs a planar shadow projection matrix.
 * @param fMatrix Output 4x4 matrix.
 * @param fLightPos Position of the light source (x, y, z, w).
 * @param fPlane Plane equation (Ax + By + Cz + D = 0).
 */
void buildShadowMatrix(float fMatrix[16], float fLightPos[4], float fPlane[4]) {
    float dot = fPlane[0] * fLightPos[0] + fPlane[1] * fLightPos[1] +
                fPlane[2] * fLightPos[2] + fPlane[3] * fLightPos[3];

    fMatrix[0] = dot - fLightPos[0] * fPlane[0];
    fMatrix[4] = 0.0f - fLightPos[0] * fPlane[1];
    fMatrix[8] = 0.0f - fLightPos[0] * fPlane[2];
    fMatrix[12] = 0.0f - fLightPos[0] * fPlane[3];

    fMatrix[1] = 0.0f - fLightPos[1] * fPlane[0];
    fMatrix[5] = dot - fLightPos[1] * fPlane[1];
    fMatrix[9] = 0.0f - fLightPos[1] * fPlane[2];
    fMatrix[13] = 0.0f - fLightPos[1] * fPlane[3];

    fMatrix[2] = 0.0f - fLightPos[2] * fPlane[0];
    fMatrix[6] = 0.0f - fLightPos[2] * fPlane[1];
    fMatrix[10] = dot - fLightPos[2] * fPlane[2];
    fMatrix[14] = 0.0f - fLightPos[2] * fPlane[3];

    fMatrix[3] = 0.0f - fLightPos[3] * fPlane[0];
    fMatrix[7] = 0.0f - fLightPos[3] * fPlane[1];
    fMatrix[11] = 0.0f - fLightPos[3] * fPlane[2];
    fMatrix[15] = dot - fLightPos[3] * fPlane[3];
}

/**
 * @brief Main rendering loop.
 * * Handles the 3-pass rendering strategy:
 * 1. Opaque objects.
 * 2. Shadows (flattened geometry).
 * 3. Transparent objects.
 */
void display() {
    // 1. CLEAR BUFFERS
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.updateLook();
    
    // Update lights
    sun.enable();
    for (auto& l : pointLights) {
		l.enable();
	}

    // PASS 1: OPAQUE WORLD
    drawOpaqueObjects();

    // PASS 2: SHADOWS
    GLfloat lightPos[] = { 1.0f, 1.0f, 1.0f, 0.0f }; 
    GLfloat groundPlane[] = { 0.0f, 1.0f, 0.0f, 0.0f }; 
    GLfloat shadowMat[16];
    buildShadowMatrix(shadowMat, lightPos, groundPlane);

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE); 
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f); 

    glPushMatrix();
    glMultMatrixf(shadowMat);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

    for (auto* obj : objects) {
        if (!obj->isTransparent() && obj->castsShadow) {
            obj->draw();
        }
    }

    glPopMatrix();
    
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE); 
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);

    // PASS 3: TRANSPARENT WORLD
    drawTransparentObjects();

    glutSwapBuffers();
}

// --- PHYSICS ENGINE ---

/**
 * @brief Checks if a point overlaps with a CollisionBox.
 * * Uses local space transformation to handle Oriented Bounding Box (OBB) collision.
 * @param box The collision box to check against.
 * @param px Point X.
 * @param py Point Y.
 * @param pz Point Z.
 * @return True if the point (inflated by player radius) intersects the box.
 */
bool isOverlap(CollisionBox* box, float px, float py, float pz) {
    // 1. Transform World Point to Local Space
    Vec3 localPos = box->getPointInLocalSpace({ px, py, pz });

    // 2. Get Local Bounds
    float hw = box->width / 2.0f;
    float hh = box->height / 2.0f;
    float hd = box->depth / 2.0f;

    // 3. Scale Player Dimensions to Local Space
    Vec3 s = box->getScale();
    float sx = (std::abs(s.x) > 0.001f) ? (1.0f / s.x) : 1.0f;
    float sy = (std::abs(s.y) > 0.001f) ? (1.0f / s.y) : 1.0f;
    float sz = (std::abs(s.z) > 0.001f) ? (1.0f / s.z) : 1.0f;

    float localRadius = PLAYER_RADIUS * std::max(std::abs(sx), std::abs(sz)); 
    float localHeight = PLAYER_HEIGHT * std::abs(sy);

    // 4. Check Overlaps
    if (localPos.x < (-hw - localRadius) || localPos.x > (hw + localRadius)) return false;
    if (localPos.z < (-hd - localRadius) || localPos.z > (hd + localRadius)) return false;
    if ((localPos.y - localHeight) < hh && localPos.y > -hh) return true;

    return false;
}

/**
 * @brief Recursively checks the scene graph for collisions.
 */
bool checkSceneCollision(GameObject* node, float px, float py, float pz) {
    CollisionBox* box = dynamic_cast<CollisionBox*>(node);
    if (box) {
        if (isOverlap(box, px, py, pz)) return true;
    }

    Container* container = dynamic_cast<Container*>(node);
    if (container) {
        for (auto* child : container->getChildren()) {
            if (checkSceneCollision(child, px, py, pz)) return true;
        }
    }

    return false;
}

/**
 * @brief Global collision check against all physics objects.
 */
bool isColliding(float px, float py, float pz) {
    for (auto* obj : physicsObjects) {
        if (checkSceneCollision(obj, px, py, pz)) return true;
    }
    return false;
}

/**
 * @brief Main update loop.
 * * Handles physics (gravity, collision), inputs, and object updates.
 */
void update() {
    // 1. Calculate Delta Time
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    // 2. PHYSICS: Gravity
    playerVelocityY -= GRAVITY * deltaTime;
    
    // Apply Vertical Movement
    camera.y += playerVelocityY * deltaTime;

    // Ground Check
    if (camera.y < PLAYER_HEIGHT) {
        camera.y = PLAYER_HEIGHT;
        playerVelocityY = 0.0f;
    }

    // Vertical Collision with Boxes
    if (isColliding(camera.x, camera.y, camera.z)) {
        if (playerVelocityY < 0) {
            camera.y -= playerVelocityY * deltaTime;
            playerVelocityY = 0.0f;
        } else {
             // Hit head
             camera.y -= playerVelocityY * deltaTime;
             playerVelocityY = 0.0f;
        }
    }


    // 3. MOVEMENT INPUT
    float dx = 0.0f;
    float dz = 0.0f;
    
    if (keys['w']) dz -= 1.0f;
    if (keys['s']) dz += 1.0f;
    if (keys['a']) dx -= 1.0f;
    if (keys['d']) dx += 1.0f;

    if (dx != 0 || dz != 0) {
        float rad = camera.yaw * M_PI / 180.0f;
        
        float inputX = 0; 
        float inputZ = 0;
        if (keys['w']) inputX += 1.0f; 
        if (keys['s']) inputX -= 1.0f;
        if (keys['a']) inputZ -= 1.0f;
        if (keys['d']) inputZ += 1.0f;
        
        float effectiveDX = inputX * camera.speed; 
        float effectiveDZ = inputZ * camera.speed;

        float cosR = cos(rad);
        float sinR = sin(rad);

        float deltaX = (effectiveDX * cosR - effectiveDZ * sinR);
        float deltaZ = (effectiveDX * sinR + effectiveDZ * cosR);

        // Try Move X
        float nextX = camera.x + deltaX;
        if (!isColliding(nextX, camera.y, camera.z)) {
            camera.x = nextX;
        }

        // Try Move Z
        float nextZ = camera.z + deltaZ;
        if (!isColliding(camera.x, camera.y, nextZ)) {
            camera.z = nextZ;
        }
    }
    
    if (keys[' ']) {
         playerVelocityY = 5.0f; 
    }

    // 4. Update Game Objects
    for (auto* obj : objects) {
        obj->update(deltaTime);
    }

    glutPostRedisplay();
}


/**
 * @brief Performs raycasting to find the closest interactable object.
 * @param obj The current node in the scene graph.
 * @param closestDist Reference to the current closest hit distance.
 * @param closestObj Reference to the current closest object pointer.
 * @param camX Ray origin X.
 * @param camY Ray origin Y.
 * @param camZ Ray origin Z.
 * @param dirX Ray direction X.
 * @param dirY Ray direction Y.
 * @param dirZ Ray direction Z.
 */
void findClosestObject(GameObject* obj, float& closestDist, GameObject*& closestObj, 
                       float camX, float camY, float camZ, 
                       float dirX, float dirY, float dirZ) {
    
    Container* container = dynamic_cast<Container*>(obj);

    if (container) {
        // Recurse into children
        for (auto* child : container->getChildren()) {
            findClosestObject(child, closestDist, closestObj, 
                              camX, camY, camZ, 
                              dirX, dirY, dirZ);
        }
    } else {
        // Perform Raycast Hit Check
        Vec3 pos = obj->getRealPosition();
        float toObjX = pos.x - camX;
        float toObjY = pos.y - camY;
        float toObjZ = pos.z - camZ;

        float t = toObjX * dirX + toObjY * dirY + toObjZ * dirZ;

        if (t > 0 && t < closestDist) {
            float rayPointX = camX + t * dirX;
            float rayPointY = camY + t * dirY;
            float rayPointZ = camZ + t * dirZ;

            float distSq = pow(pos.x - rayPointX, 2) + 
                           pow(pos.y - rayPointY, 2) + 
                           pow(pos.z - rayPointZ, 2);

            // Radius check (1.5 units)
            if (distSq < 1.5f * 1.5f) {
                closestDist = t;
                closestObj = obj;
            }
        }
    }
}

/**
 * @brief Triggers interaction logic for the object looked at.
 */
void checkInteraction() {
    float radYaw = camera.yaw * M_PI / 180.0f;
    float radPitch = camera.pitch * M_PI / 180.0f;
    
    float dirX = cos(radPitch) * cos(radYaw);
    float dirY = sin(radPitch);
    float dirZ = cos(radPitch) * sin(radYaw);
    
    float len = sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    dirX /= len; dirY /= len; dirZ /= len;

    GameObject* closestObj = nullptr;
    float closestDist = 10.0f; 

    for (auto* obj : objects) {
        findClosestObject(obj, closestDist, closestObj, 
                          camera.x, camera.y, camera.z, 
                          dirX, dirY, dirZ);
    }

    if (closestObj) {
        closestObj->interact();
    }
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;
    if (key == 27) exit(0); 

    if (key == 'e' || key == 'E') {
        checkInteraction();
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void mouseMotion(int x, int y) {
    if (isWarping) {
        isWarping = false;
        return;
    }

    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    int dx = x - centerX;
    int dy = y - centerY;

    virtualMouseX += dx; 
    virtualMouseY += dy;

    camera.mouseMove((int)virtualMouseX, (int)virtualMouseY);

    if (dx != 0 || dy != 0) {
        isWarping = true; 
        glutWarpPointer(centerX, centerY);
    }
}

/**
 * @brief Initializes the scene, materials, lights, and objects.
 */
void init() {
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE); 
	glHint(GL_FOG_HINT, GL_NICEST);

    // 1. ATMOSPHERE
    glClearColor(0.02f, 0.02f, 0.1f, 1.0f); 

    // 2. FOG
    glEnable(GL_FOG);
    GLfloat fogColor[] = { 0.02f, 0.02f, 0.1f, 1.0f }; 
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogf(GL_FOG_DENSITY, 0.03f); 

    // 3. COOL AMBIENT
    GLfloat globalAmbient[] = { 0.1f, 0.1f, 0.25f, 1.0f }; 
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    
    // 4. Floor
    Plane* floor = new Plane();
    floor->setPosition(0, 0, 0);
    floor->setScale(100, 1, 100);
    floor->castsShadow = false;
    
    Material matFloor;
    matFloor.ambient[0] = 0.2f; matFloor.ambient[1] = 0.2f; matFloor.ambient[2] = 0.2f; matFloor.ambient[3] = 1.0f;
    matFloor.diffuse[0] = 0.5f; matFloor.diffuse[1] = 0.5f; matFloor.diffuse[2] = 0.5f; matFloor.diffuse[3] = 1.0f;
    matFloor.specular[0] = 0.0f; matFloor.specular[1] = 0.0f; matFloor.specular[2] = 0.0f; matFloor.specular[3] = 1.0f;
    matFloor.shininess = 0.0f;
    
    floor->setMaterial(matFloor);
    objects.push_back(floor);

	Container* building = new Container();
	{
		// Right Wall
		Container* wall1 = createGlassWall(20.0f, 5.0f, 1.5f);
		wall1->setPosition(9.5f, 0.0f, -10.0f);
		wall1->setRotation(0, -90, 0); 
		{
			Cube* hpiller1 = new Cube();
			hpiller1->setPosition(0.0f, 2.5f, 0.0f);
			hpiller1->setScale(19.3f, 0.6f, 0.5f);
			wall1->addChild(hpiller1);

			Cube* neonLight1 = new Cube();
			neonLight1->setPosition(0.0f, 2.5f, 0.25f);
			neonLight1->setScale(19.3f, 0.1f, 0.1f);
			neonLight1->setMaterial(Material::CreateNeon(1, 1, 1));
			wall1->addChild(neonLight1);
			
			CollisionBox* wall1CollisionBox = new CollisionBox(20.0f, 5.0f, 0.5f);
			wall1CollisionBox->setPosition(0, 2.5, 0);
			wall1->addChild(wall1CollisionBox);
			physicsObjects.push_back(wall1CollisionBox);
		}
		building->addChild(wall1);

		// Left Wall
		Container* wall2 = createGlassWall(20.0f, 5.0f, 1.5f);
		wall2->setPosition(-9.5f, 0.0f, -10.0f);
		wall2->setRotation(0, -90, 0); 
		{
			Cube* hpiller2 = new Cube();
			hpiller2->setPosition(0.0f, 2.5f, 0.0f);
			hpiller2->setScale(19.3f, 0.6f, 0.5f);
			wall2->addChild(hpiller2);

			Cube* neonLight2 = new Cube();
			neonLight2->setPosition(0.0f, 2.5f, -0.25f);
			neonLight2->setScale(19.3f, 0.1f, 0.1f);
			neonLight2->setMaterial(Material::CreateNeon(1, 1, 1));
			wall2->addChild(neonLight2);

			CollisionBox* wall2CollisionBox = new CollisionBox(20.0f, 5.0f, 0.5f);
			wall2CollisionBox->setPosition(0, 2.5, 0);
			wall2->addChild(wall2CollisionBox);
			physicsObjects.push_back(wall2CollisionBox);
		}
		building->addChild(wall2);

		// Back Wall
		Container* wall3 = createGlassWall(20.0f, 5.0f, 1.5f);
		wall3->setPosition(0.0f, 0.0f, -19.5f);
		wall3->setRotation(0, 0, 0); 
		{
			Cube* hpiller3 = new Cube();
			hpiller3->setPosition(0.0f, 2.5f, 0.0f);
			hpiller3->setScale(19.3f, 0.6f, 0.5f);
			wall3->addChild(hpiller3);

			Cube* neonLight3 = new Cube();
			neonLight3->setPosition(0.0f, 2.5f, 0.25f);
			neonLight3->setScale(19.3f, 0.1f, 0.1f);
			neonLight3->setMaterial(Material::CreateNeon(1, 1, 1));
			wall3->addChild(neonLight3);

			CollisionBox* wall3CollisionBox = new CollisionBox(20.0f, 5.0f, 0.5f);
			wall3CollisionBox->setPosition(0, 2.5, 0);
			wall3->addChild(wall3CollisionBox);
			physicsObjects.push_back(wall3CollisionBox);
		}

		building->addChild(wall3);

		// Front
		Container* frontwall = new Container();
		{
			// Front Left
			Container* wall4 = createGlassWall(8.0f, 5.0f, 1.5f);
			wall4->setPosition(-6.0f, 0.0f, -0.5f);
			wall4->setRotation(0, 0, 0); 
			frontwall->addChild(wall4);

			// Front Right
			Container* wall5 = createGlassWall(8.0f, 5.0f, 1.5f);
			wall5->setPosition(+6.0f, 0.0f, -0.5f);
			wall5->setRotation(0, 0, 0); 
			frontwall->addChild(wall5);

			Cube* hpiller5 = new Cube();
			hpiller5->setPosition(-6.0f, 2.5f, 0.0f);
			hpiller5->setScale(19.3f, 0.6f, 0.5f);
			wall5->addChild(hpiller5);

			Container* sign = new Container();
			{
				Cube* signBody = new Cube();
				signBody->setScale(5.0f, 2.0f, 0.05f);
				signBody->setMaterial(Material::CreateGlass());
				sign->addChild(signBody);

				Container* signText = new Container();
				{
					Text3D* neonSign1 = new Text3D("CAR");
					neonSign1->setMaterial(Material::CreateNeon(1.0f, 0.0f, 0.0f));
					neonSign1->setScale(0.6, 0.6, 0.6);
					neonSign1->setPosition(0.0f, 0.5, 0.0f);
					signText->addChild(neonSign1);

					Text3D* neonSign2 = new Text3D("SHOWROOM");
					neonSign2->setScale(0.6, 0.6, 0.6);
					neonSign2->setPosition(0.0f, -0.4, 0.0f);
					neonSign2->setMaterial(Material::CreateNeon(1.0f, 0.0f, 0.0f));
					signText->addChild(neonSign2);
				}
				sign->addChild(signText);

				sign->setPosition(-6.0f, 3.8f, 0.0f);
			}
			wall5->addChild(sign);

			Container* door = new Container();
			{
				Container* rightCorner = new Container();
				{
					Cube* glassCorner = new Cube();
					glassCorner->setScale(1, 2.1, 0.1);
					glassCorner->setMaterial(Material::CreateGlass());
					rightCorner->addChild(glassCorner);

					Cube* opaqueCorner = new Cube();
					opaqueCorner->setScale(0.1, 2.1, 0.2);
					opaqueCorner->setPosition(-0.5, 0, 0);
					rightCorner->addChild(opaqueCorner);

				}
				door->addChild(rightCorner);

				Container* leftCorner = dynamic_cast<Container*>(rightCorner->clone());
				leftCorner->setScale(-1, 1, 1);
				leftCorner->setPosition(-4, 0, 0);
				door->addChild(leftCorner);


				Container* rightDoor = new Container();
				{
					Cube* glass = new Cube();
					glass->setScale(1.5, 2.1, 0.1);
					glass->setMaterial(Material::CreateGlass());
					rightDoor->addChild(glass);

					Cube* knob = new Cube();
					knob->setScale(0.05, 0.7, 0.2);
					knob->setPosition(-0.5, -0.02, 0);
					rightDoor->addChild(knob);

					CollisionBox* rightDoorCollision = new CollisionBox(1.5, 2.1, 0.1);
					physicsObjects.push_back(rightDoorCollision);
					rightDoor->addChild(rightDoorCollision);

					rightDoor->setPosition(-1.3, 0, 0);
					rightDoor->setUpdateCallback([](GameObject* obj, float dt) {
						obj->rotateAround(-0.6, 0, 0, 0.0f, -1.0f, 0.0f, 1.0f);
						return;
					});

					// ---------------------------------------------------------
					// RIGHT DOOR INTERACTION
					// ---------------------------------------------------------

					// Shared State for the Right Door
					static float currentAngleR = 0.0f;
					static float targetAngleR = 0.0f;

					// UPDATE CALLBACK (Animation)
					rightDoor->setUpdateCallback([](GameObject* obj, float dt) {
						float speed = 120.0f; 
						float diff = targetAngleR - currentAngleR;

						if (std::abs(diff) < 1.0f) {
							if (currentAngleR != targetAngleR) {
								float finalStep = targetAngleR - currentAngleR;
								// Hinge Pivot: Center is -1.3. Width is 1.5. 
								// Right Edge (Hinge) is -1.3 + 0.75 = -0.55
								obj->rotateAround(-0.55f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, finalStep);
								currentAngleR = targetAngleR;
							}
							return;
						}

						float step = speed * dt;
						if (step > std::abs(diff)) step = std::abs(diff);
						if (diff < 0) step = -step;

						// Rotate around Right Hinge (-0.55)
						obj->rotateAround(-0.55f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, step);
						currentAngleR += step;
					});

					// INTERACT CALLBACK (Trigger)
					rightDoor->setInteractCallback([](GameObject* obj) {
						if (std::abs(targetAngleR) > 1.0f) {
							// Close the door
							targetAngleR = 0.0f;
						} 
						else {
							// OPEN THE DOOR
							Vec3 doorPos = obj->getRealPosition();
							float dx = camera.x - doorPos.x;
							float dz = camera.z - doorPos.z;

							Vec3 rot = obj->getRealRotation();
							float rad = rot.y * 3.14159f / 180.0f;
							float fwdX = std::sin(rad); 
							float fwdZ = std::cos(rad);

							float dot = dx * fwdX + dz * fwdZ;

							// INVERTED LOGIC compared to Left Door:
							targetAngleR = (dot > 0) ? -90.0f : 90.0f;
						}
					});

				}
				door->addChild(rightDoor);

				Container* leftDoor = new Container();
				{
					Cube* glass = new Cube();
					glass->setScale(1.5, 2.1, 0.1);
					glass->setMaterial(Material::CreateGlass());
					leftDoor->addChild(glass);

					Cube* knob = new Cube();
					knob->setScale(0.05, 0.7, 0.2);
					knob->setPosition(0.5, -0.02, 0);
					leftDoor->addChild(knob);

					leftDoor->setPosition(-2.8, 0, 0);

					// Shared State for this specific door
					static float currentAngle = 0.0f;
					static float targetAngle = 0.0f;

					CollisionBox* leftDoorCollision = new CollisionBox(1.5, 2.1, 0.1);
					leftDoor->addChild(leftDoorCollision);
					physicsObjects.push_back(leftDoorCollision);


					// THE UPDATE CALLBACK (Animation Logic)
					leftDoor->setUpdateCallback([](GameObject* obj, float dt) {
						float speed = 120.0f; 
						float diff = targetAngle - currentAngle;

						if (std::abs(diff) < 1.0f) {
							if (currentAngle != targetAngle) {
								float finalStep = targetAngle - currentAngle;
								// Hinge Pivot: Door is at -2.8, Width is 1.5. Left Edge is -2.8 - 0.75 = -3.55
								obj->rotateAround(-3.55f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, finalStep);
								currentAngle = targetAngle;
							}
							return;
						}

						float step = speed * dt;
						if (step > std::abs(diff)) step = std::abs(diff);
						if (diff < 0) step = -step;

						// Apply Rotation
						obj->rotateAround(-3.55f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, step);

						// Update state
						currentAngle += step;
					});

					// THE INTERACT CALLBACK (Trigger Logic)
					leftDoor->setInteractCallback([](GameObject* obj) {
						if (std::abs(targetAngle) > 1.0f) {
							targetAngle = 0.0f;
						} 
						else {
							Vec3 doorPos = obj->getRealPosition();
							float dx = camera.x - doorPos.x;
							float dz = camera.z - doorPos.z;

							Vec3 rot = obj->getRealRotation();
							float rad = rot.y * 3.14159f / 180.0f;
							float fwdX = std::sin(rad); 
							float fwdZ = std::cos(rad);

							float dot = dx * fwdX + dz * fwdZ;

							targetAngle = (dot > 0) ? 90.0f : -90.0f;
						}
					});
				}
				door->addChild(leftDoor);
				door->setScale(1, 0.95, 1);
				door->setPosition(-4, 1.2, 0);
			}
			wall5->addChild(door);

			CollisionBox* frontwallCollisionBoxLeft = new CollisionBox(8.0f, 5.0f, 0.5f);
			frontwallCollisionBoxLeft->setPosition(-5.5, 2.5, -0.5);
			frontwall->addChild(frontwallCollisionBoxLeft);
			physicsObjects.push_back(frontwallCollisionBoxLeft);

			CollisionBox* frontwallCollisionBoxRight = new CollisionBox(8.0f, 5.0f, 0.5f);
			frontwallCollisionBoxRight->setPosition(5.5, 2.5, -0.5);
			frontwall->addChild(frontwallCollisionBoxRight);
			physicsObjects.push_back(frontwallCollisionBoxRight);

			CollisionBox* frontwallCollisionBoxTop = new CollisionBox(3.0f, 2.45f, 0.5f);
			frontwallCollisionBoxTop->setPosition(0, 3.35, -0.5);
			frontwall->addChild(frontwallCollisionBoxTop);
			physicsObjects.push_back(frontwallCollisionBoxTop);
		}
		building->addChild(frontwall);

		Container* roof = new Container();
		{
			Cube* roofCube = new Cube();
			roofCube->setScale(20.0, 0.4f, 20.0f);
			roof->addChild(roofCube);

			Cube* cubeNeon = new Cube();
			cubeNeon->setScale(3, 0.1, 15);
			cubeNeon->setPosition(0, -0.2, 0);
			cubeNeon->setMaterial(Material::CreateNeon(1, 1, 1));
			roof->addChild(cubeNeon);

			float rfactor = 0.2;
			Cube* cubeOpaque = new Cube();
			cubeOpaque->setScale(3 - rfactor, 0.1, 15 - rfactor);
			cubeOpaque->setPosition(0, -0.25, 0);
			roof->addChild(cubeOpaque);

			Cube* cubeMirror = new Cube();
			cubeMirror->setScale(2.5, 0.2, 2.2);
			cubeMirror->setPosition(0, -0.3, 6);
			cubeMirror->setMaterial(Material::CreateGlass());
			roof->addChild(cubeMirror);

			for (int i = 0; i < 4; i++) {
				Cube* cubeMirrorClone = dynamic_cast<Cube*>(cubeMirror->clone());
				cubeMirror->setPosition(0, -0.3, 6 - i * 4);
				roof->addChild(cubeMirrorClone);
			}

			CollisionBox* roofCollisionBox = new CollisionBox(20.0f, 1.0f, 20.0f);
			roof->addChild(roofCollisionBox);
			physicsObjects.push_back(roofCollisionBox);

			roof->setPosition(0.0, 5.0f, -10.0f);
		}
		building->addChild(roof);

		Container* base = new Container();
		{
			Cube* baseFloor = new Cube();
			baseFloor->setPosition(0.0, 0.0f, -10.0f);
			baseFloor->setScale(20.0, 0.4f, 20.0f);
			base->addChild(baseFloor);

			for (int i = 0; i < 5; i++) {
				Cube* mirrorPane = new Cube();
				mirrorPane->setScale(2.0, 0.05f, 2.0f);
				mirrorPane->setPosition(0.0, 0.2f, -2 - 4 * i);
				mirrorPane->setMaterial(Material::CreateGlass());
				base->addChild(mirrorPane);
			}

			Cube* mirrorPane1 = new Cube();
			mirrorPane1->setScale(2.0, 0.05f, 2.0f);
			mirrorPane1->setPosition(5, 0.2f, -6.5);
			mirrorPane1->setRotation(0, 45, 0);
			mirrorPane1->setMaterial(Material::CreateGlass());
			base->addChild(mirrorPane1);

			Cube* mirrorPane2 = new Cube();
			mirrorPane2->setScale(2.0, 0.05f, 2.0f);
			mirrorPane2->setPosition(-5, 0.2f, -6.5);
			mirrorPane2->setRotation(0, 45, 0);
			mirrorPane2->setMaterial(Material::CreateGlass());
			base->addChild(mirrorPane2);

			Cube* mirrorPane3 = new Cube();
			mirrorPane3->setScale(2.0, 0.05f, 2.0f);
			mirrorPane3->setPosition(5, 0.2f, -13);
			mirrorPane3->setRotation(0, 45, 0);
			mirrorPane3->setMaterial(Material::CreateGlass());
			base->addChild(mirrorPane3);
		}
		building->addChild(base);

		// Inside

		Container* inwall1 = createGlassWall(8.0f, 5.0f, 1.5f);
		inwall1->setPosition(2.8f, 0.0f, -4.0f);
		inwall1->setRotation(0, -90, 0); 
		CollisionBox* inwall1CollisionBox = new CollisionBox(7.0f, 5.0f, 0.5f);
		inwall1CollisionBox->setPosition(0.0f, 2.5f, 0.0f);
		inwall1->addChild(inwall1CollisionBox);
		physicsObjects.push_back(inwall1CollisionBox);
		building->addChild(inwall1);
		
		Container* inwall2 = createGlassWall(8.0f, 5.0f, 1.5f);
		inwall2->setPosition(-2.8f, 0.0f, -4.0f);
		inwall2->setRotation(0, -90, 0); 
		CollisionBox* inwall2CollisionBox = new CollisionBox(7.0f, 5.0f, 0.5f);
		inwall2CollisionBox->setPosition(0.0f, 2.5f, 0.0f);
		inwall2->addChild(inwall2CollisionBox);
		physicsObjects.push_back(inwall2CollisionBox);
		building->addChild(inwall2);

		Container* inwall3 = createGlassWall(8.0f, 5.0f, 1.5f);
		inwall3->setPosition(-2.8f, 0.0f, -16.0f);
		inwall3->setRotation(0, -90, 0); 
		CollisionBox* inwall3CollisionBox = new CollisionBox(7.0f, 5.0f, 0.5f);
		inwall3CollisionBox->setPosition(0.0f, 2.5f, 0.0f);
		inwall3->addChild(inwall3CollisionBox);
		physicsObjects.push_back(inwall3CollisionBox);
		building->addChild(inwall3);

		Container* inwall4 = createGlassWall(8.0f, 5.0f, 1.5f);
		inwall4->setPosition(2.8f, 0.0f, -16.0f);
		inwall4->setRotation(0, -90, 0); 
		CollisionBox* inwall4CollisionBox = new CollisionBox(7.0f, 5.0f, 0.5f);
		inwall4CollisionBox->setPosition(0.0f, 2.5f, 0.0f);
		inwall4->addChild(inwall4CollisionBox);
		physicsObjects.push_back(inwall4CollisionBox);
		building->addChild(inwall4);

		Container* inwall5 = createGlassWall(8.0f, 5.0f, 1.5f);
		inwall5->setPosition(6.f, 0.0f, -10.0f);
		CollisionBox* inwall5CollisionBox = new CollisionBox(7.0f, 5.0f, 0.5f);
		inwall5CollisionBox->setPosition(0.0f, 2.5f, 0.0f);
		inwall5->addChild(inwall5CollisionBox);
		physicsObjects.push_back(inwall5CollisionBox);
		building->addChild(inwall5);

		Container* inwall6 = createGlassWall(8.0f, 5.0f, 1.5f);
		inwall6->setPosition(-6.f, 0.0f, -10.0f);
		CollisionBox* inwall6CollisionBox = new CollisionBox(7.0f, 5.0f, 0.5f);
		inwall6CollisionBox->setPosition(0.0f, 2.5f, 0.0f);
		inwall6->addChild(inwall6CollisionBox);
		physicsObjects.push_back(inwall6CollisionBox);
		building->addChild(inwall6);

		building->setPosition(0.0f, 0.0f, -5.0f);


		// Room 1
		Container* pod1 = new Container();
		{
			Cylinder* basePod = new Cylinder();
			basePod->setRotation(-90, 0, 0);
			basePod->setScale(3, 3, 0.3);
			basePod->setMaterial(Material::CreateGold());
			pod1->addChild(basePod);

			Cylinder* mainPod = dynamic_cast<Cylinder*>(basePod->clone());
			mainPod->setScale(2.8, 2.8, 0.3);
			mainPod->setPosition(0.0, 0.02, 0.0);
			mainPod->setMaterial(Material::CreateGlass()); 
			pod1->addChild(mainPod);

			pod1->setScale(1.5, 1, 1.5);
			pod1->setPosition(-7.2, 0, -2.9);
		}
		building->addChild(pod1);

		// Room 2
		Container* pod2 = dynamic_cast<Container*>(pod1->clone());
		pod2->setPosition(+7.2, 0, -2.9);
		building->addChild(pod2);

		// Room 3
		Container* pod3 = dynamic_cast<Container*>(pod1->clone());
		pod3->setPosition(+7.1, 0, -17.2);
		building->addChild(pod3);

		// Room 4
		Container* pod4 = dynamic_cast<Container*>(pod1->clone());
		pod4->setPosition(-7.1, 0, -17.2);
		building->addChild(pod4);


	}
	objects.push_back(building);

	// =======  ROOM 3 ====== 
	Container* glassTable =  createGlassTable(2, 0.8, 1);
	glassTable->setPosition(8, 0, -16);
	glassTable->setRotation(0, 0, 0);
	objects.push_back(glassTable);

    // 1. Red Chair
    Container* redChair = createModernChair(0.6f, 0.1f, 0.1f);
    redChair->setPosition(7.0f, 0.0f, -17.0f);
    redChair->setRotation(0.0f, 45.0f, 0.0f); 
    objects.push_back(redChair);

    // 2. Blue Chair
    Container* blueChair = createModernChair(1.0f, 1.0f, 0.6f);
    blueChair->setPosition(8.0f, 0.0f, -17.0f);
    objects.push_back(blueChair);

    // 3. Green Chair
    Container* greenChair = createModernChair(0.1f, 0.6f, 0.1f);
    greenChair->setPosition(9.0f, 0.0f, -17.0f);
    objects.push_back(greenChair);

    // Chair 1
    Container* chair1Container = new Container();
    chair1Container->setPosition(-8.5f, 0.2f, -14.0f);
    chair1Container->setRotation(0.0f, 135.0f, 0.0f);
    {
        Model* chair1 = new Model("../Models/chair/scene.gltf");
        chair1->setScale(0.01f, 0.01f, 0.01f);
        chair1->castsShadow = false;
        chair1Container->addChild(chair1);

        CollisionBox* box = new CollisionBox(0.6f, 1.0f, 0.6f);
        box->setPosition(0.0f, 0.5f, 0.0f);
        chair1Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(chair1Container);

    // Chair 2
    Container* chair2Container = new Container();
    chair2Container->setPosition(-7.0f, 0.2f, -14.0f);
    chair2Container->setRotation(0.0f, 45.0f, 0.0f);
    {
        Model* chair2 = new Model("../Models/chair/scene.gltf"); 
        chair2->setScale(0.01f, 0.01f, 0.01f);
        chair2Container->addChild(chair2);

        CollisionBox* box = new CollisionBox(0.6f, 1.0f, 0.6f);
        box->setPosition(0.0f, 0.5f, 0.0f);
        chair2Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(chair2Container);

    // Table (Wooden)
    Container* tableContainer = new Container();
    tableContainer->setPosition(-7.7f, 0.55f, -12.9f);
    {
        Model* table = new Model("../Models/table/scene.gltf");
        table->setScale(0.7f, 0.7f, 0.7f);
        table->castsShadow = false;
        tableContainer->addChild(table);

        CollisionBox* box = new CollisionBox(1.5f, 0.8f, 1.5f);
        box->setPosition(0.0f, 0.0f, 0.0f); 
        tableContainer->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(tableContainer);

    // Sofa 1
    Container* sofa1Container = new Container();
    sofa1Container->setPosition(7.1f, 0.2f, -14.2f);
    sofa1Container->setRotation(0.0f, 90.0f, 0.0f);
    {
        Model* sofa1 = new Model("../Models/sofa/scene.gltf");
        sofa1->setScale(0.01f, 0.01f, 0.01f);
        sofa1->castsShadow = false;
        sofa1Container->addChild(sofa1);

        CollisionBox* box = new CollisionBox(1.0f, 1.0f, 2.2f);
        box->setPosition(0.0f, 0.5f, 0.0f);
        sofa1Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(sofa1Container);

    // Sofa 2
    Container* sofa2Container = new Container();
    sofa2Container->setPosition(8.7f, 0.2f, -12.8f);
    {
        Model* sofa2 = new Model("../Models/sofa/scene.gltf");
        sofa2->setScale(0.01f, 0.01f, 0.01f);
        sofa2Container->addChild(sofa2);

        CollisionBox* box = new CollisionBox(1.0f, 1.0f, 2.2f);
        box->setPosition(0.0f, 0.5f, 0.0f);
        sofa2Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(sofa2Container);

    // Tesla Car
    Container* teslaContainer = new Container();
    teslaContainer->setPosition(-7.15f, 0.3f, -8.36f);
    {
        Model* tesla = new Model("../Models/tesla-car/scene.gltf");
        tesla->setScale(0.7f, 0.7f, 0.7f);
        teslaContainer->addChild(tesla);

        // Add Collision Box
        CollisionBox* box = new CollisionBox(2.0f, 1.5f, 4.5f);
        box->setPosition(0.0f, 0.75f, 0.0f);
        teslaContainer->addChild(box);
        physicsObjects.push_back(box);

        teslaContainer->setUpdateCallback([](GameObject* obj, float dt) {
            float speed = 20.0f;
            Vec3 p = obj->getPosition();
            obj->rotateAround(p.x, p.y, p.z, 0.0f, 1.0f, 0.0f, speed * dt);
        });
    }
    objects.push_back(teslaContainer);

    // Low Poly Car
    Container* lowPolyCarContainer = new Container();
    lowPolyCarContainer->setPosition(7.04f, 0.29f, -7.88f);
    {
        Model* lowPolyCar = new Model("../Models/low_poly_car/scene.gltf");
        lowPolyCar->setScale(0.005f, 0.005f, 0.005f);
        lowPolyCarContainer->addChild(lowPolyCar);

        // Add Collision Box
        CollisionBox* box = new CollisionBox(2.0f, 1.5f, 4.0f);
        box->setPosition(0.0f, 0.75f, 0.0f);
        lowPolyCarContainer->addChild(box);
        physicsObjects.push_back(box);

        lowPolyCarContainer->setUpdateCallback([](GameObject* obj, float dt) {
            float speed = 20.0f;
            Vec3 p = obj->getPosition();
            obj->rotateAround(p.x, p.y, p.z, 0.0f, 1.0f, 0.0f, speed * dt);
        });
    }
    objects.push_back(lowPolyCarContainer);

    // Corvette
    Container* corvetteContainer = new Container();
    corvetteContainer->setPosition(7.12f, 0.3f, -22.21f);
    {
        Model* corvette = new Model("../Models/corvette/scene.gltf");
        corvette->setScale(0.9f, 0.9f, 0.9f);
        corvetteContainer->addChild(corvette);

        // Add Collision Box
        CollisionBox* box = new CollisionBox(2.0f, 1.4f, 4.5f);
        box->setPosition(0.0f, 0.7f, 0.0f);
        corvetteContainer->addChild(box);
        physicsObjects.push_back(box);

        corvetteContainer->setUpdateCallback([](GameObject* obj, float dt) {
            float speed = 20.0f;
            Vec3 p = obj->getPosition();
            obj->rotateAround(p.x, p.y, p.z, 0.0f, 1.0f, 0.0f, speed * dt);
        });
    }
    objects.push_back(corvetteContainer);

    // Rocks (Group 1)
    Container* rocks1Container = new Container();
    rocks1Container->setPosition(1.8f, 0.7f, -9.0f);
    rocks1Container->setRotation(0.0f, 70.0f, 0.0f);
    {
        Model* rocks1 = new Model("../Models/rocks/scene.gltf");
        rocks1Container->addChild(rocks1);

        CollisionBox* box = new CollisionBox(1.5f, 1.0f, 1.5f);
        rocks1Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(rocks1Container);

    // Rocks (Group 2)
    Container* rocks2Container = new Container();
    rocks2Container->setPosition(-1.8f, 0.7f, -9.0f);
    rocks2Container->setRotation(0.0f, -110.0f, 0.0f);
    {
        Model* rocks2 = new Model("../Models/rocks/scene.gltf");
        rocks2Container->addChild(rocks2);

        CollisionBox* box = new CollisionBox(1.5f, 1.0f, 1.5f);
        rocks2Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(rocks2Container);

    // Rocks (Group 3)
    Container* rocks3Container = new Container();
    rocks3Container->setPosition(1.8f, 0.7f, -21.0f);
    rocks3Container->setRotation(0.0f, 70.0f, 0.0f);
    {
        Model* rocks3 = new Model("../Models/rocks/scene.gltf");
        rocks3Container->addChild(rocks3);

        CollisionBox* box = new CollisionBox(1.5f, 1.0f, 1.5f);
        rocks3Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(rocks3Container);

    // Rocks (Group 4)
    Container* rocks4Container = new Container();
    rocks4Container->setPosition(-1.8f, 0.7f, -21.0f);
    rocks4Container->setRotation(0.0f, 70.0f, 0.0f);
    {
        Model* rocks4 = new Model("../Models/rocks/scene.gltf");
        rocks4Container->addChild(rocks4);

        CollisionBox* box = new CollisionBox(1.5f, 1.0f, 1.5f);
        rocks4Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(rocks4Container);

    // Plant 1
    Container* plant1Container = new Container();
    plant1Container->setPosition(-3.6f, 0.19f, -6.0f);
    {
        Model* plant1 = new Model("../Models/plant1/scene.gltf");
        plant1->setScale(0.2f, 0.2f, 0.2f);
        plant1Container->addChild(plant1);

        CollisionBox* box = new CollisionBox(0.5f, 1.5f, 0.5f);
        box->setPosition(0.0f, 0.75f, 0.0f);
        plant1Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(plant1Container);

    // Plant 2
    Container* plant2Container = new Container();
    plant2Container->setPosition(3.6f, 0.19f, -6.0f);
    plant2Container->setRotation(0.0f, 160.0f, 0.0f);
    {
        Model* plant2 = new Model("../Models/plant1/scene.gltf");
        plant2->setScale(0.2f, 0.2f, 0.2f);
        plant2Container->addChild(plant2);

        CollisionBox* box = new CollisionBox(0.5f, 1.5f, 0.5f);
        box->setPosition(0.0f, 0.75f, 0.0f);
        plant2Container->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(plant2Container);

    // Coffee Table (Table 2)
    Container* coffeeTableContainer = new Container();
    coffeeTableContainer->setPosition(7.0f, 0.5f, -13.0f);
    {
        Model* table2 = new Model("../Models/coffee_table/scene.gltf");
        coffeeTableContainer->addChild(table2);

        CollisionBox* box = new CollisionBox(1.2f, 0.6f, 1.2f);
        box->setPosition(0.0f, 0.0f, 0.0f);
        coffeeTableContainer->addChild(box);
        physicsObjects.push_back(box);
    }
    objects.push_back(coffeeTableContainer);

    // Lights
	// Light inside the Tesla area (Cyan/Blue "Tech" feel)
    pointLights.push_back(PointLight(2, -7.15, 2.0, -8.36,   0.2f, 0.8f, 1.0f, 1.5f));   
    
    // Light inside the Low Poly Car area (Purple "Synthwave" feel)
    pointLights.push_back(PointLight(3, 7.04, 2.0, -7.88,    0.8f, 0.2f, 1.0f, 1.5f));   
    
    // Light near the Red Neon Sign (Red "Hot" feel)
    pointLights.push_back(PointLight(4, 7.12, 3.0, -22.21,   1.0f, 0.1f, 0.1f, 2.0f));   
    
    // Back fill light (Soft White)
    pointLights.push_back(PointLight(5, -6.96, 3, -22.02,    0.8f, 0.8f, 0.9f, 0.5f));   
}

/**
 * @brief Handles window resize events.
 * @param w New width.
 * @param h New height.
 */
void reshape(int w, int h) {
    if (h == 0) h = 1;
    
    windowWidth = w;
    windowHeight = h;

    float ratio = w * 1.0 / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief Application entry point.
 * * Initializes GLUT, configures the window, sets up callbacks, and enters the main loop.
 */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Engine");
	glutFullScreen();

    init();

    glutDisplayFunc(display);
    glutIdleFunc(update);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);

	// MOUSE FIXES
    glutPassiveMotionFunc(mouseMotion); 
    glutMotionFunc(mouseMotion);        
    
    glutSetCursor(GLUT_CURSOR_NONE);    
    
    glutWarpPointer(400, 300);

    glutMainLoop();
    return 0;
}
