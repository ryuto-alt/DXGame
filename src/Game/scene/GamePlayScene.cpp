#include "GamePlayScene.h"

GamePlayScene::GamePlayScene() {
    // コンストラクタは特に何もする必要がない
}

GamePlayScene::~GamePlayScene() {
    // デストラクタも特に何もする必要がない（リソースはuniquePtrで管理）
}

void GamePlayScene::Initialize() {
    // 必要なリソースの存在確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 2.0f, -5.0f });
    camera_->SetRotate({ 0.2f, 0.0f, 0.0f });

    // 大きなステージを扱うためにファークリップ距離を増加
    camera_->SetFarClip(1000.0f);

    // 初期FOV設定
    camera_->SetFovY(initialFovY_);
    currentFovY_ = initialFovY_;

    // マウスカーソルを非表示
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

    // サイズの問題を修正するために大幅に縮小したスケールで地面オブジェクトを初期化
    groundObject_ = std::make_unique<Object3d>();
    groundObject_->Initialize(dxCommon_, spriteCommon_);
    // 「大きすぎる」モデルの問題を修正するためにスケールを大幅に縮小
    groundObject_->SetScale({ 0.1f, 0.1f, 0.1f });
    groundObject_->SetPosition({ 0.0f, -1.0f, 0.0f });

    // より良い視覚効果のために地面オブジェクトのライティングを有効化
    groundObject_->SetEnableLighting(true);

    // プレイヤーの軌跡エフェクト用のパーティクルグループを作成
    UnoEngine::GetInstance()->GetParticleManager()->CreateParticleGroup("playerTrail", "Resources/particle/smoke.png");

    // プレイヤーの軌跡用パーティクルエミッタの初期化
    trailEmitter_ = std::make_unique<ParticleEmitter>(
        "playerTrail",                       // グループ名
        Vector3(0.0f, 0.5f, 0.0f),          // 初期位置
        3,                                  // 1回の発生で生成するパーティクル数
        15.0f,                              // 発生率
        Vector3(-0.1f, 0.05f, -0.1f),       // 最小速度
        Vector3(0.1f, 0.2f, 0.1f),          // 最大速度
        Vector3(0.0f, 0.0f, 0.0f),          // 最小加速度
        Vector3(0.0f, 0.1f, 0.0f),          // 最大加速度
        0.2f,                               // 最小開始サイズ
        0.4f,                               // 最大開始サイズ
        0.0f,                               // 最小終了サイズ
        0.0f,                               // 最大終了サイズ
        Vector4(0.8f, 0.8f, 1.0f, 0.8f),    // 最小開始色（薄い青）
        Vector4(1.0f, 1.0f, 1.0f, 1.0f),    // 最大開始色（白）
        Vector4(0.5f, 0.5f, 0.8f, 0.0f),    // 最小終了色（透明な青）
        Vector4(0.7f, 0.7f, 1.0f, 0.0f),    // 最大終了色（透明な薄い青）
        0.0f,                               // 最小回転角度
        6.28f,                              // 最大回転角度（1周）
        -1.0f,                              // 最小回転速度
        1.0f,                               // 最大回転速度
        0.3f,                               // 最小生存時間
        0.8f                                // 最大生存時間
    );

    // UIスプライトの初期化
    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize(spriteCommon_, "Resources/textures/ui_game.png");
    uiSprite_->SetPosition({ 100.0f, 50.0f });
    uiSprite_->SetSize({ 200.0f, 100.0f });

    // マウス制御用の画面中央位置を取得
    screenCenterX_ = WinApp::kClientWidth / 2;
    screenCenterY_ = WinApp::kClientHeight / 2;

    // FOVテキストスプライトの初期化
    fovTextSprite_ = std::make_unique<Sprite>();
    fovTextSprite_->Initialize(spriteCommon_, "Resources/textures/fov_text.png");
    fovTextSprite_->SetPosition({ WinApp::kClientWidth - 180.0f, WinApp::kClientHeight - 50.0f });
    fovTextSprite_->SetSize({ 160.0f, 40.0f });

    // 初期化完了
    initialized_ = true;
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // マウス入力の取得
    DIMOUSESTATE mouseState = {};
    input_->GetMouseState(&mouseState);

    // マウスの動きに基づいてカメラの回転を処理
    if (!showCursor_) {
        // マウスの移動量を計算
        mouseDeltaX_ = mouseState.lX;
        mouseDeltaY_ = mouseState.lY;

        // マウスの移動量に基づいてカメラの回転を更新
        cameraRotationY_ += mouseDeltaX_ * mouseSensitivity_;
        cameraRotationX_ += mouseDeltaY_ * mouseSensitivity_;

        // カメラの回転角を制限してフリップを防止
        if (cameraRotationX_ > 1.5f) {
            cameraRotationX_ = 1.5f;
        }
        else if (cameraRotationX_ < -1.5f) {
            cameraRotationX_ = -1.5f;
        }
    }

    // FキーとGキーでFOVを更新
    if (input_->PushKey(DIK_F)) {
        // FOVを増加（広角）
        currentFovY_ += fovChangeSpeed_;
        if (currentFovY_ > maxFovY_) {
            currentFovY_ = maxFovY_;
        }
        camera_->SetFovY(currentFovY_);
    }

    if (input_->PushKey(DIK_G)) {
        // FOVを減少（狭角）
        currentFovY_ -= fovChangeSpeed_;
        if (currentFovY_ < minFovY_) {
            currentFovY_ = minFovY_;
        }
        camera_->SetFovY(currentFovY_);
    }

    // 1キーでFPSモードとTPSモードを切り替え
    if (input_->TriggerKey(DIK_1)) {
        isFPSMode_ = !isFPSMode_;
    }

    // プレイヤーの移動を更新
    MovePlayer();

    // プレイヤーの位置と表示モードに基づいてカメラを更新
    UpdateCamera();

    // 軌跡パーティクルエミッタの位置を更新
    UpdateTrailEmitter();

    // オブジェクトの更新
    playerObject_->Update();
    groundObject_->Update();

    // 軌跡エミッタの更新
    trailEmitter_->Update();

    // UIスプライトの更新
    uiSprite_->Update();
    fovTextSprite_->Update();

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        // マウスカーソルを表示
        input_->SetMouseCursor(true);
        sceneManager_->ChangeScene("Title");
    }

    // TABキーでマウスカーソルの表示を切り替え
    if (input_->TriggerKey(DIK_TAB)) {
        showCursor_ = !showCursor_;
        input_->SetMouseCursor(showCursor_);
    }
}

