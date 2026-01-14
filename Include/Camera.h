/**
 * @file Camera.h
 * @brief Defines the Camera class for handling 3D navigation and view control.
 *
 * This header contains the Camera class, which manages position, orientation (yaw/pitch),
 * and movement parameters. It provides methods to update the OpenGL view matrix
 * and handle mouse input for looking around.
 */

#pragma once

/**
 * @class Camera
 * @brief A simple First-Person Shooter (FPS) style camera.
 *
 * The Camera class encapsulates the state required for a free-roaming 3D camera.
 * It maintains the viewer's position in world space and their viewing angles
 * to calculate the appropriate look-at vector.
 */
class Camera {
public:
    /** @brief X coordinate of the camera's position. Default is 0.0. */
    float x = 0.0f;
    /** @brief Y coordinate of the camera's position. Default is 1.0. */
    float y = 1.0f;
    /** @brief Z coordinate of the camera's position. Default is 5.0. */
    float z = 5.0f;

    /** @brief Horizontal rotation angle (in degrees). Default is -90.0 (facing negative Z). */
    float yaw = -90.0f;
    /** @brief Vertical rotation angle (in degrees). Default is 0.0 (level horizon). */
    float pitch = 0.0f;

    /** @brief Movement speed factor for keyboard navigation. */
    float speed = 0.1f;
    /** @brief Mouse sensitivity factor for look rotation. */
    float sensitivity = 0.1f;

    /**
     * @brief Calculates the look vector and updates the OpenGL view matrix.
     *
     * This function uses the current yaw, pitch, and position to compute the
     * target point the camera is looking at, then calls `gluLookAt` to transform
     * the scene.
     */
    void updateLook();

    /**
     * @brief Processes mouse movement to update camera orientation.
     *
     * Calculates the change in mouse position since the last frame and modifies
     * the yaw and pitch accordingly.
     *
     * @param mx The current X-coordinate of the mouse cursor.
     * @param my The current Y-coordinate of the mouse cursor.
     */
    void mouseMove(int mx, int my);
};
