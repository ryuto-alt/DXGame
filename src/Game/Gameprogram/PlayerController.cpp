#include "PlayerController.h"
#include "ParticleEffectsManager.h"
#include "CollisionDetection.h"

PlayerController::PlayerController() {
    // Initialize default values in constructor
}

PlayerController::~PlayerController() {
    // Clean up resources if needed
}

void PlayerController::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Input* input) {
    // Store dependencies
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    input_ = input;

    // Initialize player model
    playerModel_ = std::make_unique<Model>();
    playerModel_->Initialize(dxCommon_);
    playerModel_->LoadFromObj("Resources/models", "player.obj");

    // Initialize player object
    playerObject_ = std::make_unique<Object3d>();
    playerObject_->Initialize(dxCommon_, spriteCommon_);
    playerObject_->SetModel(playerModel_.get());
    playerObject_->SetScale(scale_);
    playerObject_->SetPosition(position_);
    playerObject_->SetRotation(rotation_);

    // Debug output
    OutputDebugStringA("PlayerController: Successfully initialized\n");
}

void PlayerController::Update() {
    // Apply gravity to vertical movement
    ApplyGravity();

    // Handle collisions with the stage
    HandleCollisions();

    // Update the 3D object
    playerObject_->SetPosition(position_);
    playerObject_->SetRotation(rotation_);
    playerObject_->Update();
}

void PlayerController::Draw() {
    // Draw the player model
    if (playerObject_) {
        playerObject_->Draw();
    }
}

void PlayerController::Move(float cameraRotationY) {
    // Calculate forward and right vectors based on camera rotation
    float forwardX = sinf(cameraRotationY);
    float forwardZ = cosf(cameraRotationY);
    float rightX = cosf(cameraRotationY);
    float rightZ = -sinf(cameraRotationY);

    // Reset move vector
    moveVector_ = { 0.0f, 0.0f, 0.0f };
    isMoving_ = false;

    // Process keyboard input for movement
    if (input_->PushKey(DIK_W)) {
        moveVector_.x += forwardX * moveSpeed_;
        moveVector_.z += forwardZ * moveSpeed_;
        isMoving_ = true;
    }
    if (input_->PushKey(DIK_S)) {
        moveVector_.x -= forwardX * moveSpeed_;
        moveVector_.z -= forwardZ * moveSpeed_;
        isMoving_ = true;
    }
    if (input_->PushKey(DIK_A)) {
        moveVector_.x -= rightX * moveSpeed_;
        moveVector_.z -= rightZ * moveSpeed_;
        isMoving_ = true;
    }
    if (input_->PushKey(DIK_D)) {
        moveVector_.x += rightX * moveSpeed_;
        moveVector_.z += rightZ * moveSpeed_;
        isMoving_ = true;
    }

    // Check for jump input
    if (input_->TriggerKey(DIK_SPACE) && !isJumping_) {
        Jump();
    }

    // Store the original position before movement
    Vector3 originalPosition = position_;

    // Apply movement to position
    position_.x += moveVector_.x;
    position_.z += moveVector_.z;

    // Update player rotation to face movement direction if moving
    if (isMoving_) {
        // Calculate angle from movement direction
        float playerAngle = atan2f(moveVector_.x, moveVector_.z);
        rotation_.y = playerAngle;
    }
}

void PlayerController::Jump() {
    if (!isJumping_) {
        isJumping_ = true;
        verticalVelocity_ = jumpPower_;
    }
}

void PlayerController::ApplyGravity() {
    if (isJumping_) {
        // Apply gravity to vertical velocity
        verticalVelocity_ -= gravity_;

        // Update vertical position
        position_.y += verticalVelocity_;
    }
}

void PlayerController::SetPosition(const Vector3& position) {
    position_ = position;
    if (playerObject_) {
        playerObject_->SetPosition(position_);
    }
}

void PlayerController::SetStageModel(Model* stageModel) {
    stageModel_ = stageModel;

    // Extract collision boundaries from the stage model
    if (stageModel_) {
        CollisionDetection::ExtractStageBoundaries(stageModel_);
        OutputDebugStringA("PlayerController: Stage model set for collision detection\n");
    }
}

void PlayerController::HandleCollisions() {
    // Vector to store adjusted position
    Vector3 adjustedPosition;

    // Check for ground collision
    bool groundCollision = CollisionDetection::CheckGroundCollision(
        position_, playerRadius_, playerHeight_, adjustedPosition);

    // Apply ground collision adjustment
    if (groundCollision) {
        position_ = adjustedPosition;

        // Reset vertical velocity and jumping state if on ground
        if (isJumping_ && verticalVelocity_ <= 0.0f) {
            isJumping_ = false;
            verticalVelocity_ = 0.0f;
        }
    }

    // Check for obstacle collisions
    bool obstacleCollision = CollisionDetection::CheckObstacleCollision(
        position_, playerRadius_, adjustedPosition);

    // Apply obstacle collision adjustment
    if (obstacleCollision) {
        position_ = adjustedPosition;
    }

    // Ensure player stays within stage bounds
    const BoundingBox& stageBounds = CollisionDetection::GetStageBounds();

    // Apply stage boundary constraints
    if (position_.x - playerRadius_ < stageBounds.min.x) {
        position_.x = stageBounds.min.x + playerRadius_;
    }
    else if (position_.x + playerRadius_ > stageBounds.max.x) {
        position_.x = stageBounds.max.x - playerRadius_;
    }

    if (position_.z - playerRadius_ < stageBounds.min.z) {
        position_.z = stageBounds.min.z + playerRadius_;
    }
    else if (position_.z + playerRadius_ > stageBounds.max.z) {
        position_.z = stageBounds.max.z - playerRadius_;
    }

    // Prevent falling below the stage
    if (position_.y < stageBounds.min.y) {
        position_.y = stageBounds.min.y;
        isJumping_ = false;
        verticalVelocity_ = 0.0f;
    }
}