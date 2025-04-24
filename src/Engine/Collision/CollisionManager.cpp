// CollisionManager.cpp
// 衝突検出マネージャの実装
#include "CollisionManager.h"
#include <cassert>
#include <debugapi.h>

namespace Collision {

    // 静的メンバ変数の実体化
    CollisionManager* CollisionManager::instance_ = nullptr;

    CollisionManager* CollisionManager::GetInstance() {
        if (!instance_) {
            instance_ = new CollisionManager();
        }
        return instance_;
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

        // デバッグ出力
        OutputDebugStringA("CollisionManager: Initialized successfully\n");
    }

    void CollisionManager::Finalize() {
        // すべてのリソースを解放
        colliders_.clear();
        colliderMap_.clear();
        colliderIDMap_.clear();
        prevCollisions_.clear();
        ignorePairs_.clear();

        // シングルトンインスタンスの解放
        delete instance_;
        instance_ = nullptr;

        // デバッグ出力
        OutputDebugStringA("CollisionManager: Finalized successfully\n");
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
        // 後でUnoEngineの描画機能を使用して実装
    }

    uint64_t CollisionManager::GetColliderPairHash(uint32_t collider1ID, uint32_t collider2ID) const {
        // 小さい方を前に、大きい方を後ろに配置して一貫性を保つ
        if (collider1ID > collider2ID) {
            std::swap(collider1ID, collider2ID);
        }

        // 64ビットハッシュ値を作成
        return (static_cast<uint64_t>(collider1ID) << 32) | static_cast<uint64_t>(collider2ID);
    }

    // コライダー作成ヘルパー関数
    std::shared_ptr<SphereCollider> CollisionManager::CreateSphereCollider(const std::string& name, const Vector3& center, float radius) {
        auto collider = std::make_shared<SphereCollider>(center, radius);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<AABBCollider> CollisionManager::CreateAABBCollider(const std::string& name, const Vector3& min, const Vector3& max) {
        auto collider = std::make_shared<AABBCollider>(min, max);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<OBBCollider> CollisionManager::CreateOBBCollider(const std::string& name, const Vector3& center, const Vector3 orientations[3], const Vector3& size) {
        auto collider = std::make_shared<OBBCollider>(center, orientations, size);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<OBBCollider> CollisionManager::CreateOBBCollider(const std::string& name, const Matrix4x4& worldMatrix, const Vector3& size) {
        auto collider = std::make_shared<OBBCollider>(worldMatrix, size);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<MeshCollider> CollisionManager::CreateMeshCollider(const std::string& name) {
        auto collider = std::make_shared<MeshCollider>();
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<MeshCollider> CollisionManager::CreateTriangleMeshCollider(const std::string& name, const std::vector<Triangle>& triangles) {
        auto collider = std::make_shared<MeshCollider>();
        collider->CreateTriangleMeshCollider(triangles);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<MeshCollider> CollisionManager::CreateTriangleMeshCollider(const std::string& name, const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices) {
        auto collider = std::make_shared<MeshCollider>();
        collider->CreateTriangleMeshCollider(vertices, indices);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<MeshCollider> CollisionManager::CreateConvexMeshCollider(const std::string& name, const std::vector<Vector3>& vertices) {
        auto collider = std::make_shared<MeshCollider>();
        collider->CreateConvexMeshCollider(vertices);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

    std::shared_ptr<MeshCollider> CollisionManager::CreateHeightfieldCollider(const std::string& name, int width, int height, const std::vector<float>& heights, float scaleX, float scaleY, float scaleZ) {
        auto collider = std::make_shared<MeshCollider>();
        collider->CreateHeightfieldCollider(width, height, heights, scaleX, scaleY, scaleZ);
        collider->SetName(name);
        AddCollider(collider);
        return collider;
    }

} // namespace Collision