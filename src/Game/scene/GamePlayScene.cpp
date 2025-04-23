#include "GamePlayScene.h"

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでも特に何もしない
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期設定
    camera_->SetTranslate({ 0.0f, 0.0f, -5.0f });
    camera_->Update();

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: Successfully initialized\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // カメラの更新
    camera_->Update();

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // ImGuiで操作説明を表示
    ImGui::Begin("操作方法");
    ImGui::Text("ESC - タイトルに戻る");
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // 特に何もする必要がない
    OutputDebugStringA("GamePlayScene: Finalized\n");
}