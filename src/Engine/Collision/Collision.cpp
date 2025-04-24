// src/Engine/Collision/Collision.cpp
#include "Collision.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <cassert>

namespace Collision {

    //-----------------------------------------------------------------------------
    // シングルトンインスタンス
    //-----------------------------------------------------------------------------
    CollisionManager* CollisionManager::instance_ = nullptr;

    //-----------------------------------------------------------------------------
    // SphereCollider実装
    //-----------------------------------------------------------------------------

    SphereCollider::SphereCollider(const Vector3& center, float radius) {
        sphere_.center = center;
        sphere_.radius = radius;
    }

    bool SphereCollider::CheckCollision(ICollider* other, CollisionResult* result) {
        // コライダータイプによって処理を分岐
        switch (other->GetType()) {
        case ColliderType::Sphere:
            return CollisionDetection::SphereSphere(
                sphere_,
                static_cast<SphereCollider*>(other)->GetSphere(),
                result);

        case ColliderType::AABB:
            return CollisionDetection::SphereAABB(
                sphere_,
                static_cast<AABBCollider*>(other)->GetAABB(),
                result);

        case ColliderType::OBB:
            return CollisionDetection::SphereOBB(
                sphere_,
                static_cast<OBBCollider*>(other)->GetOBB(),
                result);

        case ColliderType::Mesh:
        case ColliderType::ConvexMesh:
        case ColliderType::Heightfield:
            return CollisionDetection::SphereMesh(
                sphere_,
                *static_cast<MeshCollider*>(other)->GetMeshCollider(),
                result);

        default:
            return false;
        }
    }

    //-----------------------------------------------------------------------------
    // AABBCollider実装
    //-----------------------------------------------------------------------------

    AABBCollider::AABBCollider(const Vector3& min, const Vector3& max) {
        aabb_.min = min;
        aabb_.max = max;
    }

    bool AABBCollider::CheckCollision(ICollider* other, CollisionResult* result) {
        // コライダータイプによって処理を分岐
        switch (other->GetType()) {
        case ColliderType::Sphere:
            return CollisionDetection::SphereAABB(
                static_cast<SphereCollider*>(other)->GetSphere(),
                aabb_,
                result);

        case ColliderType::AABB:
            return CollisionDetection::AABBAABB(
                aabb_,
                static_cast<AABBCollider*>(other)->GetAABB(),
                result);

        case ColliderType::OBB:
            return CollisionDetection::AABBOBB(
                aabb_,
                static_cast<OBBCollider*>(other)->GetOBB(),
                result);

        case ColliderType::Mesh:
        case ColliderType::ConvexMesh:
        case ColliderType::Heightfield:
            return CollisionDetection::AABBMesh(
                aabb_,
                *static_cast<MeshCollider*>(other)->GetMeshCollider(),
                result);

        default:
            return false;
        }
    }

    //-----------------------------------------------------------------------------
    // OBBCollider実装
    //-----------------------------------------------------------------------------

    OBBCollider::OBBCollider(const Vector3& center, const Vector3 orientations[3], const Vector3& size) {
        obb_.center = center;
        obb_.orientations[0] = orientations[0];
        obb_.orientations[1] = orientations[1];
        obb_.orientations[2] = orientations[2];
        obb_.size = size;
    }

    OBBCollider::OBBCollider(const Matrix4x4& worldMatrix, const Vector3& size) {
        obb_ = OBB::CreateFromMatrix(worldMatrix, size);
    }

    bool OBBCollider::CheckCollision(ICollider* other, CollisionResult* result) {
        // コライダータイプによって処理を分岐
        switch (other->GetType()) {
        case ColliderType::Sphere:
            return CollisionDetection::SphereOBB(
                static_cast<SphereCollider*>(other)->GetSphere(),
                obb_,
                result);

        case ColliderType::AABB:
            return CollisionDetection::AABBOBB(
                static_cast<AABBCollider*>(other)->GetAABB(),
                obb_,
                result);

        case ColliderType::OBB:
            return CollisionDetection::OBBOBB(
                obb_,
                static_cast<OBBCollider*>(other)->GetOBB(),
                result);

        case ColliderType::Mesh:
        case ColliderType::ConvexMesh:
        case ColliderType::Heightfield:
            return CollisionDetection::OBBMesh(
                obb_,
                *static_cast<MeshCollider*>(other)->GetMeshCollider(),
                result);

        default:
            return false;
        }
    }

    void OBBCollider::UpdateFromMatrix(const Matrix4x4& worldMatrix, const Vector3& size) {
        obb_ = OBB::CreateFromMatrix(worldMatrix, size);
    }

    //-----------------------------------------------------------------------------
    // MeshCollider実装
    //-----------------------------------------------------------------------------

    bool MeshCollider::CheckCollision(ICollider* other, CollisionResult* result) {
        // メッシュコライダーが設定されていない場合は常にfalse
        if (!meshCollider_) {
            return false;
        }

        // コライダータイプによって処理を分岐
        switch (other->GetType()) {
        case ColliderType::Sphere:
            return CollisionDetection::SphereMesh(
                static_cast<SphereCollider*>(other)->GetSphere(),
                *meshCollider_,
                result);

        case ColliderType::AABB:
            return CollisionDetection::AABBMesh(
                static_cast<AABBCollider*>(other)->GetAABB(),
                *meshCollider_,
                result);

        case ColliderType::OBB:
            return CollisionDetection::OBBMesh(
                static_cast<OBBCollider*>(other)->GetOBB(),
                *meshCollider_,
                result);

        case ColliderType::Mesh:
        case ColliderType::ConvexMesh:
        case ColliderType::Heightfield:
            return meshCollider_->Intersects(
                *static_cast<MeshCollider*>(other)->GetMeshCollider(),
                result);

        default:
            return false;
        }
    }

    void MeshCollider::CreateTriangleMeshCollider(const std::vector<Triangle>& triangles) {
        meshCollider_ = std::make_shared<TriangleMeshCollider>(triangles);
    }

    void MeshCollider::CreateTriangleMeshCollider(const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices) {
        meshCollider_ = std::make_shared<TriangleMeshCollider>(vertices, indices);
    }

    void MeshCollider::CreateConvexMeshCollider(const std::vector<Vector3>& vertices) {
        meshCollider_ = std::make_shared<ConvexMeshCollider>(vertices);
    }

    void MeshCollider::CreateHeightfieldCollider(int width, int height, const std::vector<float>& heights,
        float scaleX, float scaleY, float scaleZ) {
        meshCollider_ = std::make_shared<HeightfieldCollider>(width, height, heights, scaleX, scaleY, scaleZ);
    }

