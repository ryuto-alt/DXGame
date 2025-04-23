#include "GamePlayScene.h"

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでも特に何もしない（ResourceはuniquePtr）
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 5.0f, -15.0f });
    camera_->SetRotate({ 0.2f, 0.0f, 0.0f });

    // マウスカーソルを非表示に
    input_->SetMouseCursor(false);

    // プレイヤーモデルの初期化
    playerModel_ = std::make_unique<Model>();
    playerModel_->Initialize(dxCommon_);
    playerModel_->LoadFromObj("Resources/models", "player.obj");

    // プレイヤーオブジェクトの初期化
    playerObject_ = std::make_unique<Object3d>();
    playerObject_->Initialize(dxCommon_, spriteCommon_);
    playerObject_->SetModel(playerModel_.get());
    playerObject_->SetScale({ 1.0f, 1.0f, 1.0f });
    playerObject_->SetPosition({ 0.0f, 0.0f, 0.0f });

    // 地面モデルの初期化
    groundModel_ = std::make_unique<Model>();
    groundModel_->Initialize(dxCommon_);
    groundModel_->LoadFromObj("Resources/models/stage1", "stage1.obj");

    // 地面オブジェクトの初期化
    groundObject_ = std::make_unique<Object3d>();
    groundObject_->Initialize(dxCommon_, spriteCommon_);
    groundObject_->SetModel(groundModel_.get());
    groundObject_->SetScale({ 20.0f, 1.0f, 20.0f });
    groundObject_->SetPosition({ 0.0f, -1.0f, 0.0f });

    UnoEngine::GetInstance()->GetParticleManager()->CreateParticleGroup("effect", "Resources/particle/explosion.png");

    // パーティクルエミッターの初期化
    effectEmitter_ = std::make_unique<ParticleEmitter>(
        "effect",                // グループ名
        Vector3(0.0f, 0.5f, 0.0f), // 発生位置
        5,                      // 発生数
        2.0f,                   // 発生レート
        Vector3(-0.5f, 0.1f, -0.5f), // 初速度最小
        Vector3(0.5f, 0.5f, 0.5f),   // 初速度最大
        Vector3(0.0f, 0.1f, 0.0f),   // 加速度最小
        Vector3(0.0f, 0.2f, 0.0f)    // 加速度最大
    );

    // UIスプライトの初期化
    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize(spriteCommon_, "Resources/textures/ui_game.png");
    uiSprite_->SetPosition({ 100.0f, 50.0f });
    uiSprite_->SetSize({ 200.0f, 100.0f });

    // 初期化完了
    initialized_ = true;
}

void GamePlayScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラの制御
    ControlCamera();

    // カメラの更新
    camera_->Update();

    // プレイヤーの更新
    playerObject_->Update();

    // 地面の更新
    groundObject_->Update();

    // パーティクルエミッターの更新
    effectEmitter_->Update();

    // UIの更新
    uiSprite_->Update();

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
    // WASDキーでカメラ移動
    Vector3 cameraPos = camera_->GetTranslate();
    Vector3 cameraRot = camera_->GetRotate();

    // 左右キーでカメラ回転
    if (input_->PushKey(DIK_LEFT)) {
        cameraRotation_ -= 0.02f;
    }
    if (input_->PushKey(DIK_RIGHT)) {
        cameraRotation_ += 0.02f;
    }

    // 前後左右移動（カメラの向きを考慮）
    float speed = 0.2f;
    if (input_->PushKey(DIK_W)) {
        cameraPos.x += sinf(cameraRotation_) * speed;
        cameraPos.z += cosf(cameraRotation_) * speed;
    }
    if (input_->PushKey(DIK_S)) {
        cameraPos.x -= sinf(cameraRotation_) * speed;
        cameraPos.z -= cosf(cameraRotation_) * speed;
    }
    if (input_->PushKey(DIK_A)) {
        cameraPos.x -= cosf(cameraRotation_) * speed;
        cameraPos.z += sinf(cameraRotation_) * speed;
    }
    if (input_->PushKey(DIK_D)) {
        cameraPos.x += cosf(cameraRotation_) * speed;
        cameraPos.z -= sinf(cameraRotation_) * speed;
    }

    // 上下移動
    if (input_->PushKey(DIK_Q)) cameraPos.y += speed;
    if (input_->PushKey(DIK_E)) cameraPos.y -= speed;

    // カメラに反映
    camera_->SetTranslate(cameraPos);
    camera_->SetRotate({ cameraRot.x, cameraRotation_, cameraRot.z });
}

void GamePlayScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 3Dオブジェクトの描画
    playerObject_->Draw();
    groundObject_->Draw();

    // スプライト共通設定
    spriteCommon_->CommonDraw();

    // UIの描画
    uiSprite_->Draw();
}

void GamePlayScene::Finalize() {
    // マウスカーソルを表示に戻す
    input_->SetMouseCursor(true);
}