void GamePlayScene::MovePlayer() {
    // 現在のプレイヤー位置を取得
    Vector3 playerPos = playerObject_->GetPosition();

    // カメラの回転に基づいて移動方向を計算
    float forwardX = sinf(cameraRotationY_);
    float forwardZ = cosf(cameraRotationY_);

    float rightX = cosf(cameraRotationY_);
    float rightZ = -sinf(cameraRotationY_);

    // 移動ベクトルを計算
    Vector3 moveVec = { 0.0f, 0.0f, 0.0f };

    // 前後移動（WとSキー）
    if (input_->PushKey(DIK_W)) {
        moveVec.x += forwardX * playerSpeed_;
        moveVec.z += forwardZ * playerSpeed_;
    }
    if (input_->PushKey(DIK_S)) {
        moveVec.x -= forwardX * playerSpeed_;
        moveVec.z -= forwardZ * playerSpeed_;
    }

    // 左右移動（AとDキー）
    if (input_->PushKey(DIK_A)) {
        moveVec.x -= rightX * playerSpeed_;
        moveVec.z -= rightZ * playerSpeed_;
    }
    if (input_->PushKey(DIK_D)) {
        moveVec.x += rightX * playerSpeed_;
        moveVec.z += rightZ * playerSpeed_;
    }

    // ジャンプ処理（スペースキー）
    if (input_->TriggerKey(DIK_SPACE) && !isJumping_) {
        // 地面にいる場合はジャンプ開始
        isJumping_ = true;
        verticalVelocity_ = jumpPower_;
    }

    // 重力を適用して垂直位置を更新
    if (isJumping_) {
        // 垂直速度に重力を適用
        verticalVelocity_ -= gravity_;

        // 垂直位置を更新
        playerPos.y += verticalVelocity_;

        // プレイヤーが着地したかチェック
        if (playerPos.y <= 0.0f) {
            playerPos.y = 0.0f;
            isJumping_ = false;
            verticalVelocity_ = 0.0f;
        }
    }

    // 水平方向の移動をプレイヤーの位置に適用
    playerPos.x += moveVec.x;
    playerPos.z += moveVec.z;

    // プレイヤーの位置を更新
    playerObject_->SetPosition(playerPos);

    // 移動方向を向くようにプレイヤーを回転（移動中の場合）
    if (moveVec.x != 0.0f || moveVec.z != 0.0f) {
        // 移動方向から角度を計算
        float playerAngle = atan2f(moveVec.x, moveVec.z);
        playerObject_->SetRotation({ 0.0f, playerAngle, 0.0f });
    }
}

