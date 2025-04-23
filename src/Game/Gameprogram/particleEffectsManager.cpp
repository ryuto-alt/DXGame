#include "ParticleEffectsManager.h"

ParticleEffectsManager::ParticleEffectsManager() {
    // Initialize particle manager reference
    particleManager_ = ParticleManager::GetInstance();
}

ParticleEffectsManager::~ParticleEffectsManager() {
    // No special cleanup needed, unique_ptr handles memory
}

void ParticleEffectsManager::Initialize() {
    // Create particle groups
    particleManager_->CreateParticleGroup("playerTrail", "Resources/particle/smoke.png");
    particleManager_->CreateParticleGroup("jumpEffect", "Resources/particle/smoke.png");

    // Initialize trail particle emitter
    trailEmitter_ = std::make_unique<ParticleEmitter>(
        "playerTrail",                       // Group name
        Vector3(0.0f, 0.5f, 0.0f),          // Initial position
        3,                                  // Emit count
        15.0f,                              // Emit rate
        Vector3(-0.1f, 0.05f, -0.1f),       // Min velocity
        Vector3(0.1f, 0.2f, 0.1f),          // Max velocity
        Vector3(0.0f, 0.0f, 0.0f),          // Min acceleration
        Vector3(0.0f, 0.1f, 0.0f),          // Max acceleration
        0.2f,                               // Min start size
        0.4f,                               // Max start size
        0.0f,                               // Min end size
        0.0f,                               // Max end size
        Vector4(0.8f, 0.8f, 1.0f, 0.8f),    // Min start color (light blue)
        Vector4(1.0f, 1.0f, 1.0f, 1.0f),    // Max start color (white)
        Vector4(0.5f, 0.5f, 0.8f, 0.0f),    // Min end color (transparent blue)
        Vector4(0.7f, 0.7f, 1.0f, 0.0f),    // Max end color
        0.0f,                               // Min rotation
        6.28f,                              // Max rotation (full circle)
        -1.0f,                              // Min rotation velocity
        1.0f,                               // Max rotation velocity
        0.3f,                               // Min lifetime
        0.8f                                // Max lifetime
    );

    // Initialize jump effect emitter (but don't emit until needed)
    jumpEmitter_ = std::make_unique<ParticleEmitter>(
        "jumpEffect",
        Vector3(0.0f, 0.0f, 0.0f),
        10,                                  // More particles for jump
        1.0f,                               // Lower rate (one-time burst)
        Vector3(-0.3f, 0.1f, -0.3f),        // Min velocity
        Vector3(0.3f, 0.5f, 0.3f),          // Max velocity
        Vector3(0.0f, -0.01f, 0.0f),        // Min acceleration
        Vector3(0.0f, -0.03f, 0.0f),        // Max acceleration
        0.3f,                               // Min start size
        0.6f,                               // Max start size
        0.0f,                               // Min end size
        0.1f,                               // Max end size
        Vector4(0.9f, 0.9f, 1.0f, 0.9f),    // Min start color
        Vector4(1.0f, 1.0f, 1.0f, 1.0f),    // Max start color
        Vector4(0.6f, 0.6f, 0.9f, 0.0f),    // Min end color
        Vector4(0.8f, 0.8f, 1.0f, 0.0f),    // Max end color
        0.0f,                               // Min rotation
        6.28f,                              // Max rotation
        -2.0f,                              // Min rotation velocity
        2.0f,                               // Max rotation velocity
        0.5f,                               // Min lifetime
        1.2f                                // Max lifetime
    );

    // Initially disable continuous emission for jump effect
    jumpEmitter_->SetEmitting(false);

    // Debug output
    OutputDebugStringA("ParticleEffectsManager: Successfully initialized\n");
}

void ParticleEffectsManager::Update() {
    // Update trail effect
    UpdateTrailEffect();

    // Update emitters
    trailEmitter_->Update();
    jumpEmitter_->Update();
}

void ParticleEffectsManager::UpdateTrailEffect() {
    if (!targetPlayer_) {
        return; // No target player
    }

    // Get player position and rotation
    Vector3 playerPos = targetPlayer_->GetPosition();
    Vector3 playerRot = targetPlayer_->GetRotation();

    // Calculate position behind player based on rotation
    Vector3 emitterPos;
    emitterPos.x = playerPos.x - sinf(playerRot.y) * trailOffsetDistance_;
    emitterPos.y = playerPos.y + trailHeightOffset_;
    emitterPos.z = playerPos.z - cosf(playerRot.y) * trailOffsetDistance_;

    // Update emitter position
    trailEmitter_->SetPosition(emitterPos);

    // Enable/disable emission based on player movement
    trailEmitter_->SetEmitting(targetPlayer_->IsMoving());
}

void ParticleEffectsManager::CreateJumpEffect(const Vector3& position) {
    if (!jumpEmitter_) {
        return;
    }

    // Set emitter position at the jump location
    Vector3 effectPos = position;
    effectPos.y += 0.1f; // Slightly above ground
    jumpEmitter_->SetPosition(effectPos);

    // Create a burst of particles
    particleManager_->Emit(
        "jumpEffect",
        effectPos,
        15, // Burst count
        Vector3(-0.3f, 0.1f, -0.3f),
        Vector3(0.3f, 0.5f, 0.3f),
        Vector3(0.0f, -0.01f, 0.0f),
        Vector3(0.0f, -0.03f, 0.0f),
        0.3f, 0.6f, 0.0f, 0.1f,
        Vector4(0.9f, 0.9f, 1.0f, 0.9f),
        Vector4(1.0f, 1.0f, 1.0f, 1.0f),
        Vector4(0.6f, 0.6f, 0.9f, 0.0f),
        Vector4(0.8f, 0.8f, 1.0f, 0.0f),
        0.0f, 6.28f, -2.0f, 2.0f, 0.5f, 1.2f
    );
}