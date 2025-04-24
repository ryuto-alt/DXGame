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
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    camera_->Update();

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // 青い画面の描画は自動的に行われる
    // DirectXCommonのBegin()で青色がクリアカラーとして設定されているため、
    // 何も描画しなければ青い画面になる

    // 操作説明だけImGuiで表示
    ImGui::Begin("操作説明");
    ImGui::Text("ESC - タイトルに戻る");
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // 特に何もする必要がない
    OutputDebugStringA("GamePlayScene: 終了処理完了\n");
}