    void MeshCollider::UpdateFromMatrix(const Matrix4x4& worldMatrix) {
        if (meshCollider_) {
            meshCollider_->ApplyTransform(worldMatrix);
        }
    }

    //-----------------------------------------------------------------------------
    // CollisionManager実装
    //-----------------------------------------------------------------------------

    CollisionManager* CollisionManager::GetInstance() {
        if (!instance_) {
            instance_ = new CollisionManager();
        }
        return instance_;
    }

    void CollisionManager::DeleteInstance() {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    void CollisionManager::Initialize() {
        // コライダーリストのクリア
        colliders_.clear();
        colliderMap_.clear();
        colliderIDMap_.clear();
        prevCollisions_.clear();
        ignorePairs_.clear();

        // 次のコライダーIDをリセット
        nextColliderID_ = 1;
    }

    void CollisionManager::Finalize() {
        // すべてのリソースを解放
        colliders_.clear();
        colliderMap_.clear();
        colliderIDMap_.clear();
        prevCollisions_.clear();
        ignorePairs_.clear();
    }

    void CollisionManager::AddCollider(std::shared_ptr<ICollider> collider) {
        // パラメータチェック
        if (!collider) {
            return;
        }

        // 既に登録されているかチェック
        if (collider->id_ != 0) {
            return;
        }

        // IDを割り当て
        collider->id_ = nextColliderID_++;

        // コライダーリストに追加
        colliders_.push_back(collider);

        // 名前が設定されている場合は名前マップにも追加
        if (!collider->name_.empty()) {
            colliderMap_[collider->name_] = collider;
        }

        // IDマップに追加
        colliderIDMap_[collider->id_] = collider;
    }

    void CollisionManager::RemoveCollider(uint32_t colliderID) {
        // IDからコライダーを検索
        auto it = colliderIDMap_.find(colliderID);
        if (it == colliderIDMap_.end()) {
            return;
        }

        // 名前マップから削除
        if (!it->second->name_.empty()) {
            colliderMap_.erase(it->second->name_);
        }

        // コライダーリストから削除
        auto listIt = std::find(colliders_.begin(), colliders_.end(), it->second);
        if (listIt != colliders_.end()) {
            colliders_.erase(listIt);
        }

        // IDマップから削除
        colliderIDMap_.erase(it);
    }

    void CollisionManager::RemoveCollider(const std::string& name) {
        // 名前からコライダーを検索
        auto it = colliderMap_.find(name);
        if (it == colliderMap_.end()) {
            return;
        }

        // IDマップから削除
        colliderIDMap_.erase(it->second->id_);

        // コライダーリストから削除
        auto listIt = std::find(colliders_.begin(), colliders_.end(), it->second);
        if (listIt != colliders_.end()) {
            colliders_.erase(listIt);
        }

        // 名前マップから削除
        colliderMap_.erase(it);
    }

    void CollisionManager::RemoveCollider(ICollider* collider) {
        if (!collider) {
            return;
        }

        RemoveCollider(collider->id_);
    }

    std::shared_ptr<ICollider> CollisionManager::FindCollider(uint32_t colliderID) {
        // IDからコライダーを検索
        auto it = colliderIDMap_.find(colliderID);
        if (it == colliderIDMap_.end()) {
            return nullptr;
        }

        return it->second;
    }

    std::shared_ptr<ICollider> CollisionManager::FindCollider(const std::string& name) {
        // 名前からコライダーを検索
        auto it = colliderMap_.find(name);
        if (it == colliderMap_.end()) {
            return nullptr;
        }

        return it->second;
    }

    void CollisionManager::CheckAllCollisions() {
        // 現在の衝突状態をクリア（前回の状態を保存するため、マップは消去しない）
        std::unordered_map<uint64_t, bool> currentCollisions;

        // すべてのコライダーの組み合わせをチェック
        for (size_t i = 0; i < colliders_.size(); i++) {
            for (size_t j = i + 1; j < colliders_.size(); j++) {
                // 両方のコライダーが有効かチェック
                if (!colliders_[i]->IsEnabled() || !colliders_[j]->IsEnabled()) {
                    continue;
                }

                // フィルターのチェック
                if (!colliders_[i]->filter_.CanCollide(colliders_[j]->filter_)) {
                    continue;
                }

                // 無視ペアのチェック
                uint64_t pairHash = GetColliderPairHash(colliders_[i]->id_, colliders_[j]->id_);
                if (ignorePairs_.find(pairHash) != ignorePairs_.end()) {
                    continue;
                }

                // 衝突チェック
                CollisionResult result;
                bool isColliding = colliders_[i]->CheckCollision(colliders_[j].get(), &result);

                // 衝突状態を記録
                currentCollisions[pairHash] = isColliding;

                // 前回の衝突状態を取得
                bool wasColliding = prevCollisions_.find(pairHash) != prevCollisions_.end() && prevCollisions_[pairHash];

                // イベント通知
                if (isColliding) {
                    // 衝突情報を作成
                    CollisionInfo info;
                    info.pCollider1 = colliders_[i].get();
                    info.pCollider2 = colliders_[j].get();
                    info.collider1ID = colliders_[i]->id_;
                    info.collider2ID = colliders_[j]->id_;
                    info.collider1Name = colliders_[i]->name_;
                    info.collider2Name = colliders_[j]->name_;
                    info.result = result;

                    if (!wasColliding) {
                        // 衝突開始
                        info.event = CollisionEvent::Enter;
                        colliders_[i]->OnCollision(info);

                        // 相手側の衝突情報を作成
                        std::swap(info.pCollider1, info.pCollider2);
                        std::swap(info.collider1ID, info.collider2ID);
                        std::swap(info.collider1Name, info.collider2Name);
                        colliders_[j]->OnCollision(info);
                    }
                    else {
                        // 衝突中
                        info.event = CollisionEvent::Stay;
                        colliders_[i]->OnCollision(info);

                        // 相手側の衝突情報を作成
                        std::swap(info.pCollider1, info.pCollider2);
                        std::swap(info.collider1ID, info.collider2ID);
                        std::swap(info.collider1Name, info.collider2Name);
                        colliders_[j]->OnCollision(info);
                    }
                }
                else if (wasColliding) {
                    // 衝突終了
                    CollisionInfo info;
                    info.pCollider1 = colliders_[i].get();
                    info.pCollider2 = colliders_[j].get();
                    info.collider1ID = colliders_[i]->id_;
                    info.collider2ID = colliders_[j]->id_;
                    info.collider1Name = colliders_[i]->name_;
                    info.collider2Name = colliders_[j]->name_;
                    info.event = CollisionEvent::Exit;
                    colliders_[i]->OnCollision(info);

                    // 相手側の衝突情報を作成
                    std::swap(info.pCollider1, info.pCollider2);
                    std::swap(info.collider1ID, info.collider2ID);
                    std::swap(info.collider1Name, info.collider2Name);
                    colliders_[j]->OnCollision(info);
                }
            }
        }

        // 現在の衝突状態を前回の状態として保存
        prevCollisions_ = currentCollisions;
    }

    void CollisionManager::ClearCollisions() {
        prevCollisions_.clear();
    }

    void CollisionManager::AddIgnorePair(uint32_t collider1ID, uint32_t collider2ID) {
        ignorePairs_.insert(GetColliderPairHash(collider1ID, collider2ID));
    }

    void CollisionManager::AddIgnorePair(const std::string& collider1Name, const std::string& collider2Name) {
        // 名前からコライダーを検索
        auto it1 = colliderMap_.find(collider1Name);
        auto it2 = colliderMap_.find(collider2Name);

        if (it1 == colliderMap_.end() || it2 == colliderMap_.end()) {
            return;
        }

        // 無視ペアに追加
        AddIgnorePair(it1->second->id_, it2->second->id_);
    }

    void CollisionManager::RemoveIgnorePair(uint32_t collider1ID, uint32_t collider2ID) {
        ignorePairs_.erase(GetColliderPairHash(collider1ID, collider2ID));
    }

    void CollisionManager::RemoveIgnorePair(const std::string& collider1Name, const std::string& collider2Name) {
        // 名前からコライダーを検索
        auto it1 = colliderMap_.find(collider1Name);
        auto it2 = colliderMap_.find(collider2Name);

        if (it1 == colliderMap_.end() || it2 == colliderMap_.end()) {
            return;
        }

        // 無視ペアから削除
        RemoveIgnorePair(it1->second->id_, it2->second->id_);
    }

    void CollisionManager::DebugDraw() {
        // デバッグ描画の実装（現在は空）
        // 実際のデバッグ描画は、UnoEngineの描画機能を使用して別途実装する
    }

    uint64_t CollisionManager::GetColliderPairHash(uint32_t collider1ID, uint32_t collider2ID) const {
        // 小さい方を前に、大きい方を後ろに配置して一貫性を保つ
        if (collider1ID > collider2ID) {
            std::swap(collider1ID, collider2ID);
        }

        // 64ビットハッシュ値を作成
        return (static_cast<uint64_t>(collider1ID) << 32) | static_cast<uint64_t>(collider2ID);
    }

    //-----------------------------------------------------------------------------
    // 衝突判定関数群の実装
    //-----------------------------------------------------------------------------
    namespace CollisionDetection {

        // 球と球の衝突判定
        bool SphereSphere(const Sphere& sphere1, const Sphere& sphere2, CollisionResult* result) {
            // 中心間のベクトルを計算
            Vector3 diff = {
                sphere2.center.x - sphere1.center.x,
                sphere2.center.y - sphere1.center.y,
                sphere2.center.z - sphere1.center.z
            };

            // 中心間の距離の二乗を計算
            float distSquared =
                diff.x * diff.x +
                diff.y * diff.y +
                diff.z * diff.z;

            // 半径の和の二乗を計算
            float radiusSum = sphere1.radius + sphere2.radius;
            float radiusSumSquared = radiusSum * radiusSum;

            // 距離が半径の和より小さければ衝突
            if (distSquared <= radiusSumSquared) {
                if (result) {
                    // 衝突情報を設定
                    result->hasCollision = true;

                    // 中心間の距離を計算
                    float distance = std::sqrt(distSquared);

                    // 法線を計算（球2から球1への単位ベクトル）
                    if (distance > 0.0001f) {
                        result->normal.x = -diff.x / distance;
                        result->normal.y = -diff.y / distance;
                        result->normal.z = -diff.z / distance;
                    }
                    else {
                        // 中心が重なっている場合、上向きを仮定
                        result->normal = { 0.0f, 1.0f, 0.0f };
                    }

                    // めり込み量を計算
                    result->penetration = radiusSum - distance;

                    // 衝突点を計算（球1の中心から半径分だけ法線方向に進んだ点）
                    result->collisionPoint.x = sphere1.center.x - result->normal.x * sphere1.radius;
                    result->collisionPoint.y = sphere1.center.y - result->normal.y * sphere1.radius;
                    result->collisionPoint.z = sphere1.center.z - result->normal.z * sphere1.radius;
                }

                return true;
            }

            return false;
        }

        // 球とAABBの衝突判定
        bool SphereAABB(const Sphere& sphere, const AABB& aabb, CollisionResult* result) {
            // 球の中心をAABBに対してクランプ（最近接点を見つける）
            Vector3 closestPoint = {
                std::max(aabb.min.x, std::min(sphere.center.x, aabb.max.x)),
                std::max(aabb.min.y, std::min(sphere.center.y, aabb.max.y)),
                std::max(aabb.min.z, std::min(sphere.center.z, aabb.max.z))
            };

            // 球の中心と最近接点の距離の二乗を計算
            Vector3 diff = {
                closestPoint.x - sphere.center.x,
                closestPoint.y - sphere.center.y,
                closestPoint.z - sphere.center.z
            };

            float distSquared =
                diff.x * diff.x +
                diff.y * diff.y +
                diff.z * diff.z;

            // 距離が半径以下なら衝突
            if (distSquared <= sphere.radius * sphere.radius) {
                if (result) {
                    // 衝突情報を設定
                    result->hasCollision = true;

                    // 衝突点は最近接点
                    result->collisionPoint = closestPoint;

                    // 法線を計算（AABBから球への単位ベクトル）
                    if (distSquared > 0.0001f) {
                        float distance = std::sqrt(distSquared);
                        result->normal.x = diff.x / distance;
                        result->normal.y = diff.y / distance;
                        result->normal.z = diff.z / distance;

                        // めり込み量を計算
                        result->penetration = sphere.radius - distance;
                    }
                    else {
                        // 球の中心がAABB上にある場合、方向を決定できない
                        // 最も近いAABBの面の法線を使用
                        Vector3 center = aabb.GetCenter();
                        Vector3 size = aabb.GetSize();

                        // X方向のチェック
                        float dx1 = std::abs(sphere.center.x - aabb.min.x);
                        float dx2 = std::abs(sphere.center.x - aabb.max.x);

                        // Y方向のチェック
                        float dy1 = std::abs(sphere.center.y - aabb.min.y);
                        float dy2 = std::abs(sphere.center.y - aabb.max.y);

                        // Z方向のチェック
                        float dz1 = std::abs(sphere.center.z - aabb.min.z);
                        float dz2 = std::abs(sphere.center.z - aabb.max.z);

                        // 最小距離の方向を選択
                        float minDist = std::min({ dx1, dx2, dy1, dy2, dz1, dz2 });

                        if (minDist == dx1) result->normal = { -1.0f, 0.0f, 0.0f };
                        else if (minDist == dx2) result->normal = { 1.0f, 0.0f, 0.0f };
                        else if (minDist == dy1) result->normal = { 0.0f, -1.0f, 0.0f };
                        else if (minDist == dy2) result->normal = { 0.0f, 1.0f, 0.0f };
                        else if (minDist == dz1) result->normal = { 0.0f, 0.0f, -1.0f };
                        else result->normal = { 0.0f, 0.0f, 1.0f };

                        // めり込み量を計算
                        result->penetration = sphere.radius;
                    }
                }

                return true;
            }

            return false;
        }

        // 球とOBBの衝突判定
        bool SphereOBB(const Sphere& sphere, const OBB& obb, CollisionResult* result) {
            // 球の中心をOBBのローカル座標系に変換
            Vector3 localCenter = {
                sphere.center.x - obb.center.x,
                sphere.center.y - obb.center.y,
                sphere.center.z - obb.center.z
            };

            // ローカル座標系で各軸への投影
            Vector3 localPoint;
            localPoint.x =
                localCenter.x * obb.orientations[0].x +
                localCenter.y * obb.orientations[0].y +
                localCenter.z * obb.orientations[0].z;

            localPoint.y =
                localCenter.x * obb.orientations[1].x +
                localCenter.y * obb.orientations[1].y +
                localCenter.z * obb.orientations[1].z;

            localPoint.z =
                localCenter.x * obb.orientations[2].x +
                localCenter.y * obb.orientations[2].y +
                localCenter.z * obb.orientations[2].z;

            // ローカル座標系での最近接点
            Vector3 closestPoint = {
                std::max(-obb.size.x, std::min(localPoint.x, obb.size.x)),
                std::max(-obb.size.y, std::min(localPoint.y, obb.size.y)),
                std::max(-obb.size.z, std::min(localPoint.z, obb.size.z))
            };

            // 最近接点までの距離の二乗
            Vector3 localDiff = {
                closestPoint.x - localPoint.x,
                closestPoint.y - localPoint.y,
                closestPoint.z - localPoint.z
            };

            float distSquared =
                localDiff.x * localDiff.x +
                localDiff.y * localDiff.y +
                localDiff.z * localDiff.z;

            // 距離が半径以下なら衝突
            if (distSquared <= sphere.radius * sphere.radius) {
                if (result) {
                    // 衝突情報を設定
                    result->hasCollision = true;

                    // 衝突点をワールド座標系に戻す
                    Vector3 worldOffset = {
                        closestPoint.x * obb.orientations[0].x +
                        closestPoint.y * obb.orientations[1].x +
                        closestPoint.z * obb.orientations[2].x,

                        closestPoint.x * obb.orientations[0].y +
                        closestPoint.y * obb.orientations[1].y +
                        closestPoint.z * obb.orientations[2].y,

                        closestPoint.x * obb.orientations[0].z +
                        closestPoint.y * obb.orientations[1].z +
                        closestPoint.z * obb.orientations[2].z
                    };

                    result->collisionPoint.x = obb.center.x + worldOffset.x;
                    result->collisionPoint.y = obb.center.y + worldOffset.y;
                    result->collisionPoint.z = obb.center.z + worldOffset.z;

                    // 法線を計算
                    Vector3 normal = {
                        sphere.center.x - result->collisionPoint.x,
                        sphere.center.y - result->collisionPoint.y,
                        sphere.center.z - result->collisionPoint.z
                    };

                    if (distSquared > 0.0001f) {
                        float distance = std::sqrt(distSquared);
                        result->normal.x = normal.x / distance;
                        result->normal.y = normal.y / distance;
                        result->normal.z = normal.z / distance;

                        // めり込み量を計算
                        result->penetration = sphere.radius - distance;
                    }
                    else {
                        // 球の中心がOBB上にある場合、方向を決定できない
                        // 上向きを仮定
                        result->normal = { 0.0f, 1.0f, 0.0f };
                        result->penetration = sphere.radius;
                    }
                }

                return true;
            }

            return false;
        }

        // 球とメッシュの衝突判定
        bool SphereMesh(const Sphere& sphere, const IMeshCollider& mesh, CollisionResult* result) {
            // メッシュコライダーのIntersectsメソッドを使用
            return mesh.Intersects(sphere, result);
        }

        // AABBとAABBの衝突判定
        bool AABBAABB(const AABB& aabb1, const AABB& aabb2, CollisionResult* result) {
            // 各軸で重なっているかチェック
            bool overlapX = aabb1.max.x >= aabb2.min.x && aabb1.min.x <= aabb2.max.x;
            bool overlapY = aabb1.max.y >= aabb2.min.y && aabb1.min.y <= aabb2.max.y;
            bool overlapZ = aabb1.max.z >= aabb2.min.z && aabb1.min.z <= aabb2.max.z;

            // すべての軸で重なっていれば衝突
            if (overlapX && overlapY && overlapZ) {
                if (result) {
                    // 衝突情報を設定
                    result->hasCollision = true;

                    // 各軸でのめり込み量を計算
                    float overlapXAmount = std::min(aabb1.max.x - aabb2.min.x, aabb2.max.x - aabb1.min.x);
                    float overlapYAmount = std::min(aabb1.max.y - aabb2.min.y, aabb2.max.y - aabb1.min.y);
                    float overlapZAmount = std::min(aabb1.max.z - aabb2.min.z, aabb2.max.z - aabb1.min.z);

                    // 最小のめり込み方向を選択
                    if (overlapXAmount <= overlapYAmount && overlapXAmount <= overlapZAmount) {
                        // X軸方向
                        result->penetration = overlapXAmount;

                        // aabb1が右にあるか左にあるかで法線の向きを決定
                        float center1X = (aabb1.min.x + aabb1.max.x) * 0.5f;
                        float center2X = (aabb2.min.x + aabb2.max.x) * 0.5f;

                        if (center1X < center2X) {
                            result->normal = { -1.0f, 0.0f, 0.0f };
                        }
                        else {
                            result->normal = { 1.0f, 0.0f, 0.0f };
                        }
                    }
                    else if (overlapYAmount <= overlapXAmount && overlapYAmount <= overlapZAmount) {
                        // Y軸方向
                        result->penetration = overlapYAmount;

                        // aabb1が上にあるか下にあるかで法線の向きを決定
                        float center1Y = (aabb1.min.y + aabb1.max.y) * 0.5f;
                        float center2Y = (aabb2.min.y + aabb2.max.y) * 0.5f;

                        if (center1Y < center2Y) {
                            result->normal = { 0.0f, -1.0f, 0.0f };
                        }
                        else {
                            result->normal = { 0.0f, 1.0f, 0.0f };
                        }
                    }
                    else {
                        // Z軸方向
                        result->penetration = overlapZAmount;

                        // aabb1が前にあるか後ろにあるかで法線の向きを決定
                        float center1Z = (aabb1.min.z + aabb1.max.z) * 0.5f;
                        float center2Z = (aabb2.min.z + aabb2.max.z) * 0.5f;

                        if (center1Z < center2Z) {
                            result->normal = { 0.0f, 0.0f, -1.0f };
                        }
                        else {
                            result->normal = { 0.0f, 0.0f, 1.0f };
                        }
                    }

                    // 衝突点を計算（簡易的に両方のAABBの中心間の中点を使用）
                    Vector3 center1 = {
                        (aabb1.min.x + aabb1.max.x) * 0.5f,
                        (aabb1.min.y + aabb1.max.y) * 0.5f,
                        (aabb1.min.z + aabb1.max.z) * 0.5f
                    };

                    Vector3 center2 = {
                        (aabb2.min.x + aabb2.max.x) * 0.5f,
                        (aabb2.min.y + aabb2.max.y) * 0.5f,
                        (aabb2.min.z + aabb2.max.z) * 0.5f
                    };

                    result->collisionPoint.x = (center1.x + center2.x) * 0.5f;
                    result->collisionPoint.y = (center1.y + center2.y) * 0.5f;
                    result->collisionPoint.z = (center1.z + center2.z) * 0.5f;
                }

                return true;
            }

            return false;
        }

        // AABBとOBBの衝突判定 (簡易版)
        bool AABBOBB(const AABB& aabb, const OBB& obb, CollisionResult* result) {
            // 簡略化した実装として、AABBをOBBに変換して処理
            OBB obbFromAABB;
            obbFromAABB.center = {
                (aabb.min.x + aabb.max.x) * 0.5f,
                (aabb.min.y + aabb.max.y) * 0.5f,
                (aabb.min.z + aabb.max.z) * 0.5f
            };

            // 標準軸
            obbFromAABB.orientations[0] = { 1.0f, 0.0f, 0.0f };
            obbFromAABB.orientations[1] = { 0.0f, 1.0f, 0.0f };
            obbFromAABB.orientations[2] = { 0.0f, 0.0f, 1.0f };

            obbFromAABB.size = {
                (aabb.max.x - aabb.min.x) * 0.5f,
                (aabb.max.y - aabb.min.y) * 0.5f,
                (aabb.max.z - aabb.min.z) * 0.5f
            };

            // OBB同士の衝突判定を呼び出す
            return OBBOBB(obbFromAABB, obb, result);
        }

        // AABBとメッシュの衝突判定
        bool AABBMesh(const AABB& aabb, const IMeshCollider& mesh, CollisionResult* result) {
            // メッシュコライダーのIntersectsメソッドを使用
            return mesh.Intersects(aabb, result);
        }

        // OBBとOBBの衝突判定（分離軸テスト）
        bool OBBOBB(const OBB& obb1, const OBB& obb2, CollisionResult* result) {
            // 15個の分離軸をチェック
            float minPenetration = std::numeric_limits<float>::max();
            Vector3 bestAxis = { 0.0f, 0.0f, 0.0f };

            // 中心間のベクトル
            Vector3 centerDiff = {
                obb2.center.x - obb1.center.x,
                obb2.center.y - obb1.center.y,
                obb2.center.z - obb1.center.z
            };

            // 1. obb1の3軸
            for (int i = 0; i < 3; i++) {
                // 分離軸
                Vector3 axis = obb1.orientations[i];

                // 中心間の距離をこの軸に投影
                float centerDistance = std::abs(
                    centerDiff.x * axis.x +
                    centerDiff.y * axis.y +
                    centerDiff.z * axis.z
                );

                // obb1の投影半径
                float radius1 = obb1.size[i];

                // obb2の投影半径
                float radius2 =
                    obb2.size.x * std::abs(obb2.orientations[0].x * axis.x + obb2.orientations[0].y * axis.y + obb2.orientations[0].z * axis.z) +
                    obb2.size.y * std::abs(obb2.orientations[1].x * axis.x + obb2.orientations[1].y * axis.y + obb2.orientations[1].z * axis.z) +
                    obb2.size.z * std::abs(obb2.orientations[2].x * axis.x + obb2.orientations[2].y * axis.y + obb2.orientations[2].z * axis.z);

                // 投影半径の和と中心間距離を比較
                float overlap = radius1 + radius2 - centerDistance;

                // 分離している場合は衝突なし
                if (overlap <= 0.0f) {
                    return false;
                }

                // 最小のめり込み量を記録
                if (overlap < minPenetration) {
                    minPenetration = overlap;
                    bestAxis = axis;

                    // ベクトルの向きを調整（中心間の方向と同じ向きにする）
                    float dot = centerDiff.x * axis.x + centerDiff.y * axis.y + centerDiff.z * axis.z;
                    if (dot < 0.0f) {
                        bestAxis.x = -bestAxis.x;
                        bestAxis.y = -bestAxis.y;
                        bestAxis.z = -bestAxis.z;
                    }
                }
            }

            // 2. obb2の3軸
            for (int i = 0; i < 3; i++) {
                // 分離軸
                Vector3 axis = obb2.orientations[i];

                // 中心間の距離をこの軸に投影
                float centerDistance = std::abs(
                    centerDiff.x * axis.x +
                    centerDiff.y * axis.y +
                    centerDiff.z * axis.z
                );

                // obb1の投影半径
                float radius1 =
                    obb1.size.x * std::abs(obb1.orientations[0].x * axis.x + obb1.orientations[0].y * axis.y + obb1.orientations[0].z * axis.z) +
                    obb1.size.y * std::abs(obb1.orientations[1].x * axis.x + obb1.orientations[1].y * axis.y + obb1.orientations[1].z * axis.z) +
                    obb1.size.z * std::abs(obb1.orientations[2].x * axis.x + obb1.orientations[2].y * axis.y + obb1.orientations[2].z * axis.z);

                // obb2の投影半径
                float radius2 = obb2.size[i];

                // 投影半径の和と中心間距離を比較
                float overlap = radius1 + radius2 - centerDistance;

                // 分離している場合は衝突なし
                if (overlap <= 0.0f) {
                    return false;
                }

                // 最小のめり込み量を記録
                if (overlap < minPenetration) {
                    minPenetration = overlap;
                    bestAxis = axis;

                    // ベクトルの向きを調整（中心間の方向と同じ向きにする）
                    float dot = centerDiff.x * axis.x + centerDiff.y * axis.y + centerDiff.z * axis.z;
                    if (dot < 0.0f) {
                        bestAxis.x = -bestAxis.x;
                        bestAxis.y = -bestAxis.y;
                        bestAxis.z = -bestAxis.z;
                    }
                }
            }

            // 3. 9個の軸（両方のOBBの軸の外積）
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    // 分離軸（外積）
                    Vector3 axis = {
                        obb1.orientations[i].y * obb2.orientations[j].z - obb1.orientations[i].z * obb2.orientations[j].y,
                        obb1.orientations[i].z * obb2.orientations[j].x - obb1.orientations[i].x * obb2.orientations[j].z,
                        obb1.orientations[i].x * obb2.orientations[j].y - obb1.orientations[i].y * obb2.orientations[j].x
                    };

                    // 軸の長さがほぼ0の場合（平行な軸）は次の軸へ
                    float axisSqLength = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
                    if (axisSqLength < 0.00001f) {
                        continue;
                    }

                    // 軸を正規化
                    float axisLength = std::sqrt(axisSqLength);
                    axis.x /= axisLength;
                    axis.y /= axisLength;
                    axis.z /= axisLength;

                    // 中心間の距離をこの軸に投影
                    float centerDistance = std::abs(
                        centerDiff.x * axis.x +
                        centerDiff.y * axis.y +
                        centerDiff.z * axis.z
                    );

                    // obb1の投影半径
                    float radius1 =
                        obb1.size.x * std::abs(obb1.orientations[0].x * axis.x + obb1.orientations[0].y * axis.y + obb1.orientations[0].z * axis.z) +
                        obb1.size.y * std::abs(obb1.orientations[1].x * axis.x + obb1.orientations[1].y * axis.y + obb1.orientations[1].z * axis.z) +
                        obb1.size.z * std::abs(obb1.orientations[2].x * axis.x + obb1.orientations[2].y * axis.y + obb1.orientations[2].z * axis.z);

                    // obb2の投影半径
                    float radius2 =
                        obb2.size.x * std::abs(obb2.orientations[0].x * axis.x + obb2.orientations[0].y * axis.y + obb2.orientations[0].z * axis.z) +
                        obb2.size.y * std::abs(obb2.orientations[1].x * axis.x + obb2.orientations[1].y * axis.y + obb2.orientations[1].z * axis.z) +
                        obb2.size.z * std::abs(obb2.orientations[2].x * axis.x + obb2.orientations[2].y * axis.y + obb2.orientations[2].z * axis.z);

                    // 投影半径の和と中心間距離を比較
                    float overlap = radius1 + radius2 - centerDistance;

                    // 分離している場合は衝突なし
                    if (overlap <= 0.0f) {
                        return false;
                    }

                    // 最小のめり込み量を記録
                    if (overlap < minPenetration) {
                        minPenetration = overlap;
                        bestAxis = axis;

                        // ベクトルの向きを調整（中心間の方向と同じ向きにする）
                        float dot = centerDiff.x * axis.x + centerDiff.y * axis.y + centerDiff.z * axis.z;
                        if (dot < 0.0f) {
                            bestAxis.x = -bestAxis.x;
                            bestAxis.y = -bestAxis.y;
                            bestAxis.z = -bestAxis.z;
                        }
                    }
                }
            }

            // すべての軸でめり込みがあれば衝突
            if (result) {
                // 衝突情報を設定
                result->hasCollision = true;
                result->normal = bestAxis;
                result->penetration = minPenetration;

                // 衝突点を簡易的に計算（両方のOBBの中心間の中点を使用）
                result->collisionPoint.x = (obb1.center.x + obb2.center.x) * 0.5f;
                result->collisionPoint.y = (obb1.center.y + obb2.center.y) * 0.5f;
                result->collisionPoint.z = (obb1.center.z + obb2.center.z) * 0.5f;
            }

            return true;
        }

