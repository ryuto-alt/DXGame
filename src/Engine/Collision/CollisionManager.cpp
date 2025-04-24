#include "CollisionManager.h"
#include <algorithm>

namespace Collision {

    // 静的メンバ変数の初期化
    uint32_t CollisionObject::nextID_ = 0;
    CollisionManager* CollisionManager::instance_ = nullptr;

    CollisionManager* CollisionManager::GetInstance() {
        if (!instance_) {
            instance_ = new CollisionManager();
        }
        return instance_;
    }

    void CollisionManager::AddCollider(std::shared_ptr<CollisionObject> collider) {
        // 既に登録されているかチェック
        auto it = std::find_if(colliders_.begin(), colliders_.end(),
            [collider](const std::shared_ptr<CollisionObject>& existing) {
                return existing->GetID() == collider->GetID();
            });

        // 登録されていなければ追加
        if (it == colliders_.end()) {
            colliders_.push_back(collider);
        }
    }

    void CollisionManager::RemoveCollider(std::shared_ptr<CollisionObject> collider) {
        RemoveCollider(collider->GetID());
    }

    void CollisionManager::RemoveCollider(uint32_t id) {
        // 指定IDのコライダーを削除
        auto it = std::remove_if(colliders_.begin(), colliders_.end(),
            [id](const std::shared_ptr<CollisionObject>& collider) {
                return collider->GetID() == id;
            });

        // 実際に削除
        if (it != colliders_.end()) {
            colliders_.erase(it, colliders_.end());
        }
    }

    void CollisionManager::ClearColliders() {
        colliders_.clear();
    }

    void CollisionManager::Update(float deltaTime) {
        // すべてのコライダーの組み合わせで衝突判定
        for (size_t i = 0; i < colliders_.size(); ++i) {
            // 無効なコライダーはスキップ
            if (!colliders_[i]->IsEnabled()) continue;

            for (size_t j = i + 1; j < colliders_.size(); ++j) {
                // 無効なコライダーはスキップ
                if (!colliders_[j]->IsEnabled()) continue;

                // 衝突判定
                CollisionResult result;

                // 両方とも剛体の場合や、少なくとも一方が速度を持つ場合はスウィープテストを行う
                if ((colliders_[i]->IsRigidbody() && colliders_[j]->IsRigidbody()) ||
                    Utility::Length(colliders_[i]->GetVelocity()) > 0.0001f ||
                    Utility::Length(colliders_[j]->GetVelocity()) > 0.0001f) {

                    // 動いているオブジェクトを優先してスウィープテスト
                    if (Utility::Length(colliders_[i]->GetVelocity()) >
                        Utility::Length(colliders_[j]->GetVelocity())) {
                        result = CheckSweepCollision(colliders_[i].get(), colliders_[j].get(), deltaTime);
                    }
                    else {
                        result = CheckSweepCollision(colliders_[j].get(), colliders_[i].get(), deltaTime);
                        // 法線の向きを反転
                        if (result.isColliding) {
                            result.normal = Utility::Multiply(result.normal, -1.0f);
                        }
                    }
                }
                else {
                    // 通常の衝突判定
                    result = CheckCollision(colliders_[i].get(), colliders_[j].get());
                }

                // 衝突していれば通知
                if (result.isColliding) {
                    // コライダー1のコールバックを呼び出し
                    if (colliders_[i]->onCollisionEnter) {
                        colliders_[i]->onCollisionEnter(colliders_[j].get(), result);
                    }

                    // コライダー2のコールバックを呼び出し（法線の向きを反転）
                    if (colliders_[j]->onCollisionEnter) {
                        // 法線の向きを反転
                        CollisionResult reversedResult = result;
                        reversedResult.normal = Utility::Multiply(result.normal, -1.0f);

                        colliders_[j]->onCollisionEnter(colliders_[i].get(), reversedResult);
                    }
                }
            }
        }
    }