void GamePlayScene::UpdateCamera() {
    Vector3 playerPos = playerObject_->GetPosition();
    Vector3 cameraPos;

    if (isFPSMode_) {
        // FPSモード - カメラはプレイヤーの目の高さに位置
        cameraPos = playerPos;
        cameraPos.y += 1.7f; // 目の高さ
    }
    else {
        // TPSモード - カメラはプレイヤーの後ろについていく
        float distance = 10.0f; // 距離を5.0fから10.0fに増加
        cameraPos.x = playerPos.x - sinf(cameraRotationY_) * distance;
        cameraPos.y = playerPos.y + 3.0f; // 以前の2.0fよりやや高く
        cameraPos.z = playerPos.z - cosf(cameraRotationY_) * distance;
    }

    // カメラの位置と回転を更新
    camera_->SetTranslate(cameraPos);
    camera_->SetRotate({ cameraRotationX_, cameraRotationY_, 0.0f });

    // カメラを更新
    camera_->Update();
}

void GamePlayScene::UpdateTrailEmitter() {
    // プレイヤーの現在の位置と回転を取得
    Vector3 playerPos = playerObject_->GetPosition();
    Vector3 playerRot = playerObject_->GetRotation();

    // プレイヤーの回転に基づいて後ろの位置を計算
    float offsetDistance = 0.5f;
    Vector3 emitterPos;
    emitterPos.x = playerPos.x - sinf(playerRot.y) * offsetDistance;
    emitterPos.y = playerPos.y + 0.5f; // 地面よりやや上
    emitterPos.z = playerPos.z - cosf(playerRot.y) * offsetDistance;

    // エミッタの位置を更新
    trailEmitter_->SetPosition(emitterPos);

    // 移動中の場合のみパーティクルを有効化
    bool isMoving = input_->PushKey(DIK_W) || input_->PushKey(DIK_A) ||
        input_->PushKey(DIK_S) || input_->PushKey(DIK_D);
    trailEmitter_->SetEmitting(isMoving);
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // 3Dオブジェクトの描画
    playerObject_->Draw();
    groundObject_->Draw();

    // スプライト共通設定をUIに設定
    spriteCommon_->CommonDraw();

    // UIの描画
    uiSprite_->Draw();

    // FOVテキストの描画
    fovTextSprite_->Draw();

    // ImGuiを使用して現在のFOV値とコントロールを表示
    ImGui::Begin("ゲームコントロール");
    ImGui::Text("FOV: %.1f 度", currentFovY_ * 57.3f); // ラジアンを度に変換
    ImGui::Text("カメラモード: %s", isFPSMode_ ? "一人称視点" : "三人称視点");
    ImGui::Separator();
    ImGui::Text("操作方法:");
    ImGui::Text("WASD - 移動");
    ImGui::Text("スペース - ジャンプ");
    ImGui::Text("F/G - FOV変更");
    ImGui::Text("1 - カメラモード切替");
    ImGui::Text("TAB - カーソル表示/非表示");
    ImGui::Text("ESC - タイトルに戻る");
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // シーン終了時にマウスカーソルを表示
    input_->SetMouseCursor(true);
}