#include "UIManager.h"

UIManager::UIManager() {
    // Initialize default values
}

UIManager::~UIManager() {
    // Cleanup if needed
}

void UIManager::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    // Store dependencies
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;

    // Initialize UI sprite
    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize(spriteCommon_, "Resources/textures/ui_game.png");
    uiSprite_->SetPosition({ 100.0f, 50.0f });
    uiSprite_->SetSize({ 200.0f, 100.0f });

    // Check if FOV text texture exists
    const std::string fovTextPath = "Resources/textures/default_white.png";
    if (!TextureManager::GetInstance()->IsTextureExists(fovTextPath)) {
        TextureManager::GetInstance()->LoadDefaultTexture();
    }

    // Initialize FOV text sprite (initially transparent)
    fovTextSprite_ = std::make_unique<Sprite>();
    fovTextSprite_->Initialize(spriteCommon_, TextureManager::GetInstance()->GetDefaultTexturePath());
    fovTextSprite_->SetPosition({ WinApp::kClientWidth - 180.0f, WinApp::kClientHeight - 50.0f });
    fovTextSprite_->SetSize({ 160.0f, 40.0f });
    fovTextSprite_->setColor({ 1.0f, 1.0f, 1.0f, 0.0f }); // Transparent

    // Debug output
    OutputDebugStringA("UIManager: Successfully initialized\n");
}

void UIManager::Update() {
    // Update UI sprites
    if (uiSprite_) {
        uiSprite_->Update();
    }

    if (fovTextSprite_) {
        fovTextSprite_->Update();
    }
}

void UIManager::Draw() {
    // Set common sprite settings
    spriteCommon_->CommonDraw();

    // Draw UI elements
    if (uiSprite_) {
        uiSprite_->Draw();
    }

    if (fovTextSprite_) {
        fovTextSprite_->Draw();
    }

    // Render ImGui elements
    RenderImGui();
}

void UIManager::RenderImGui() {
    // Start ImGui window
    ImGui::Begin("ゲームコントロール");

    // Display camera FOV information if camera controller is available
    if (cameraController_) {
        float fovDegrees = cameraController_->GetCurrentFOV() * 57.3f; // Convert radians to degrees
        ImGui::Text("FOV: %.1f 度", fovDegrees);
        ImGui::Text("カメラモード: %s", cameraController_->IsFPSMode() ? "一人称視点" : "三人称視点");
    }

    // Display control instructions
    ImGui::Separator();
    ImGui::Text("操作方法:");
    ImGui::Text("WASD - 移動");
    ImGui::Text("スペース - ジャンプ");
    ImGui::Text("F/G - FOV変更");
    ImGui::Text("1 - カメラモード切替");
    ImGui::Text("TAB - カーソル表示/非表示");
    ImGui::Text("ESC - タイトルに戻る");

    // End ImGui window
    ImGui::End();
}