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

    // 敵関連
    std::shared_ptr<Collision::CapsuleCollider> enemyCollider;
    std::unique_ptr<Model> enemyModel;
    std::unique_ptr<Object3d> enemyObject;

    // 障害物関連
    std::vector<std::shared_ptr<Collision::CollisionObject>> obstacles;
    std::vector<std::unique_ptr<Model>> obstacleModels;
    std::vector<std::unique_ptr<Object3d>> obstacleObjects;

    // コリジョンハンドラー
    void PlayerCollisionHandler(Collision::CollisionObject* other, const Collision::CollisionResult& result);
    void EnemyCollisionHandler(Collision::CollisionObject* other, const Collision::CollisionResult& result);

    // プレイヤー移動処理
    void UpdatePlayerMovement();
};