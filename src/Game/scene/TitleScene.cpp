#include "TitleScene.h"

TitleScene::TitleScene() {
    // コンストラクタでは特に何もしない
}

TitleScene::~TitleScene() {
    // デストラクタでも特に何もしない（ResourceはuniquePtr）
}

void TitleScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

    // 3Dモデルの初期化
    sphereModel_ = std::make_unique<Model>();
    sphereModel_->Initialize(dxCommon_);
    sphereModel_->LoadFromObj("Resources/models", "sphere.obj");

    // 3Dオブジェクトの初期化
    sphereObject_ = std::make_unique<Object3d>();
    sphereObject_->Initialize(dxCommon_, spriteCommon_);
    sphereObject_->SetModel(sphereModel_.get());
    sphereObject_->SetScale({ 2.0f, 2.0f, 2.0f });
    sphereObject_->SetPosition({ 0.0f, 0.0f, 0.0f });

    // ライティングを有効化
    sphereObject_->SetEnableLighting(true);

    // タイトルロゴの初期化
    titleLogo_ = std::make_unique<Sprite>();
    titleLogo_->Initialize(spriteCommon_, "Resources/textures/title_logo.png");
    titleLogo_->SetPosition({ WinApp::kClientWidth / 2.0f, 200.0f });
    titleLogo_->SetSize({ 500.0f, 150.0f });
    titleLogo_->SetAnchorPoint({ 0.5f, 0.5f });

    // 初期化完了
    initialized_ = true;
}

void TitleScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラの更新
    camera_->Update();

    // 3Dオブジェクトのアニメーション
    rotationAngle_ += 0.01f;
    sphereObject_->SetRotation({ 0.0f, rotationAngle_, 0.0f });
    sphereObject_->Update();

    // タイトルロゴの更新
    titleLogo_->Update();

    // スペースキーでゲームプレイシーンへ
    if (input_->TriggerKey(DIK_SPACE)) {
        sceneManager_->ChangeScene("GamePlay");
    }
}

void TitleScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 3Dオブジェクトの描画
    sphereObject_->Draw();

    // スプライト共通設定
    spriteCommon_->CommonDraw();

    // タイトルロゴの描画
    titleLogo_->Draw();
}

void TitleScene::Finalize() {
    // 特に追加のリソース解放が必要なければ何もしない
}