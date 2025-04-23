#pragma once

#include "UnoEngine.h"
#include "CollisionDetection.h" // 当たり判定システム用のヘッダー

// 前方宣言
class ParticleEffectsManager;

class PlayerController {
public:
    // コンストラクタ/デストラクタ
    PlayerController();
    ~PlayerController();

    // 基本メソッド
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Input* input);
    void Update();
    void Draw();

    // 移動関連メソッド
    void Move(float cameraRotationY);
    void Jump();
    void ApplyGravity();

    // 衝突判定処理メソッド
    void HandleCollisions();

    // アクセサ
    const Vector3& GetPosition() const { return position_; }
    void SetPosition(const Vector3& position);
    const Vector3& GetRotation() const { return rotation_; }
    bool IsMoving() const { return isMoving_; }
    bool IsJumping() const { return isJumping_; }
    Object3d* GetObject3d() const { return playerObject_.get(); }

    // プレイヤーの物理特性
    float GetRadius() const { return playerRadius_; }
    float GetHeight() const { return playerHeight_; }

    // 依存関係の設定
    void SetParticleEffects(ParticleEffectsManager* particleEffects) { particleEffects_ = particleEffects; }

    // ステージモデルの設定（当たり判定用）
    void SetStageModel(Model* stageModel);

private:
    // 3Dモデルとオブジェクト
    std::unique_ptr<Model> playerModel_;
    std::unique_ptr<Object3d> playerObject_;

    // 移動パラメータ
    Vector3 position_ = { 0.0f, 5.0f, 0.0f };  // 開始位置を少し高くする
    Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
    Vector3 scale_ = { 1.0f, 1.0f, 1.0f };
    float moveSpeed_ = 0.5f;
    Vector3 moveVector_ = { 0.0f, 0.0f, 0.0f };
    bool isMoving_ = false;

    // ジャンプパラメータ
    bool isJumping_ = false;
    bool onGround_ = false;  // 地面に接地しているか
    float verticalVelocity_ = 0.0f;
    float jumpPower_ = 0.45f;
    float gravity_ = 0.01f;

    // 当たり判定パラメータ
    float playerRadius_ = 0.5f;  // プレイヤーの半径
    float playerHeight_ = 2.0f;  // プレイヤーの高さ
    BoundingBox playerBoundingBox;  // プレイヤーのバウンディングボックス

    // 依存関係
    Input* input_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    ParticleEffectsManager* particleEffects_ = nullptr;
    Model* stageModel_ = nullptr;  // ステージモデル参照（当たり判定用）
};