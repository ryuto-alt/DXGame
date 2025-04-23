#include "TitleScene.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <cassert>

TitleScene::TitleScene() {
    // コンストラクタでの初期化は最小限にする
}

TitleScene::~TitleScene() {
    // デストラクタでは特に何もしない
}

void TitleScene::Initialize() {
    // リソースのnullチェック
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(srvManager_);
    assert(camera_);

    // ImGuiの初期化
    InitializeImGui();

    // 3Dモデルの初期化
    Initialize3DModels();

    // スプライトの初期化
    InitializeSprites();

    // カメラの初期設定
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

    // 初期化完了フラグ
    initialized_ = true;
}

void TitleScene::InitializeImGui() {
    // ImGuiの設定（必要に応じて）
}

void TitleScene::Initialize3DModels() {
    // Sphereモデルの初期化
    sphereModel_ = std::make_unique<Model>();
    sphereModel_->Initialize(dxCommon_);
    sphereModel_->LoadFromObj("Resources/models", "sphere.obj");

    // 3Dオブジェクトの初期化
    sphereObject_ = std::make_unique<Object3d>();
    sphereObject_->Initialize(dxCommon_, spriteCommon_);
    sphereObject_->SetModel(sphereModel_.get());

    // オブジェクトの初期設定
    sphereObject_->SetScale({ 1.0f, 1.0f, 1.0f });
    sphereObject_->SetPosition({ 0.0f, 0.0f, 0.0f });

    // ライティングを有効化
    sphereObject_->SetEnableLighting(true);

    // ディレクショナルライトの設定
    DirectionalLight light;
    light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    light.direction = { 0.5f, -1.0f, 0.5f };
    light.intensity = 1.0f;
    sphereObject_->SetDirectionalLight(light);
}

void TitleScene::InitializeSprites() {
    // タイトルロゴの初期化（テクスチャがある場合）
    try {
        titleLogo_ = std::make_unique<Sprite>();
        titleLogo_->Initialize(spriteCommon_, "Resources/textures/title_logo.png");
        titleLogo_->SetPosition({ WinApp::kClientWidth / 2.0f, 200.0f });
        titleLogo_->SetSize({ 600.0f, 150.0f });
        titleLogo_->SetAnchorPoint({ 0.5f, 0.5f });
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("Failed to initialize title logo: " + std::string(e.what()) + "\n").c_str());
        titleLogo_ = nullptr;
    }
}

void TitleScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラの更新
    camera_->Update();

    // オブジェクトの回転
    rotationAngle_ += 0.01f;
    sphereObject_->SetRotation({ 0.0f, rotationAngle_, 0.0f });

    // 3Dオブジェクトの更新
    sphereObject_->Update();

    // titleLogo_がnullptrでない場合のみ更新
    if (titleLogo_) {
        titleLogo_->Update();
    }

    // スペースキーでゲームプレイシーンへ遷移
    if (input_->TriggerKey(DIK_SPACE)) {
        sceneManager_->ChangeScene("GamePlay");
    }
}

void TitleScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 3Dオブジェクトの描画準備（SRVヒープの設定）
    srvManager_->PreDraw();

    // 3Dオブジェクトの描画
    sphereObject_->Draw();

    // スプライトの描画準備（共通設定）
    spriteCommon_->CommonDraw();

    // titleLogo_がnullptrでない場合のみ描画
    if (titleLogo_) {
        titleLogo_->Draw();
    }

    // ImGuiの描画
    DrawImGui();
}

void TitleScene::DrawImGui() {
    // ImGuiの新しいフレーム開始
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // タイトル表示用の大きなウィンドウ
    ImGui::SetNextWindowPos(ImVec2(WinApp::kClientWidth / 2 - 150, 100), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);
    ImGui::Begin("##Title", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "3D Game Demo");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Press SPACE key to start game");
    ImGui::End();

    // デバッグ情報ウィンドウ
    ImGui::Begin("TitleScene Debug");
    ImGui::Text("Camera Position: %.2f, %.2f, %.2f",
        camera_->GetTranslate().x,
        camera_->GetTranslate().y,
        camera_->GetTranslate().z);
    ImGui::Text("Rotation Angle: %.2f", rotationAngle_);
    ImGui::End();

    // ImGuiの描画
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());
}

void TitleScene::Finalize() {
    // リソースの解放（必要に応じて）
    sphereObject_.reset();
    sphereModel_.reset();
    if (titleLogo_) {
        titleLogo_.reset();
    }
}