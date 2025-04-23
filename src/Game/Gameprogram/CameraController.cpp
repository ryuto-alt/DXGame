#include "CameraController.h"

CameraController::CameraController() {
    // Initialize default values in constructor
}

CameraController::~CameraController() {
    // Cleanup resources if needed
}

void CameraController::Initialize(Camera* camera, Input* input) {
    // Store dependencies
    camera_ = camera;
    input_ = input;

    // Set initial camera properties
    camera_->SetFovY(initialFovY_);
    currentFovY_ = initialFovY_;

    // Calculate screen center for mouse control
    screenCenterX_ = WinApp::kClientWidth / 2;
    screenCenterY_ = WinApp::kClientHeight / 2;

    // Initialize cursor state
    SetShowCursor(showCursor_);

    // Debug output
    OutputDebugStringA("CameraController: Successfully initialized\n");
}

void CameraController::Update() {
    // Process mouse input for camera rotation
    ProcessMouseInput();

    // Update camera position based on target and view mode
    UpdateCameraPosition();

    // Process FOV changes
    UpdateFOV();

    // Check for view mode toggle
    if (input_->TriggerKey(DIK_1)) {
        ToggleViewMode();
    }

    // Check for cursor visibility toggle
    if (input_->TriggerKey(DIK_TAB)) {
        SetShowCursor(!showCursor_);
    }

    // Apply all changes to camera
    camera_->SetRotate({ rotationX_, rotationY_, 0.0f });
    camera_->Update();
}

void CameraController::ProcessMouseInput() {
    if (showCursor_) {
        return; // Skip mouse camera control when cursor is visible
    }

    // Get mouse state
    DIMOUSESTATE mouseState = {};
    input_->GetMouseState(&mouseState);

    // Update camera rotation based on mouse movement
    mouseDeltaX_ = mouseState.lX;
    mouseDeltaY_ = mouseState.lY;

    rotationY_ += mouseDeltaX_ * mouseSensitivity_;
    rotationX_ += mouseDeltaY_ * mouseSensitivity_;

    // Clamp vertical rotation to prevent flipping
    if (rotationX_ > 1.5f) {
        rotationX_ = 1.5f;
    }
    else if (rotationX_ < -1.5f) {
        rotationX_ = -1.5f;
    }
}

void CameraController::UpdateCameraPosition() {
    if (!targetPlayer_) {
        return; // No target to follow
    }

    Vector3 targetPos = targetPlayer_->GetPosition();
    Vector3 cameraPos;

    if (isFPSMode_) {
        // First-person view: camera at eye level
        cameraPos = targetPos;
        cameraPos.y += 1.7f; // Eye height
    }
    else {
        // Third-person view: camera at distance from player
        float distance = 50.0f;

        // Calculate position using spherical coordinates
        cameraPos.x = targetPos.x - sinf(rotationY_) * cosf(rotationX_) * distance;
        cameraPos.y = targetPos.y + sinf(rotationX_) * distance;
        cameraPos.z = targetPos.z - cosf(rotationY_) * cosf(rotationX_) * distance;
    }

    // Update camera position
    camera_->SetTranslate(cameraPos);
}

void CameraController::UpdateFOV() {
    // Increase FOV with F key
    if (input_->PushKey(DIK_F)) {
        currentFovY_ += fovChangeSpeed_;
        if (currentFovY_ > maxFovY_) {
            currentFovY_ = maxFovY_;
        }
        camera_->SetFovY(currentFovY_);
    }

    // Decrease FOV with G key
    if (input_->PushKey(DIK_G)) {
        currentFovY_ -= fovChangeSpeed_;
        if (currentFovY_ < minFovY_) {
            currentFovY_ = minFovY_;
        }
        camera_->SetFovY(currentFovY_);
    }
}

void CameraController::SetShowCursor(bool show) {
    showCursor_ = show;
    input_->SetMouseCursor(showCursor_);
}