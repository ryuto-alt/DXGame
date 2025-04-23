#pragma once

#include "UnoEngine.h"
#include "PlayerController.h"

class ParticleEffectsManager {
public:
    // Constructor/Destructor
    ParticleEffectsManager();
    ~ParticleEffectsManager();

    // Core methods
    void Initialize();
    void Update();

    // Effect methods
    void UpdateTrailEffect();
    void CreateJumpEffect(const Vector3& position);

    // Target settings
    void SetTargetPlayer(PlayerController* player) { targetPlayer_ = player; }

private:
    // Particle emitters
    std::unique_ptr<ParticleEmitter> trailEmitter_;
    std::unique_ptr<ParticleEmitter> jumpEmitter_;

    // Effect parameters
    float trailOffsetDistance_ = 0.5f;
    float trailHeightOffset_ = 0.5f;

    // Dependencies
    PlayerController* targetPlayer_ = nullptr;
    ParticleManager* particleManager_ = nullptr;
};