#pragma once

#include "UnoEngine.h"
#include "PlayerController.h"

class CameraController {
public:
    // Constructor/Destructor
    CameraController();
    ~CameraController();

    // Core methods
    void Initialize(Camera* camera, Input* input);
    void Update();

    // Camera control
    void ProcessMouseInput();
    void UpdateCameraPosition();
    void UpdateFOV();

    // View mode control
    void ToggleViewMode() { isFPSMode_ = !isFPSMode_; }
    bool IsFPSMode() const { return isFPSMode_; }

    // Target settings
    void SetTargetPlayer(PlayerController* player) { targetPlayer_ = player; }

    // Accessors
    float GetRotationX() const { return rotationX_; }
    float GetRotationY() const { return rotationY_; }
    float GetCurrentFOV() const { return currentFovY_; }
    bool GetShowCursor() const { return showCursor_; }
    void SetShowCursor(bool show);

private:
    // Camera parameters
    float rotationX_ = 0.0f;     // Vertical rotation (pitch)
    float rotationY_ = 0.0f;     // Horizontal rotation (yaw)
    bool isFPSMode_ = false;     // First-person vs third-person mode
    bool showCursor_ = false;    // Mouse cursor visibility

    // Mouse control
    int mouseDeltaX_ = 0;             // Mouse X movement
    int mouseDeltaY_ = 0;             // Mouse Y movement
    float mouseSensitivity_ = 0.003f; // Mouse sensitivity
    int screenCenterX_ = 0;           // Screen center X coordinate
    int screenCenterY_ = 0;           // Screen center Y coordinate

    // FOV control
    float initialFovY_ = 0.45f;       // Initial FOV (radians)
    float currentFovY_ = 0.45f;       // Current FOV
    float minFovY_ = 0.25f;           // Minimum FOV
    float maxFovY_ = 1.5f;            // Maximum FOV
    float fovChangeSpeed_ = 0.01f;    // FOV change per frame

    // Dependencies
    Camera* camera_ = nullptr;
    Input* input_ = nullptr;
    PlayerController* targetPlayer_ = nullptr;
};