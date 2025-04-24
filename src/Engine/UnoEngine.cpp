#include "UnoEngine.h"
#include "GameSceneFactory.h" // 具象クラスはcppファイルでインクルード
#include <cassert>

// 静的メンバ変数の実体化
UnoEngine* UnoEngine::instance_ = nullptr;

UnoEngine* UnoEngine::GetInstance() {
    if (!instance_) {
        instance_ = new UnoEngine();
    }
    return instance_;
}

void UnoEngine::Initialize() {
    try {
        // WinAppの初期化
        winApp_ = std::make_unique<WinApp>();
        winApp_->Initialize();

        // DirectXCommonの初期化
        dxCommon_ = std::make_unique<DirectXCommon>();
        dxCommon_->Initialize(winApp_.get());

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
        input_->Initialize(winApp_.get());

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

        // 衝突判定マネージャの初期化（特別な初期化は不要）
        // すでにシングルトンパターンで実装されているため、呼び出すだけで初期化される

        // シーンマネージャーの取得と初期化
        SceneManager* sceneManager = SceneManager::GetInstance();
        sceneManager->SetDirectXCommon(dxCommon_.get());
        sceneManager->SetInput(input_.get());
        sceneManager->SetSpriteCommon(spriteCommon_.get());
        sceneManager->SetSrvManager(srvManager_.get());
        sceneManager->SetCamera(camera_.get());
        sceneManager->SetWinApp(winApp_.get());

        // シーンファクトリーが設定されていれば初期化
        if (sceneFactory_) {
            sceneManager->Initialize(sceneFactory_.get());
        }

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Successfully initialized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Initialize: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Update() {
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

        // カメラの更新
        camera_->Update();

        // パーティクルマネージャの更新
        ParticleManager::GetInstance()->Update(camera_.get());

        // 衝突判定マネージャの更新
        Collision::CollisionManager::GetInstance()->Update(1.0f / 60.0f); // 60FPS想定

        // シーンマネージャーの更新
        SceneManager::GetInstance()->Update();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Update: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Draw() {
    try {
        // DirectXの描画準備
        dxCommon_->Begin();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }

        // シーンマネージャーの描画
        SceneManager::GetInstance()->Draw();

        // パーティクルの描画
        ParticleManager::GetInstance()->Draw();

        // ImGuiの準備と描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());

        // 描画終了
        dxCommon_->End();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Draw: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Finalize() {
    try {
        // シーンマネージャーの終了処理
        SceneManager::GetInstance()->Finalize();

        // パーティクルマネージャーの終了処理
        ParticleManager::GetInstance()->Finalize();

        // 衝突判定マネージャの終了処理
        Collision::CollisionManager::GetInstance()->ClearColliders();

        // ImGuiの解放
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // テクスチャマネージャの解放
        TextureManager::GetInstance()->Finalize();

        // オーディオマネージャの解放（必要に応じて）
        AudioManager::GetInstance()->Finalize();

        // 各リソースはunique_ptrにより自動的に解放される
        // 明示的にnullptrを設定
        camera_.reset();
        spriteCommon_.reset();
        input_.reset();
        srvManager_.reset();
        sceneFactory_.reset();
        dxCommon_.reset();
        winApp_.reset();

        // シングルトンインスタンスの解放
        delete instance_;
        instance_ = nullptr;

        // デバッグ出力
        OutputDebugStringA("UnoEngine: Successfully finalized\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Finalize: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Run() {
    // ゲームループ
    while (!IsEndRequested()) {
        // ImGuiの新しいフレーム
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 更新
        Update();

        // 描画
        Draw();
    }

    // 終了処理
    Finalize();
}

void UnoEngine::SetSceneFactory(SceneFactory* sceneFactory) {
    // nullptrチェック
    if (sceneFactory == nullptr) {
        sceneFactory_.reset();
        return;
    }

    // unique_ptrにセット（所有権を移譲）
    sceneFactory_ = std::unique_ptr<SceneFactory>(sceneFactory);

    // シーンマネージャーの初期化
    SceneManager::GetInstance()->Initialize(sceneFactory_.get());
}

void UnoEngine::InitializeImGui() {
    try {
        // ImGui初期化
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // ImGuiのIO設定を取得
        ImGuiIO& io = ImGui::GetIO();

        // 日本語フォント設定
        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;  // 既存のフォントと統合

        // 日本語フォントのパス（例：MS Gothic）
        // Windows標準フォントを使用
        const char* fontPath = "C:\\Windows\\Fonts\\msgothic.ttc";

        // 日本語の文字範囲を指定
        static const ImWchar japaneseFontRanges[] = {
            0x0020, 0x00FF, // 基本ラテン文字
            0x3000, 0x30FF, // 日本語（平仮名、カタカナ）
            0x31F0, 0x31FF, // カタカナ拡張
            0xFF00, 0xFFEF, // 全角文字
            0x4E00, 0x9FAF, // CJK統合漢字
            0,
        };

        // デフォルトフォント読み込み
        io.Fonts->AddFontDefault();

        // 日本語フォント読み込み
        io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &fontConfig, japaneseFontRanges);

        // ファイルが見つからない場合のフォールバック処理
        if (!io.Fonts->Fonts.Size || io.Fonts->Fonts.Size <= 1) {
            // デフォルトフォントのみの場合は警告を出力
            OutputDebugStringA("WARNING: 日本語フォントが読み込めませんでした。フォールバックフォントを使用します。\n");

            // フォールバックフォントとしてWindows標準のフォントを試行
            const char* fallbackFonts[] = {
                "C:\\Windows\\Fonts\\meiryo.ttc",
                "C:\\Windows\\Fonts\\msgothic.ttc",
                "C:\\Windows\\Fonts\\YuGothM.ttc"
            };

            for (const char* fallbackFont : fallbackFonts) {
                io.Fonts->AddFontFromFileTTF(fallbackFont, 16.0f, &fontConfig, japaneseFontRanges);
                if (io.Fonts->Fonts.Size > 1) {
                    OutputDebugStringA(("日本語フォールバックフォントを読み込みました: " + std::string(fallbackFont) + "\n").c_str());
                    break;
                }
            }
        }
        else {
            OutputDebugStringA("日本語フォントが正常に読み込まれました。\n");
        }

        // フォントテクスチャをビルド
        io.Fonts->Build();

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
        OutputDebugStringA("UnoEngine: ImGui initialized successfully\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Failed to initialize ImGui: " + std::string(e.what()) + "\n").c_str());
    }
}