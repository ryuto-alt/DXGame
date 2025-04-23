#pragma once

#include "UnoEngine.h"

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

    // Accessors
    const Vector3& GetPosition() const { return position_; }
    void SetPosition(const Vector3& position);
    const Vector3& GetRotation() const { return rotation_; }
    bool IsMoving() const { return isMoving_; }
    bool IsJumping() const { return isJumping_; }
    Object3d* GetObject3d() const { return playerObject_.get(); }

    // Set dependencies
    void SetParticleEffects(ParticleEffectsManager* particleEffects) { particleEffects_ = particleEffects; }

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

    // Dependencies
    Input* input_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    ParticleEffectsManager* particleEffects_ = nullptr;
};