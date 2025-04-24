// src/Engine/Physics/CollisionManager.cpp
#include "CollisionManager.h"
#include <cassert>
#include "ImGuiManager.h"  // ImGuiを使用するためのインクルード

// 静的メンバ変数の実体化
CollisionManager* CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance() {
    if (!instance_) {
        instance_ = new CollisionManager();
    }
    return instance_;
}

CollisionManager::CollisionManager()
    : physicsWorld_(nullptr) {
}

CollisionManager::~CollisionManager() {
    Finalize();
}

void CollisionManager::Initialize() {
    // ReactPhysics3Dの物理ワールド設定
    reactphysics3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = reactphysics3d::Vector3(0.0f, -9.81f, 0.0f);  // 重力設定
    settings.defaultVelocitySolverNbIterations = 10;  // 速度ソルバーの反復回数
    settings.isSleepingEnabled = true;  // スリープ機能を有効化

    // 物理ワールドの生成
    physicsWorld_ = physicsCommon_.createPhysicsWorld(settings);

    // デバッグ出力
    OutputDebugStringA("CollisionManager: Successfully initialized\n");
}

void CollisionManager::Update() {
    // 物理シミュレーションの更新（1/60秒を想定）
    if (physicsWorld_) {
        physicsWorld_->update(1.0f / 60.0f);
    }

    // 各衝突オブジェクトの更新
    for (auto& pair : collisionObjects_) {
        pair.second->Update();
    }
}

void CollisionManager::Finalize() {
    // 全ての衝突オブジェクトを削除
    collisionObjects_.clear();

    // 物理ワールドの削除
    if (physicsWorld_) {
        physicsCommon_.destroyPhysicsWorld(physicsWorld_);
        physicsWorld_ = nullptr;
    }

    // シングルトンインスタンスの削除
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

CollisionObject* CollisionManager::CreateCollisionObject(
    const std::string& name,
    CollisionType type,
    const Vector3& position,
    const Vector3& rotation,
    const Vector3& scale) {

    // 同名のオブジェクトが既に存在する場合はアサート
    assert(collisionObjects_.find(name) == collisionObjects_.end());

    // 新しい衝突オブジェクトを生成
    std::unique_ptr<CollisionObject> obj = std::make_unique<CollisionObject>(name, type);

    // 衝突オブジェクトの初期化
    obj->Initialize(this, position, rotation, scale);

    // マップに追加
    CollisionObject* objPtr = obj.get();
    collisionObjects_[name] = std::move(obj);

    // ポインタを返す
    return objPtr;
}

void CollisionManager::DestroyCollisionObject(const std::string& name) {
    // 指定された名前のオブジェクトが存在する場合は削除
    auto it = collisionObjects_.find(name);
    if (it != collisionObjects_.end()) {
        it->second->Finalize();  // ファイナライズを呼び出し
        collisionObjects_.erase(it);  // マップから削除
    }
}

CollisionObject* CollisionManager::GetCollisionObject(const std::string& name) {
    // 指定された名前のオブジェクトを検索
    auto it = collisionObjects_.find(name);
    if (it != collisionObjects_.end()) {
        return it->second.get();
    }
    // 見つからなかった場合はnullptr
    return nullptr;
}

CollisionInfo CollisionManager::TestCollision(CollisionObject* objectA, CollisionObject* objectB) {
    // nullptrチェック
    if (!objectA || !objectB) {
        return CollisionInfo();
    }

    // 衝突情報
    CollisionInfo info;
    info.objectA = objectA;
    info.objectB = objectB;

    // ReactPhysics3Dの衝突テスト
    reactphysics3d::CollisionBody* bodyA = objectA->GetCollisionBody();
    reactphysics3d::CollisionBody* bodyB = objectB->GetCollisionBody();

    // 衝突形状を取得
    reactphysics3d::Collider* colliderA = bodyA->getCollider(0);
    reactphysics3d::Collider* colliderB = bodyB->getCollider(0);

    // 衝突形状データの取得
    const reactphysics3d::CollisionShape* shapeA = colliderA->getCollisionShape();
    const reactphysics3d::CollisionShape* shapeB = colliderB->getCollisionShape();

    // 衝突アルゴリズム選択
    reactphysics3d::CollisionCallback::CallbackData callbackData;

    // 衝突テスト用の一時データ
    reactphysics3d::ContactPair contactPair;
    reactphysics3d::ContactPoint contactPoint;

    // 衝突テスト実行
    reactphysics3d::Collider* colliders[2] = { colliderA, colliderB };
    reactphysics3d::Transform transforms[2] = { bodyA->getTransform(), bodyB->getTransform() };
    bool isColliding = physicsWorld_->testCollision(colliders[0], transforms[0], colliders[1], transforms[1], callbackData);

    // 衝突結果の設定
    info.isColliding = isColliding;

    // 衝突点などの詳細情報を設定（衝突時のみ）
    if (isColliding && callbackData.contactPoints.size() > 0) {
        // 最初の衝突点情報を取得
        const reactphysics3d::ContactPoint& contact = callbackData.contactPoints[0];

        // 衝突点
        reactphysics3d::Vector3 point = contact.getWorldPointOnBody1();
        info.collisionPoint = { point.x, point.y, point.z };

        // 衝突法線
        reactphysics3d::Vector3 normal = contact.getNormal();
        info.collisionNormal = { normal.x, normal.y, normal.z };

        // めり込み量
        info.penetrationDepth = contact.getPenetrationDepth();
    }

    return info;
}

void CollisionManager::DebugDraw() {
    if (ImGui::Begin("Collision Debug")) {
        ImGui::Text("Physics Objects: %zu", collisionObjects_.size());

        // 各衝突オブジェクトの情報を表示
        for (auto& pair : collisionObjects_) {
            if (ImGui::TreeNode(pair.first.c_str())) {
                const Vector3& pos = pair.second->GetPosition();
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

                // 衝突タイプの表示
                const char* typeStr = "Unknown";
                switch (pair.second->GetCollisionType()) {
                case CollisionType::Sphere: typeStr = "Sphere"; break;
                case CollisionType::Box: typeStr = "Box"; break;
                case CollisionType::Plane: typeStr = "Plane"; break;
                }
                ImGui::Text("Type: %s", typeStr);

                // 衝突状態
                ImGui::Text("Colliding: %s", pair.second->IsColliding() ? "Yes" : "No");

                ImGui::TreePop();
            }
        }

        // 衝突ペアの表示
        if (ImGui::TreeNode("Collision Pairs")) {
            // すべての組み合わせをテスト（単純な実装）
            for (auto& pairA : collisionObjects_) {
                for (auto& pairB : collisionObjects_) {
                    // 同じオブジェクト同士はスキップ
                    if (pairA.first == pairB.first) continue;

                    // 衝突テスト
                    CollisionInfo info = TestCollision(pairA.second.get(), pairB.second.get());

                    // 衝突している場合のみ表示
                    if (info.isColliding) {
                        ImGui::Text("%s <-> %s (Depth: %.2f)",
                            pairA.first.c_str(),
                            pairB.first.c_str(),
                            info.penetrationDepth);
                    }
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}