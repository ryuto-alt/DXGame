#include "CollisionManager.h"
#include "CollisionVisualizer.h"
#include <algorithm>

// 静的メンバ変数の実体化
CollisionManager* CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance() {
    if (!instance_) {
        instance_ = new CollisionManager();
    }
    return instance_;
}

void CollisionManager::Initialize() {
    // 初期化処理
    colliders_.clear();
    colliderLayers_.clear();
    colliderNames_.clear();

    // デフォルトですべてのレイヤー同士を衝突させる
    for (size_t i = 0; i < static_cast<size_t>(CollisionLayer::MaxLayers); ++i) {
        for (size_t j = 0; j < static_cast<size_t>(CollisionLayer::MaxLayers); ++j) {
            collisionMatrix_[i][j] = true;
        }
    }
}

void CollisionManager::Update() {
    // 全てのコライダーを更新
    for (auto* collider : colliders_) {
        if (collider && collider->IsEnabled()) {
            collider->Update();
        }
    }
}

void CollisionManager::CheckAllCollisions() {
    // コライダーの数
    size_t colliderCount = colliders_.size();

    // 衝突情報
    CollisionInfo info;

    // 総当たりで衝突判定
    for (size_t i = 0; i < colliderCount; ++i) {
        ColliderBase* colliderA = colliders_[i];

        // 無効なコライダーはスキップ
        if (!colliderA || !colliderA->IsEnabled()) {
            continue;
        }

        for (size_t j = i + 1; j < colliderCount; ++j) {
            ColliderBase* colliderB = colliders_[j];

            // 無効なコライダーはスキップ
            if (!colliderB || !colliderB->IsEnabled()) {
                continue;
            }

            // レイヤーチェック - 衝突判定が無効ならスキップ
            if (!ShouldLayersCollide(GetColliderLayer(colliderA), GetColliderLayer(colliderB))) {
                continue;
            }

            // 衝突判定実行
            if (colliderA->CheckCollision(colliderB, info)) {
                // 衝突していた場合、それぞれのコライダーのコールバックを呼び出す
                colliderA->OnCollision(info);

                // 相手から見た衝突情報を生成（法線を反転）
                CollisionInfo reverseInfo = info;
                reverseInfo.normal.x = -info.normal.x;
                reverseInfo.normal.y = -info.normal.y;
                reverseInfo.normal.z = -info.normal.z;

                colliderB->OnCollision(reverseInfo);

                // 衝突点を可視化
                CollisionVisualizer::GetInstance()->AddCollisionPoint(info.collisionPoint, info.normal);

                // グローバルコールバックがあれば呼び出す
                if (globalCollisionCallback_) {
                    globalCollisionCallback_(colliderA, colliderB, info);
                }
            }
        }
    }
}

void CollisionManager::AddCollider(ColliderBase* collider) {
    // すでに登録済みの場合は追加しない
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it == colliders_.end()) {
        colliders_.push_back(collider);

        // デフォルトレイヤーを設定
        SetColliderLayer(collider, CollisionLayer::Default);

        // デフォルト名を設定
        SetColliderName(collider, "Collider" + std::to_string(colliders_.size()));
    }
}

void CollisionManager::RemoveCollider(ColliderBase* collider) {
    // コライダーを検索して削除
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it != colliders_.end()) {
        // レイヤーマップからも削除
        colliderLayers_.erase(collider);

        // 名前マップからも削除
        colliderNames_.erase(collider);

        // コライダーリストから削除
        colliders_.erase(it);
    }
}

void CollisionManager::ClearColliders() {
    // すべてのコライダーをクリア
    colliders_.clear();
    colliderLayers_.clear();
    colliderNames_.clear();
}

void CollisionManager::SetLayerCollision(CollisionLayer layer1, CollisionLayer layer2, bool shouldCollide) {
    // レイヤー同士の衝突設定
    size_t index1 = static_cast<size_t>(layer1);
    size_t index2 = static_cast<size_t>(layer2);

    if (index1 < static_cast<size_t>(CollisionLayer::MaxLayers) &&
        index2 < static_cast<size_t>(CollisionLayer::MaxLayers)) {
        collisionMatrix_[index1][index2] = shouldCollide;
        collisionMatrix_[index2][index1] = shouldCollide; // 対称性を保つ
    }
}

void CollisionManager::SetColliderLayer(ColliderBase* collider, CollisionLayer layer) {
    // コライダーのレイヤー設定
    if (collider) {
        colliderLayers_[collider] = layer;
    }
}

CollisionLayer CollisionManager::GetColliderLayer(ColliderBase* collider) const {
    // コライダーのレイヤー取得
    if (collider) {
        auto it = colliderLayers_.find(collider);
        if (it != colliderLayers_.end()) {
            return it->second;
        }
    }
    return CollisionLayer::Default;
}

bool CollisionManager::ShouldLayersCollide(CollisionLayer layer1, CollisionLayer layer2) const {
    // レイヤー同士が衝突するかチェック
    size_t index1 = static_cast<size_t>(layer1);
    size_t index2 = static_cast<size_t>(layer2);

    if (index1 < static_cast<size_t>(CollisionLayer::MaxLayers) &&
        index2 < static_cast<size_t>(CollisionLayer::MaxLayers)) {
        return collisionMatrix_[index1][index2];
    }
    return true; // デフォルトは衝突する
}

void CollisionManager::SetColliderName(ColliderBase* collider, const std::string& name) {
    // コライダーに名前を設定（デバッグ用）
    if (collider) {
        colliderNames_[collider] = name;
    }
}

const std::string& CollisionManager::GetColliderName(ColliderBase* collider) const {
    // コライダーの名前を取得（デバッグ用）
    static const std::string emptyName = "Unnamed";

    if (collider) {
        auto it = colliderNames_.find(collider);
        if (it != colliderNames_.end()) {
            return it->second;
        }
    }
    return emptyName;
}

std::vector<ColliderBase*> CollisionManager::GetCollidersInLayer(CollisionLayer layer) {
    // 特定のレイヤーのコライダーを取得
    std::vector<ColliderBase*> result;

    for (auto* collider : colliders_) {
        if (collider && GetColliderLayer(collider) == layer) {
            result.push_back(collider);
        }
    }

    return result;
}