#pragma once

#include "UnoEngine.h"
#include "PlayerController.h"
#include "CameraController.h"
#include "ParticleEffectsManager.h"
#include "UIManager.h"
#include "CollisionVisualizer.h" // Add the visualizer header

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
    // Initialization flag
    bool initialized_ = false;

    // Ground object
    std::unique_ptr<Model> groundModel_;
    std::unique_ptr<Object3d> groundObject_;

    // Game components
    std::unique_ptr<PlayerController> playerController_;
    std::unique_ptr<CameraController> cameraController_;
    std::unique_ptr<ParticleEffectsManager> particleEffects_;
    std::unique_ptr<UIManager> uiManager_;

    // Collision visualization (for debugging)
    std::unique_ptr<CollisionVisualizer> collisionVisualizer_;
};