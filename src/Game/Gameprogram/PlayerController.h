#pragma once

#include "UnoEngine.h"
#include "CollisionDetection.h" // Add the collision detection header

// Forward declarations
class ParticleEffectsManager;

class PlayerController {
public:
    // Constructor/Destructor
    PlayerController();
    ~PlayerController();

    // Core methods
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Input* input);
    void Update();
    void Draw();

    // Movement related methods
    void Move(float cameraRotationY);
    void Jump();
    void ApplyGravity();

    // Collision handling methods
    void HandleCollisions(); // New method to handle collisions

    // Accessors
    const Vector3& GetPosition() const { return position_; }
    void SetPosition(const Vector3& position);
    const Vector3& GetRotation() const { return rotation_; }
    bool IsMoving() const { return isMoving_; }
    bool IsJumping() const { return isJumping_; }
    Object3d* GetObject3d() const { return playerObject_.get(); }

    // Player properties
    float GetRadius() const { return playerRadius_; }
    float GetHeight() const { return playerHeight_; }

    // Set dependencies
    void SetParticleEffects(ParticleEffectsManager* particleEffects) { particleEffects_ = particleEffects; }

    // Set collision detection with stage model
    void SetStageModel(Model* stageModel);

private:
    // 3D model and object
    std::unique_ptr<Model> playerModel_;
    std::unique_ptr<Object3d> playerObject_;

    // Movement parameters
    Vector3 position_ = { 0.0f, 0.0f, 0.0f };
    Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
    Vector3 scale_ = { 1.0f, 1.0f, 1.0f };
    float moveSpeed_ = 0.5f;
    Vector3 moveVector_ = { 0.0f, 0.0f, 0.0f };
    bool isMoving_ = false;

    // Jump parameters
    bool isJumping_ = false;
    float verticalVelocity_ = 0.0f;
    float jumpPower_ = 0.45f;
    float gravity_ = 0.01f;

    // Collision parameters
    float playerRadius_ = 0.5f; // Radius of the player's collision cylinder
    float playerHeight_ = 2.0f; // Height of the player

    // Dependencies
    Input* input_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    ParticleEffectsManager* particleEffects_ = nullptr;
    Model* stageModel_ = nullptr; // Reference to the stage model for collision
};