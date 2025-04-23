#include "SceneManager.h"
#include "SceneFactory.h"
#include <cassert>

// 静的メンバ変数の実体化
SceneManager* SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
    if (!instance_) {
        instance_ = new SceneManager();
    }
    return instance_;
}

void SceneManager::Initialize(SceneFactory* sceneFactory) {
    assert(sceneFactory);
    sceneFactory_ = sceneFactory;

    // 最初のシーンをTitleに設定
    nextScene_ = "Title";

    // デバッグ出力
    OutputDebugStringA("SceneManager initialized successfully\n");
}

void SceneManager::Update() {
    // SRVヒープを毎フレーム設定（これが重要）
    if (srvManager_) {
        srvManager_->PreDraw();
    }

    // シーン切り替えチェック
    if (!nextScene_.empty()) {
        // デバッグ出力
        OutputDebugStringA(("SceneManager: Changing scene to " + nextScene_ + "\n").c_str());

        // 現在のシーンの終了処理
        if (currentScene_) {
            currentScene_->Finalize();
            currentScene_.reset(); // unique_ptrをクリア
        }

        // 次のシーンを生成
        currentScene_ = sceneFactory_->CreateScene(nextScene_);

        // シーンマネージャーのポインタをセット
        currentScene_->SetSceneManager(this);

        // 共通リソースをセット
        currentScene_->SetDirectXCommon(dxCommon_);
        currentScene_->SetInput(input_);
        currentScene_->SetSpriteCommon(spriteCommon_);
        currentScene_->SetSrvManager(srvManager_);
        currentScene_->SetCamera(camera_);

        try {
            // シーンの初期化（例外をキャッチ）
            currentScene_->Initialize();
            OutputDebugStringA(("SceneManager: Successfully initialized scene " + nextScene_ + "\n").c_str());
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Failed to initialize scene " + nextScene_ + ": " + e.what() + "\n").c_str());
        }

        // 次のシーン名をクリア
        nextScene_.clear();
    }

    // 現在のシーンの更新
    if (currentScene_) {
        try {
            currentScene_->Update();
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Exception in scene update: " + std::string(e.what()) + "\n").c_str());
        }
    }
}

void SceneManager::Draw() {
    // SRVヒープを描画前に設定
    if (srvManager_) {
        srvManager_->PreDraw();
    }

    // 現在のシーンの描画
    if (currentScene_) {
        try {
            currentScene_->Draw();
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Exception in scene draw: " + std::string(e.what()) + "\n").c_str());
        }
    }
}

void SceneManager::Finalize() {
    // 現在のシーンの終了処理
    if (currentScene_) {
        try {
            currentScene_->Finalize();
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Exception in scene finalize: " + std::string(e.what()) + "\n").c_str());
        }
        currentScene_.reset(); // 明示的にunique_ptrをクリア
    }

    // シングルトンインスタンスの解放
    delete instance_;
    instance_ = nullptr;

    // デバッグ出力
    OutputDebugStringA("SceneManager finalized successfully\n");
}

void SceneManager::ChangeScene(const std::string& sceneName) {
    // 次のシーン名を設定
    nextScene_ = sceneName;

    // デバッグ出力
    OutputDebugStringA(("SceneManager: Scene change requested to " + sceneName + "\n").c_str());
}