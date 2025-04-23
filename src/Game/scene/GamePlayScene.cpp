#include "GamePlayScene.h"
#include "SceneManager.h"
#include <cassert>

GamePlayScene::GamePlayScene() {
    // コンストラクタは空のままにする
}

GamePlayScene::~GamePlayScene() {
    // デストラクタも空のままにする
}

void GamePlayScene::Initialize() {
    // 必須のリソースチェック
    assert(dxCommon_);
    assert(input_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 1.0f, -5.0f });

    // マウスカーソルを非表示に
    input_->SetMouseCursor(false);

    // 初期化完了
    initialized_ = true;
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 基本的なカメラ操作
    ControlCamera();

    // カメラの更新
    camera_->Update();

    // ESCキーでタイトルシーンへ戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        // マウスカーソルを表示に戻す
        input_->SetMouseCursor(true);
        sceneManager_->ChangeScene("Title");
    }

    // TABキーでマウスカーソルの表示切替
    if (input_->TriggerKey(DIK_TAB)) {
        showCursor_ = !showCursor_;
        input_->SetMouseCursor(showCursor_);
    }
}

void GamePlayScene::ControlCamera() {
    // 基本的なWASDキー操作のみ実装
    Vector3 cameraPos = camera_->GetTranslate();

    camera_->SetTranslate(cameraPos);
}

void GamePlayScene::Draw() {

}

void GamePlayScene::Finalize() {
    // マウスカーソルを表示に戻す
    input_->SetMouseCursor(true);
}