        // OBBとメッシュの衝突判定
        bool OBBMesh(const OBB& obb, const IMeshCollider& mesh, CollisionResult* result) {
            // メッシュコライダーのIntersectsメソッドを使用
            return mesh.Intersects(obb, result);
        }

        // レイとメッシュの衝突判定
        bool RayMesh(const Ray& ray, const IMeshCollider& mesh, float maxDistance, CollisionResult* result) {
            // メッシュコライダーのIntersectsメソッドを使用
            return mesh.Intersects(ray, maxDistance, result);
        }

        // レイと球の衝突判定
        bool RaySphere(const Ray& ray, const Sphere& sphere, float maxDistance, CollisionResult* result) {
            // レイの始点から球の中心へのベクトル
            Vector3 m = {
                sphere.center.x - ray.origin.x,
                sphere.center.y - ray.origin.y,
                sphere.center.z - ray.origin.z
            };

            // mをレイの方向に投影した長さ
            float b =
                m.x * ray.direction.x +
                m.y * ray.direction.y +
                m.z * ray.direction.z;

            // 球の中心からレイへの最短距離の二乗
            float c =
                m.x * m.x +
                m.y * m.y +
                m.z * m.z -
                sphere.radius * sphere.radius;

            // レイの始点が球の内部にある場合
            if (c <= 0.0f) {
                if (result) {
                    result->hasCollision = true;
                    result->collisionPoint = ray.origin;

                    // 法線は球の中心からレイの始点への方向
                    if (c < -0.00001f) {
                        float mLength = std::sqrt(m.x * m.x + m.y * m.y + m.z * m.z);
                        result->normal.x = -m.x / mLength;
                        result->normal.y = -m.y / mLength;
                        result->normal.z = -m.z / mLength;
                    }
                    else {
                        // 球の中心上にある場合は適当な方向
                        result->normal = { 0.0f, 1.0f, 0.0f };
                    }

                    result->penetration = 0.0f;
                }

                return true;
            }

            // レイが球から離れていく場合や、最短距離が球の半径より大きい場合
            if (b <= 0.0f || c > b * b) {
                return false;
            }

            // レイと球の交点までの距離を計算
            float t = b - std::sqrt(b * b - c);

            // 最大距離を超えた場合は衝突なし
            if (t > maxDistance) {
                return false;
            }

            // 衝突情報を設定
            if (result) {
                result->hasCollision = true;

                // 交点の座標
                result->collisionPoint.x = ray.origin.x + ray.direction.x * t;
                result->collisionPoint.y = ray.origin.y + ray.direction.y * t;
                result->collisionPoint.z = ray.origin.z + ray.direction.z * t;

                // 法線は球の中心から交点への方向
                Vector3 normal = {
                    result->collisionPoint.x - sphere.center.x,
                    result->collisionPoint.y - sphere.center.y,
                    result->collisionPoint.z - sphere.center.z
                };

                float normalLength = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                result->normal.x = normal.x / normalLength;
                result->normal.y = normal.y / normalLength;
                result->normal.z = normal.z / normalLength;

                result->penetration = t;
            }

            return true;
        }

