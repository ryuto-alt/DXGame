// src/Engine/Physics/CollisionManager.h
#pragma once

#include <reactphysics3d/reactphysics3d.h>
#include <memory>
#include <unordered_map>
#include <string>
#include "Vector3.h"
#include "CollisionObject.h"

// 衝突タイプの列挙型
enum class CollisionType {
    Sphere,
    Box,
    Plane
};

// 衝突情報構造体
struct CollisionInfo {
    bool isColliding;                  // 衝突しているかどうか
    Vector3 collisionPoint;            // 衝突点
    Vector3 collisionNormal;           // 衝突法線（向き）
    float penetrationDepth;            // めり込み量
    CollisionObject* objectA;          // 衝突オブジェクトA
    CollisionObject* objectB;          // 衝突オブジェクトB

    // コンストラクタ
    CollisionInfo() : isColliding(false), collisionPoint(), collisionNormal(),
        penetrationDepth(0.0f), objectA(nullptr), objectB(nullptr) {
    }
};

// 衝突マネージャークラス
class CollisionManager {
public:
    // コンストラクタ/デストラクタ
    CollisionManager();
    ~CollisionManager();

    // インスタンス取得
    static CollisionManager* GetInstance();

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 終了処理
    void Finalize();

    // 衝突オブジェクト生成
    CollisionObject* CreateCollisionObject(
        const std::string& name,
        CollisionType type,
        const Vector3& position,
        const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
        const Vector3& scale = { 1.0f, 1.0f, 1.0f });

    // 衝突オブジェクト削除
    void DestroyCollisionObject(const std::string& name);

    // 衝突オブジェクト取得
    CollisionObject* GetCollisionObject(const std::string& name);

    // 衝突テスト
    CollisionInfo TestCollision(CollisionObject* objectA, CollisionObject* objectB);

    // デバッグ描画
    void DebugDraw();

    // ReactPhysics3D物理ワールドの取得
    reactphysics3d::PhysicsWorld* GetPhysicsWorld() { return physicsWorld_; }

    // ReactPhysics3D物理コモンの取得
    reactphysics3d::PhysicsCommon* GetPhysicsCommon() { return &physicsCommon_; }

private:
    // 静的インスタンス
    static CollisionManager* instance_;

    // ReactPhysics3D関連
    reactphysics3d::PhysicsCommon physicsCommon_;
    reactphysics3d::PhysicsWorld* physicsWorld_;

    // 衝突オブジェクト管理マップ
    std::unordered_map<std::string, std::unique_ptr<CollisionObject>> collisionObjects_;
};