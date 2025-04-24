#pragma once
#include "UnoEngine.h"

class GamePlayScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    GamePlayScene();
    ~GamePlayScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // 初期化済みフラグ
    bool initialized_ = false;

    // 衝突判定用オブジェクト
    std::unique_ptr<Model> playerModel_;
    std::unique_ptr<Object3d> playerObject_;
    std::unique_ptr<Model> stageModel_;
    std::unique_ptr<Object3d> stageObject_;

    // コライダー
    SphereCollider* playerCollider_ = nullptr;
    BoxCollider* stageCollider_ = nullptr;

    // 衝突フラグ
    bool isColliding_ = false;

    // 移動速度
    float moveSpeed_ = 0.1f;

    // カメラ関連
    Vector3 cameraOffset_ = { 0.0f, 2.0f, -5.0f }; // プレイヤーからのカメラ位置オフセット
    float cameraDistance_ = 20.0f;                  // カメラの距離
    float cameraHeight_ = 2.0f;                    // カメラの高さ
    float cameraYaw_ = 0.0f;                       // カメラのヨー角（Y軸回転）
    float cameraPitch_ = 0.2f;                     // カメラのピッチ角（X軸回転）
    float cameraFollowSpeed_ = 0.5f;               // カメラの追従速度
    bool thirdPersonCamera_ = true;                // 三人称視点カメラフラグ

    // マウス関連
    float mouseSensitivity_ = 0.002f;              // マウス感度
    int prevMouseX_ = 0;                           // 前フレームのマウスX座標
    int prevMouseY_ = 0;                           // 前フレームのマウスY座標
    bool isFirstFrame_ = true;                     // 初回フレームフラグ
    bool isMouseRightPressed_ = false;             // マウス右ボタン押下フラグ

    // 衝突時のコールバック
    void OnPlayerCollision(const CollisionInfo& info);

    // カメラ更新処理
    void UpdateCamera();

    // マウス入力処理
    void ProcessMouseInput();

    // プレイヤー移動処理
    void MovePlayer();

    // ImGuiフォント設定
    void SetupImGuiFont();
};