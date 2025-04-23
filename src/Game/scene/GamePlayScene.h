#pragma once

#include "UnoEngine.h"

class GamePlayScene : public IScene {
public:
    // Constructor/Destructor
    GamePlayScene();
    ~GamePlayScene() override;

    // IScene implementation
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // Player movement method
    void MovePlayer();

    // Camera update method
    void UpdateCamera();

    // Trail particle emitter update method
    void UpdateTrailEmitter();

private:
    // Initialization flag
    bool initialized_ = false;

    // Player character
    std::unique_ptr<Model> playerModel_;
    std::unique_ptr<Object3d> playerObject_;
    float playerSpeed_ = 0.5f; // Increased from 0.15f to 0.5f

    // Ground object
    std::unique_ptr<Model> groundModel_;
    std::unique_ptr<Object3d> groundObject_;

    // Jump parameters
    bool isJumping_ = false;
    float verticalVelocity_ = 0.0f;
    float jumpPower_ = 0.25f;    // Initial upward velocity on jump
    float gravity_ = 0.01f;      // Downward acceleration per frame

    // Particle emitters
    std::unique_ptr<ParticleEmitter> trailEmitter_;  // Trail behind player
    std::unique_ptr<ParticleEmitter> jumpEmitter_;   // Effect when jumping

    // UI elements
    std::unique_ptr<Sprite> uiSprite_;
    std::unique_ptr<Sprite> fovTextSprite_;  // FOV display text

    // Camera control
    float cameraRotationY_ = 0.0f;    // Horizontal rotation (yaw)
    float cameraRotationX_ = 0.0f;    // Vertical rotation (pitch)
    bool showCursor_ = false;         // Mouse cursor visibility flag

    // Mouse control
    int mouseDeltaX_ = 0;             // Mouse X movement since last frame
    int mouseDeltaY_ = 0;             // Mouse Y movement since last frame
    float mouseSensitivity_ = 0.003f; // Mouse sensitivity multiplier
    int screenCenterX_ = 0;           // Screen center X coordinate
    int screenCenterY_ = 0;           // Screen center Y coordinate

    // View mode
    bool isFPSMode_ = false;          // Toggle between TPS (false) and FPS (true)

    // FOV control
    float initialFovY_ = 0.45f;       // Initial FOV in radians (about 25 degrees)
    float currentFovY_ = 0.45f;       // Current FOV value
    float minFovY_ = 0.25f;           // Minimum FOV (~14 degrees)
    float maxFovY_ = 1.5f;            // Maximum FOV (~86 degrees)
    float fovChangeSpeed_ = 0.01f;    // FOV change amount per frame
};