    CollisionResult CollisionManager::CheckCollision(
        const CollisionObject* collider1,
        const CollisionObject* collider2
    ) {
        CollisionResult result;

        // 形状の種類に応じて適切な衝突判定関数を呼び出す
        if (collider1->GetShapeType() == CollisionObject::ShapeType::Sphere &&
            collider2->GetShapeType() == CollisionObject::ShapeType::Sphere) {

            // 球 vs 球
            const Sphere* sphere1 = static_cast<const Sphere*>(collider1->GetShapeData());
            const Sphere* sphere2 = static_cast<const Sphere*>(collider2->GetShapeData());

            result = CollisionDetector::CheckSphereToSphere(*sphere1, *sphere2);
        }
        else if (collider1->GetShapeType() == CollisionObject::ShapeType::Sphere &&
            collider2->GetShapeType() == CollisionObject::ShapeType::Capsule) {

            // 球 vs カプセル
            const Sphere* sphere = static_cast<const Sphere*>(collider1->GetShapeData());
            const Capsule* capsule = static_cast<const Capsule*>(collider2->GetShapeData());

            result = CollisionDetector::CheckSphereToCapusle(*sphere, *capsule);
        }
        else if (collider1->GetShapeType() == CollisionObject::ShapeType::Capsule &&
            collider2->GetShapeType() == CollisionObject::ShapeType::Sphere) {

            // カプセル vs 球
            const Capsule* capsule = static_cast<const Capsule*>(collider1->GetShapeData());
            const Sphere* sphere = static_cast<const Sphere*>(collider2->GetShapeData());

            result = CollisionDetector::CheckSphereToCapusle(*sphere, *capsule);

            // 法線の向きを反転（球からカプセルへの向きになっているため）
            if (result.isColliding) {
                result.normal = Utility::Multiply(result.normal, -1.0f);
            }
        }
        else if (collider1->GetShapeType() == CollisionObject::ShapeType::Capsule &&
            collider2->GetShapeType() == CollisionObject::ShapeType::Capsule) {

            // カプセル vs カプセル
            const Capsule* capsule1 = static_cast<const Capsule*>(collider1->GetShapeData());
            const Capsule* capsule2 = static_cast<const Capsule*>(collider2->GetShapeData());

            result = CollisionDetector::CheckCapsuleToCapsule(*capsule1, *capsule2);
        }

        return result;
    }

    CollisionResult CollisionManager::CheckSweepCollision(
        const CollisionObject* movingCollider,
        const CollisionObject* staticCollider,
        float deltaTime
    ) {
        CollisionResult result;

        // 移動オブジェクトの速度
        const Vector3& velocity = movingCollider->GetVelocity();

        // 形状の種類に応じて適切なスウィープテスト関数を呼び出す
        if (movingCollider->GetShapeType() == CollisionObject::ShapeType::Sphere) {
            const Sphere* movingSphere = static_cast<const Sphere*>(movingCollider->GetShapeData());

            if (staticCollider->GetShapeType() == CollisionObject::ShapeType::Sphere) {
                // 球 vs 球（スウィープ）
                const Sphere* staticSphere = static_cast<const Sphere*>(staticCollider->GetShapeData());
                result = CollisionDetector::CheckSphereSweepToSphere(*movingSphere, velocity, *staticSphere, deltaTime);
            }
            else if (staticCollider->GetShapeType() == CollisionObject::ShapeType::Capsule) {
                // 球 vs カプセル（スウィープ）
                const Capsule* staticCapsule = static_cast<const Capsule*>(staticCollider->GetShapeData());
                result = CollisionDetector::CheckSphereSweepToCapsule(*movingSphere, velocity, *staticCapsule, deltaTime);
            }
        }
        else {
            // その他の組み合わせは未実装
            // 通常の衝突判定を使用
            result = CheckCollision(movingCollider, staticCollider);
        }

        return result;
    }

    void CollisionManager::DebugDraw() {
        // Debug描画は別途実装
        // 今回は基本実装のみなのでスキップ
    }

}