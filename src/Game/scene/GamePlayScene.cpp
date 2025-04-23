#include "GamePlayScene.h"
#include "CollisionDetection.h" // Include the new collision detection header

GamePlayScene::GamePlayScene() {
    // Constructor - initialize members to default values
}

GamePlayScene::~GamePlayScene() {
    // Destructor - unique_ptr handles memory cleanup
}

void GamePlayScene::Initialize() {
    // Ensure required dependencies are available
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    try {
        // Initialize ground model
        groundModel_ = std::make_unique<Model>();
        groundModel_->Initialize(dxCommon_);
        groundModel_->LoadFromObj("Resources/models/stage1", "stage1.obj");

        // Initialize ground object
        groundObject_ = std::make_unique<Object3d>();
        groundObject_->Initialize(dxCommon_, spriteCommon_);
        groundObject_->SetModel(groundModel_.get());
        groundObject_->SetScale({ 1.0f, 1.0f, 1.0f });
        groundObject_->SetPosition({ 0.0f, -1.0f, 0.0f });
        groundObject_->SetEnableLighting(true);

        // Create and initialize the various controllers
        playerController_ = std::make_unique<PlayerController>();
        playerController_->Initialize(dxCommon_, spriteCommon_, input_);
        // Set the stage model for collision detection
        playerController_->SetStageModel(groundModel_.get());

        cameraController_ = std::make_unique<CameraController>();
        cameraController_->Initialize(camera_, input_);
        cameraController_->SetTargetPlayer(playerController_.get());

        particleEffects_ = std::make_unique<ParticleEffectsManager>();
        particleEffects_->Initialize();
        particleEffects_->SetTargetPlayer(playerController_.get());

        // Connect player to particles for effects
        playerController_->SetParticleEffects(particleEffects_.get());

        uiManager_ = std::make_unique<UIManager>();
        uiManager_->Initialize(dxCommon_, spriteCommon_);
        uiManager_->SetPlayerController(playerController_.get());
        uiManager_->SetCameraController(cameraController_.get());

        // Set initialization flag
        initialized_ = true;
        OutputDebugStringA("GamePlayScene: Successfully initialized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: GamePlayScene initialization failed: " + std::string(e.what()) + "\n").c_str());
    }
}

void GamePlayScene::Update() {
    // Skip if not initialized
    if (!initialized_) return;

    try {
        // Update camera controller first to get latest rotation
        cameraController_->Update();

        // Update player with camera rotation for movement
        float cameraRotationY = cameraController_->GetRotationY();
        playerController_->Move(cameraRotationY);
        playerController_->Update();

        // Update particle effects
        particleEffects_->Update();

        // Update ground object
        if (groundObject_) {
            groundObject_->Update();
        }

        // Update UI elements
        uiManager_->Update();

        // Handle scene exit with ESC key
        if (input_->TriggerKey(DIK_ESCAPE)) {
            // Reset mouse cursor visibility before exiting
            input_->SetMouseCursor(true);
            sceneManager_->ChangeScene("Title");
        }
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in GamePlayScene::Update: " + std::string(e.what()) + "\n").c_str());
    }
}

void GamePlayScene::Draw() {
    // Skip if not initialized
    if (!initialized_) return;

    try {
        // Draw ground
        if (groundObject_) {
            groundObject_->Draw();
        }

        // Draw player
        playerController_->Draw();

        // Draw UI elements
        uiManager_->Draw();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in GamePlayScene::Draw: " + std::string(e.what()) + "\n").c_str());
    }
}

void GamePlayScene::Finalize() {
    // Ensure mouse cursor is visible when leaving scene
    input_->SetMouseCursor(true);

    // Components will be cleaned up automatically by unique_ptr destructors
    OutputDebugStringA("GamePlayScene: Finalized\n");
}