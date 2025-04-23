// MyGame.cpp
#include "MyGame.h"
#include "GameSceneFactory.h" // 実装ファイルで具象クラスをインクルード
#include <cassert>

MyGame::MyGame() : engine_(nullptr) {
    // コンストラクタでは特に何もしない
}

MyGame::~MyGame() {
    // デストラクタでは特に何もしない
    // unique_ptrが自動的にリソースを解放
}

void MyGame::Initialize() {
    try {
        // UnoEngineのインスタンス取得
        engine_ = UnoEngine::GetInstance();

        // エンジンの初期化
        engine_->Initialize();

        // GameSceneFactoryの作成 - SceneFactoryへのポインタとして扱う
        SceneFactory* factory = new GameSceneFactory();

        // unique_ptrに所有権を移す（SceneFactory型として）
        sceneFactory_.reset(factory);

        // エンジンにSceneFactoryを設定
        engine_->SetSceneFactory(sceneFactory_.get());

        // 初期シーンへの遷移
        engine_->GetSceneManager()->ChangeScene("Title");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in MyGame::Initialize: " + std::string(e.what()) + "\n").c_str());
    }
}

void MyGame::Update() {
    // UnoEngineの更新処理を呼び出す
    if (engine_) {
        engine_->Update();

        // 終了リクエストがあれば反映
        if (engine_->IsEndRequested()) {
            endRequest_ = true;
        }
    }
}

void MyGame::Draw() {
    // UnoEngineの描画処理を呼び出す
    if (engine_) {
        engine_->Draw();
    }
}

void MyGame::Finalize() {
    // sceneFactory_はunique_ptrで自動的に解放される

    // UnoEngineのFinalizeを呼び出す
    // 注意: シングルトンなのでリソース解放は別途行われる
}