#pragma once
#include "UnoEngine.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

// AABB構造体の定義
struct AABB {
    Vector3 min;
    Vector3 max;
};

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

    // 障害物管理
    std::vector<AABB> allObstacles_;
    std::vector<BoxCollider*> obstacleColliders_;
    std::ifstream objFileStream_; // OBJファイルストリーム

    // 衝突フラグ
    bool isColliding_ = false;

    // 移動速度
    float moveSpeed_ = 0.1f;

    // ジャンプ関連
    bool isJumping_ = false;
    float verticalVelocity_ = 0.0f;

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

    // OBJファイルからAABBを生成
    void UpdateStageAABB();

    // 障害物を追加
    void AddObstacle(std::vector<AABB>& obstacles, const Vector3& min, const Vector3& max);

    // 障害物用コライダーを作成
    void CreateObstacleColliders();
};