        // レイとAABBの衝突判定
        bool RayAABB(const Ray& ray, const AABB& aabb, float maxDistance, CollisionResult* result) {
            // スラブ法による交差判定
            float tmin = -std::numeric_limits<float>::max();
            float tmax = std::numeric_limits<float>::max();

            // 各軸に対して交差判定
            for (int i = 0; i < 3; i++) {
                float d, invD, t1, t2;

                // X軸
                if (i == 0) {
                    d = ray.direction.x;
                    invD = (std::abs(d) < 0.00001f) ? std::numeric_limits<float>::max() : (1.0f / d);
                    t1 = (aabb.min.x - ray.origin.x) * invD;
                    t2 = (aabb.max.x - ray.origin.x) * invD;
                }
                // Y軸
                else if (i == 1) {
                    d = ray.direction.y;
                    invD = (std::abs(d) < 0.00001f) ? std::numeric_limits<float>::max() : (1.0f / d);
                    t1 = (aabb.min.y - ray.origin.y) * invD;
                    t2 = (aabb.max.y - ray.origin.y) * invD;
                }
                // Z軸
                else {
                    d = ray.direction.z;
                    invD = (std::abs(d) < 0.00001f) ? std::numeric_limits<float>::max() : (1.0f / d);
                    t1 = (aabb.min.z - ray.origin.z) * invD;
                    t2 = (aabb.max.z - ray.origin.z) * invD;
                }

                // 負の方向の場合はt1とt2を入れ替え
                if (t1 > t2) {
                    std::swap(t1, t2);
                }

                // 交差範囲の更新
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);

                // 交差範囲が無効になった場合
                if (tmin > tmax) {
                    return false;
                }
            }

