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

protected:
    // 初期化済みフラグ
    bool initialized_ = false;

    // プレイヤー関連
    std::shared_ptr<Collision::SphereCollider> playerCollider;
    std::unique_ptr<Model> playerModel;
    std::unique_ptr<Object3d> playerObject;
    float playerSpeed = 0.1f;
    bool playerCollisionDetected = false; // 衝突検知フラグ

    // 敵関連
    std::shared_ptr<Collision::CapsuleCollider> enemyCollider;
    std::unique_ptr<Model> enemyModel;
    std::unique_ptr<Object3d> enemyObject;
    bool enemyCollisionDetected = false; // 衝突検知フラグ

    // 障害物関連
    std::vector<std::shared_ptr<Collision::CollisionObject>> obstacles;
    std::vector<std::unique_ptr<Model>> obstacleModels;
    std::vector<std::unique_ptr<Object3d>> obstacleObjects;
    std::vector<bool> obstacleCollisionDetected; // 各障害物の衝突検知フラグ

    // カメラ視点関連
    float cameraYaw = 0.0f;   // カメラの水平角度
    float cameraPitch = 0.3f; // カメラの垂直角度（初期値は少し下向き）
    float cameraDistance = 10.0f; // カメラとプレイヤーの距離
    Vector2 lastMousePos = { 0.0f, 0.0f }; // 前回のマウス位置
    bool isFirstMouseInput = true; // 初回マウス入力フラグ

    // コリジョンハンドラー
    void PlayerCollisionHandler(Collision::CollisionObject* other, const Collision::CollisionResult& result);
    void EnemyCollisionHandler(Collision::CollisionObject* other, const Collision::CollisionResult& result);

    // プレイヤー移動処理
    void UpdatePlayerMovement();

    // カメラ更新処理
    void UpdateCamera();
};