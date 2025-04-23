#include "PlayerController.h"
#include "ParticleEffectsManager.h"

PlayerController::PlayerController() {
    // コンストラクタでデフォルト値の初期化
}

PlayerController::~PlayerController() {
    // リソースクリーンアップ（必要があれば）
}

void PlayerController::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Input* input) {
    // 依存関係の保存
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    input_ = input;

    // プレイヤーモデルの初期化
    playerModel_ = std::make_unique<Model>();
    playerModel_->Initialize(dxCommon_);
    playerModel_->LoadFromObj("Resources/models", "player.obj");

    // プレイヤーオブジェクトの初期化
    playerObject_ = std::make_unique<Object3d>();
    playerObject_->Initialize(dxCommon_, spriteCommon_);
    playerObject_->SetModel(playerModel_.get());
    playerObject_->SetScale(scale_);
    playerObject_->SetPosition(position_);
    playerObject_->SetRotation(rotation_);

    // プレイヤーのバウンディングボックス初期化
    playerBoundingBox.min = { position_.x - playerRadius_, position_.y - playerHeight_ / 2, position_.z - playerRadius_ };
    playerBoundingBox.max = { position_.x + playerRadius_, position_.y + playerHeight_ / 2, position_.z + playerRadius_ };

    // デバッグ出力
    OutputDebugStringA("PlayerController: 初期化完了\n");
}

void PlayerController::Update() {
    // 重力の適用
    ApplyGravity();

    // ステージとの衝突判定と解決
    HandleCollisions();

    // 3Dオブジェクトの更新
    playerObject_->SetPosition(position_);
    playerObject_->SetRotation(rotation_);
    playerObject_->Update();
}

void PlayerController::Draw() {
    // プレイヤーモデルの描画
    if (playerObject_) {
        playerObject_->Draw();
    }
}

void PlayerController::Move(float cameraRotationY) {
    // カメラの回転に基づいて前方と右方向のベクトルを計算
    float forwardX = sinf(cameraRotationY);
    float forwardZ = cosf(cameraRotationY);
    float rightX = cosf(cameraRotationY);
    float rightZ = -sinf(cameraRotationY);

    // 移動ベクトルをリセット
    moveVector_ = { 0.0f, 0.0f, 0.0f };
    isMoving_ = false;

    // キーボード入力による移動処理
    if (input_->PushKey(DIK_W)) {
        moveVector_.x += forwardX * moveSpeed_;
        moveVector_.z += forwardZ * moveSpeed_;
        isMoving_ = true;
    }
    if (input_->PushKey(DIK_S)) {
        moveVector_.x -= forwardX * moveSpeed_;
        moveVector_.z -= forwardZ * moveSpeed_;
        isMoving_ = true;
    }
    if (input_->PushKey(DIK_A)) {
        moveVector_.x -= rightX * moveSpeed_;
        moveVector_.z -= rightZ * moveSpeed_;
        isMoving_ = true;
    }
    if (input_->PushKey(DIK_D)) {
        moveVector_.x += rightX * moveSpeed_;
        moveVector_.z += rightZ * moveSpeed_;
        isMoving_ = true;
    }

    // ジャンプ入力の処理
    if (input_->TriggerKey(DIK_SPACE) && onGround_) {
        Jump();
    }

    // 移動前の位置を保存
    Vector3 originalPosition = position_;

    // 位置に移動を適用
    position_.x += moveVector_.x;
    position_.z += moveVector_.z;

    // 移動中なら、移動方向を向くようにプレイヤーの回転を更新
    if (isMoving_) {
        // 移動方向から角度を計算
        float playerAngle = atan2f(moveVector_.x, moveVector_.z);
        rotation_.y = playerAngle;
    }
}

void PlayerController::Jump() {
    if (onGround_) {
        isJumping_ = true;
        onGround_ = false;
        verticalVelocity_ = jumpPower_;
    }
}

void PlayerController::ApplyGravity() {
    // 重力を垂直速度に適用
    verticalVelocity_ -= gravity_;

    // 垂直位置を更新
    position_.y += verticalVelocity_;
}

void PlayerController::SetPosition(const Vector3& position) {
    position_ = position;
    if (playerObject_) {
        playerObject_->SetPosition(position_);
    }
}

void PlayerController::SetStageModel(Model* stageModel) {
    stageModel_ = stageModel;

    // ステージモデルから衝突境界を抽出
    if (stageModel_) {
        CollisionDetection::ExtractStageBoundaries(stageModel_);
        OutputDebugStringA("PlayerController: ステージモデルの当たり判定を設定しました\n");
    }
}

void PlayerController::HandleCollisions() {
    // 調整後の位置を格納する変数
    Vector3 adjustedPosition;

    // プレイヤーのバウンディングボックスを更新
    playerBoundingBox.min = { position_.x - playerRadius_, position_.y - playerHeight_ / 2, position_.z - playerRadius_ };
    playerBoundingBox.max = { position_.x + playerRadius_, position_.y + playerHeight_ / 2, position_.z + playerRadius_ };

    // 地面との衝突判定
    bool groundCollision = CollisionDetection::CheckGroundCollision(
        position_, playerRadius_, playerHeight_, adjustedPosition);

    // 地面との衝突があれば位置を調整
    if (groundCollision) {
        position_ = adjustedPosition;

        // 地面に着地したら、垂直速度をリセットして接地状態にする
        if (verticalVelocity_ <= 0.0f) {
            onGround_ = true;
            verticalVelocity_ = 0.0f;
        }
    }

    // 障害物との衝突判定
    bool obstacleCollision = CollisionDetection::CheckObstacleCollision(
        position_, playerRadius_, adjustedPosition);

    // 障害物との衝突があれば位置を調整
    if (obstacleCollision) {
        position_ = adjustedPosition;
    }

    // ステージの境界を取得
    const BoundingBox& stageBounds = CollisionDetection::GetStageBounds();

    // ステージ境界内に収める
    if (position_.x - playerRadius_ < stageBounds.min.x) {
        position_.x = stageBounds.min.x + playerRadius_;
    }
    else if (position_.x + playerRadius_ > stageBounds.max.x) {
        position_.x = stageBounds.max.x - playerRadius_;
    }

    if (position_.z - playerRadius_ < stageBounds.min.z) {
        position_.z = stageBounds.min.z + playerRadius_;
    }
    else if (position_.z + playerRadius_ > stageBounds.max.z) {
        position_.z = stageBounds.max.z - playerRadius_;
    }

    // ステージ下限より下に落ちないようにする
    if (position_.y < stageBounds.min.y) {
        position_.y = stageBounds.min.y;
        verticalVelocity_ = 0.0f;
        onGround_ = true;
    }
}