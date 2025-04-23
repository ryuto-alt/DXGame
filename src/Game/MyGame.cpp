#include "MyGame.h"
#include "D3DResourceCheck.h"
#include <string>
#include <algorithm>
#include <ParticleManager.h>

MyGame::MyGame()
    : winApp_(nullptr),
    dxCommon_(nullptr),
    input_(nullptr),
    spriteCommon_(nullptr),
    srvManager_(nullptr),
    camera_(nullptr),
    sceneManager_(nullptr),
    sceneFactory_(nullptr) {
}

MyGame::~MyGame() {
    // Finalizeメソッドで解放するため、ここでは何もしない
}

void MyGame::Initialize() {
    try {
        // WinAppが設定されていることを確認
        assert(winApp_ != nullptr);

        // DirectXCommonの初期化
        dxCommon_ = std::make_unique<DirectXCommon>();
        dxCommon_->Initialize(winApp_);

        // SRVマネージャの初期化
        srvManager_ = std::make_unique<SrvManager>();
        srvManager_->Initialize(dxCommon_.get());

        // ここで明示的にPreDrawを呼び出し、ディスクリプタヒープを設定
        srvManager_->PreDraw();

        // テクスチャマネージャの初期化
        TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // デフォルトテクスチャの事前読み込み
        TextureManager::GetInstance()->LoadDefaultTexture();

        // ImGuiの初期化
        InitializeImGui();

        // 入力初期化
        input_ = std::make_unique<Input>();
        input_->Initialize(winApp_);

        // スプライト共通部分の初期化
        spriteCommon_ = std::make_unique<SpriteCommon>();
        spriteCommon_->Initialize(dxCommon_.get());

        // カメラの作成と初期化
        camera_ = std::make_unique<Camera>();
        camera_->SetTranslate({ 0.0f, 0.0f, -5.0f });
        Object3dCommon::SetDefaultCamera(camera_.get());

        // パーティクルマネージャの初期化
        ParticleManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // 基本的なパーティクルグループの作成
        ParticleManager::GetInstance()->CreateParticleGroup("smoke", "Resources/particle/smoke.png");

        // シーンファクトリーの作成
        sceneFactory_ = std::make_unique<GameSceneFactory>();

        // シーンマネージャーの取得と初期化
        sceneManager_ = SceneManager::GetInstance();
        sceneManager_->SetDirectXCommon(dxCommon_.get());
        sceneManager_->SetInput(input_.get());
        sceneManager_->SetSpriteCommon(spriteCommon_.get());
        sceneManager_->SetSrvManager(srvManager_.get());
        sceneManager_->SetCamera(camera_.get());
        sceneManager_->SetWinApp(winApp_);
        sceneManager_->Initialize(sceneFactory_.get());

        // デバッグ出力
        OutputDebugStringA("MyGame: Successfully initialized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in MyGame::Initialize: " + std::string(e.what()) + "\n").c_str());
    }
}

void MyGame::InitializeImGui() {
    try {
        // ImGui初期化
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(winApp_->GetHwnd());

        // SrvManagerのディスクリプタヒープを使用
        ImGui_ImplDX12_Init(
            dxCommon_->GetDevice(),
            2, // SwapChainのバッファ数
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            srvManager_->GetDescriptorHeap().Get(),
            srvManager_->GetCPUDescriptorHandle(0), // ImGui用に0番を使用
            srvManager_->GetGPUDescriptorHandle(0)
        );

        // デバッグ出力
        OutputDebugStringA("MyGame: ImGui initialized successfully\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Failed to initialize ImGui: " + std::string(e.what()) + "\n").c_str());
    }
}

void MyGame::Update() {
    try {
        // Windowsのメッセージ処理
        if (winApp_->ProcessMessage()) {
            endRequest_ = true;
            return;
        }

        // 入力更新
        input_->Update();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }

        // パーティクルマネージャの更新
        ParticleManager::GetInstance()->Update(camera_.get());

        // シーンマネージャーの更新
        sceneManager_->Update();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in MyGame::Update: " + std::string(e.what()) + "\n").c_str());
    }
}

void MyGame::Draw() {
    try {
        // DirectXの描画準備
        dxCommon_->Begin();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }

        // シーンマネージャーの描画
        sceneManager_->Draw();

        // パーティクルの描画
        ParticleManager::GetInstance()->Draw();

        // 描画終了
        dxCommon_->End();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in MyGame::Draw: " + std::string(e.what()) + "\n").c_str());
    }
}

void MyGame::Finalize() {
    try {
        // シーンマネージャーの終了処理
        if (sceneManager_) {
            sceneManager_->Finalize();
            // シングルトンなのでここではnullptrにするだけ
            sceneManager_ = nullptr;
        }

        // パーティクルマネージャーの終了処理
        ParticleManager::GetInstance()->Finalize();

        // ImGuiの解放
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // テクスチャマネージャの解放
        TextureManager::GetInstance()->Finalize();

        // 各リソースはunique_ptrにより自動的に解放される
        // 明示的にnullptrを設定
        camera_.reset();
        spriteCommon_.reset();
        input_.reset();
        srvManager_.reset();
        sceneFactory_.reset();
        dxCommon_.reset();

        // winAppはmain.cppで解放するため、ここでは解放しない

        // デバッグ出力
        OutputDebugStringA("MyGame: Successfully finalized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in MyGame::Finalize: " + std::string(e.what()) + "\n").c_str());
    }
}