            // 最大距離を超えた場合は衝突なし
            if (tmin > maxDistance || tmax < 0.0f) {
                return false;
            }

            // 衝突点までの距離
            float t = (tmin >= 0.0f) ? tmin : tmax;

            // 衝突情報を設定
            if (result) {
                result->hasCollision = true;

                // 交点の座標
                result->collisionPoint.x = ray.origin.x + ray.direction.x * t;
                result->collisionPoint.y = ray.origin.y + ray.direction.y * t;
                result->collisionPoint.z = ray.origin.z + ray.direction.z * t;

                // 法線の計算
                Vector3 center = {
                    (aabb.min.x + aabb.max.x) * 0.5f,
                    (aabb.min.y + aabb.max.y) * 0.5f,
                    (aabb.min.z + aabb.max.z) * 0.5f
                };

                Vector3 halfSize = {
                    (aabb.max.x - aabb.min.x) * 0.5f,
                    (aabb.max.y - aabb.min.y) * 0.5f,
                    (aabb.max.z - aabb.min.z) * 0.5f
                };

                Vector3 d = {
                    result->collisionPoint.x - center.x,
                    result->collisionPoint.y - center.y,
                    result->collisionPoint.z - center.z
                };

                // 各面までの距離の割合を計算
                float x = std::abs(d.x / halfSize.x);
                float y = std::abs(d.y / halfSize.y);
                float z = std::abs(d.z / halfSize.z);

                // 最も近い面の法線を選択
                if (x > y && x > z) {
                    result->normal = { d.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f };
                }
                else if (y > z) {
                    result->normal = { 0.0f, d.y > 0.0f ? 1.0f : -1.0f, 0.0f };
                }
                else {
                    result->normal = { 0.0f, 0.0f, d.z > 0.0f ? 1.0f : -1.0f };
                }

                result->penetration = t;
            }

