#pragma once

#include "UnoEngine.h"
#include "PlayerController.h"
#include "CameraController.h"

class UIManager {
public:
    // Constructor/Destructor
    UIManager();
    ~UIManager();

    // Core methods
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void Update();
    void Draw();

    // Set dependencies
    void SetPlayerController(PlayerController* player) { playerController_ = player; }
    void SetCameraController(CameraController* camera) { cameraController_ = camera; }

private:
    // UI Sprites
    std::unique_ptr<Sprite> uiSprite_;
    std::unique_ptr<Sprite> fovTextSprite_;

    // ImGui rendering
    void RenderImGui();

    // Dependencies
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    PlayerController* playerController_ = nullptr;
    CameraController* cameraController_ = nullptr;
};