            return true;
        }

        // レイとOBBの衝突判定
        bool RayOBB(const Ray& ray, const OBB& obb, float maxDistance, CollisionResult* result) {
            // レイをOBBのローカル座標系に変換
            Vector3 p = {
                ray.origin.x - obb.center.x,
                ray.origin.y - obb.center.y,
                ray.origin.z - obb.center.z
            };

            Vector3 localOrigin = {
                p.x * obb.orientations[0].x + p.y * obb.orientations[0].y + p.z * obb.orientations[0].z,
                p.x * obb.orientations[1].x + p.y * obb.orientations[1].y + p.z * obb.orientations[1].z,
                p.x * obb.orientations[2].x + p.y * obb.orientations[2].y + p.z * obb.orientations[2].z
            };

            Vector3 localDirection = {
                ray.direction.x * obb.orientations[0].x + ray.direction.y * obb.orientations[0].y + ray.direction.z * obb.orientations[0].z,
                ray.direction.x * obb.orientations[1].x + ray.direction.y * obb.orientations[1].y + ray.direction.z * obb.orientations[1].z,
                ray.direction.x * obb.orientations[2].x + ray.direction.y * obb.orientations[2].y + ray.direction.z * obb.orientations[2].z
            };

            // ローカル座標系でのAABB
            AABB localAABB = {
                {-obb.size.x, -obb.size.y, -obb.size.z},
                {obb.size.x, obb.size.y, obb.size.z}
            };

            // ローカルレイ
            Ray localRay = { localOrigin, localDirection };

            // ローカル座標系でのレイ-AABB交差判定
            CollisionResult localResult;
            if (!RayAABB(localRay, localAABB, maxDistance, &localResult)) {
                return false;
            }

            // 交点をワールド座標系に戻す
            if (result) {
                result->hasCollision = true;

                // ローカル座標系での法線
                Vector3 localNormal = localResult.normal;

                // ワールド座標系での法線
                result->normal.x =
                    localNormal.x * obb.orientations[0].x +
                    localNormal.y * obb.orientations[1].x +
                    localNormal.z * obb.orientations[2].x;

                result->normal.y =
                    localNormal.x * obb.orientations[0].y +
                    localNormal.y * obb.orientations[1].y +
                    localNormal.z * obb.orientations[2].y;

                result->normal.z =
                    localNormal.x * obb.orientations[0].z +
                    localNormal.y * obb.orientations[1].z +
                    localNormal.z * obb.orientations[2].z;

                // ローカル座標系での交点
                Vector3 localPoint = localResult.collisionPoint;

                // ワールド座標系での交点
                result->collisionPoint.x =
                    obb.center.x +
                    localPoint.x * obb.orientations[0].x +
                    localPoint.y * obb.orientations[1].x +
                    localPoint.z * obb.orientations[2].x;

                result->collisionPoint.y =
                    obb.center.y +
                    localPoint.x * obb.orientations[0].y +
                    localPoint.y * obb.orientations[1].y +
                    localPoint.z * obb.orientations[2].y;

                result->collisionPoint.z =
                    obb.center.z +
                    localPoint.x * obb.orientations[0].z +
                    localPoint.y * obb.orientations[1].z +
                    localPoint.z * obb.orientations[2].z;

                result->penetration = localResult.penetration;
            }

            return true;
        }

        // 点と球の衝突判定
        bool PointSphere(const Vector3& point, const Sphere& sphere, CollisionResult* result) {
            // 点と球の中心の距離を計算
            Vector3 diff = {
                point.x - sphere.center.x,
                point.y - sphere.center.y,
                point.z - sphere.center.z
            };

            float distSquared =
                diff.x * diff.x +
                diff.y * diff.y +
                diff.z * diff.z;

            // 距離が半径以下なら衝突
            if (distSquared <= sphere.radius * sphere.radius) {
                if (result) {
                    // 衝突情報を設定
                    result->hasCollision = true;
                    result->collisionPoint = point;

                    if (distSquared > 0.00001f) {
                        float distance = std::sqrt(distSquared);

                        // 法線は球の中心から点への方向
                        result->normal.x = diff.x / distance;
                        result->normal.y = diff.y / distance;
                        result->normal.z = diff.z / distance;

                        // めり込み量
                        result->penetration = sphere.radius - distance;
                    }
                    else {
                        // 点が球の中心に近い場合
                        result->normal = { 0.0f, 1.0f, 0.0f };
                        result->penetration = sphere.radius;
                    }
                }

                return true;
            }

            return false;
        }

        // 点とAABBの衝突判定
        bool PointAABB(const Vector3& point, const AABB& aabb, CollisionResult* result) {
            // 点がAABBの内部にあるかチェック
            if (point.x >= aabb.min.x && point.x <= aabb.max.x &&
                point.y >= aabb.min.y && point.y <= aabb.max.y &&
                point.z >= aabb.min.z && point.z <= aabb.max.z) {

                if (result) {
                    // 衝突情報を設定
                    result->hasCollision = true;
                    result->collisionPoint = point;

                    // 最も近い面の法線を計算
                    Vector3 center = {
                        (aabb.min.x + aabb.max.x) * 0.5f,
                        (aabb.min.y + aabb.max.y) * 0.5f,
                        (aabb.min.z + aabb.max.z) * 0.5f
                    };

                    Vector3 halfSize = {
                        (aabb.max.x - aabb.min.x) * 0.5f,
                        (aabb.max.y - aabb.min.y) * 0.5f,
                        (aabb.max.z - aabb.min.z) * 0.5f
                    };

                    Vector3 d = {
                        point.x - center.x,
                        point.y - center.y,
                        point.z - center.z
                    };

                    // 各面までの距離の割合を計算
                    float x = std::abs(d.x / halfSize.x);
                    float y = std::abs(d.y / halfSize.y);
                    float z = std::abs(d.z / halfSize.z);

                    // 最も近い面の法線を選択
                    if (x > y && x > z) {
                        result->normal = { d.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f };
                        result->penetration = halfSize.x - std::abs(d.x);
                    }
                    else if (y > z) {
                        result->normal = { 0.0f, d.y > 0.0f ? 1.0f : -1.0f, 0.0f };
                        result->penetration = halfSize.y - std::abs(d.y);
                    }
                    else {
                        result->normal = { 0.0f, 0.0f, d.z > 0.0f ? 1.0f : -1.0f };
                        result->penetration = halfSize.z - std::abs(d.z);
                    }
                }

                return true;
            }

            return false;
        }

        // 点とOBBの衝突判定
        bool PointOBB(const Vector3& point, const OBB& obb, CollisionResult* result) {
            // 点をOBBのローカル座標系に変換
            Vector3 localPoint = {
                (point.x - obb.center.x) * obb.orientations[0].x +
                (point.y - obb.center.y) * obb.orientations[0].y +
                (point.z - obb.center.z) * obb.orientations[0].z,

                (point.x - obb.center.x) * obb.orientations[1].x +
                (point.y - obb.center.y) * obb.orientations[1].y +
                (point.z - obb.center.z) * obb.orientations[1].z,

                (point.x - obb.center.x) * obb.orientations[2].x +
                (point.y - obb.center.y) * obb.orientations[2].y +
                (point.z - obb.center.z) * obb.orientations[2].z
            };

            // ローカル座標系でのAABB
            AABB localAABB = {
                {-obb.size.x, -obb.size.y, -obb.size.z},
                {obb.size.x, obb.size.y, obb.size.z}
            };

            // ローカル座標系での点-AABB交差判定
            CollisionResult localResult;
            if (!PointAABB(localPoint, localAABB, &localResult)) {
                return false;
            }

            // 交点をワールド座標系に戻す
            if (result) {
                result->hasCollision = true;
                result->collisionPoint = point;

                // ローカル座標系での法線
                Vector3 localNormal = localResult.normal;

                // ワールド座標系での法線
                result->normal.x =
                    localNormal.x * obb.orientations[0].x +
                    localNormal.y * obb.orientations[1].x +
                    localNormal.z * obb.orientations[2].x;

                result->normal.y =
                    localNormal.x * obb.orientations[0].y +
                    localNormal.y * obb.orientations[1].y +
                    localNormal.z * obb.orientations[2].y;

                result->normal.z =
                    localNormal.x * obb.orientations[0].z +
                    localNormal.y * obb.orientations[1].z +
                    localNormal.z * obb.orientations[2].z;

                result->penetration = localResult.penetration;
            }

            return true;
        }

        // 点とメッシュの衝突判定
        bool PointMesh(const Vector3& point, const IMeshCollider& mesh, CollisionResult* result) {
            // メッシュコライダーのIntersectsメソッドを使用
            return mesh.Intersects(point, result);
        }

} // namespace CollisionDetection

} // namespace Collision