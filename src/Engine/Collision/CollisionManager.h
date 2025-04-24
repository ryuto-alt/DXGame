#pragma once
#include "ColliderBase.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

// 衝突フィルタリング用のレイヤー定義
enum class CollisionLayer {
    Default = 0,
    Player = 1,
    Enemy = 2,
    Bullet = 3,
    Item = 4,
    Ground = 5,
    Wall = 6,
    Trigger = 7,
    // 必要に応じて追加
    MaxLayers = 32 // 最大32レイヤー
};

class CollisionManager {
private:
    // シングルトンインスタンス
    static CollisionManager* instance_;

    // コンストラクタ（シングルトン）
    CollisionManager() = default;
    // デストラクタ（シングルトン）
    ~CollisionManager() = default;
    // コピー禁止
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;

    // コライダーのリスト
    std::vector<ColliderBase*> colliders_;

    // レイヤーマスク（衝突判定フィルタリング用）
    using LayerMask = std::array<bool, static_cast<size_t>(CollisionLayer::MaxLayers)>;
    std::array<LayerMask, static_cast<size_t>(CollisionLayer::MaxLayers)> collisionMatrix_;

    // コライダーのレイヤーマップ
    std::unordered_map<ColliderBase*, CollisionLayer> colliderLayers_;

    // コライダー名前マップ（デバッグ用）
    std::unordered_map<ColliderBase*, std::string> colliderNames_;

    // グローバル衝突コールバック
    std::function<void(ColliderBase*, ColliderBase*, const CollisionInfo&)> globalCollisionCallback_;

public:
    // シングルトンインスタンスの取得
    static CollisionManager* GetInstance();

    // 初期化
    void Initialize();

    // 更新（全てのコライダーの位置更新）
    void Update();

    // 衝突判定処理（全ての組み合わせを判定）
    void CheckAllCollisions();

    // コライダーの登録
    void AddCollider(ColliderBase* collider);

    // コライダーの削除
    void RemoveCollider(ColliderBase* collider);

    // すべてのコライダーをクリア
    void ClearColliders();

    // レイヤー間の衝突設定
    void SetLayerCollision(CollisionLayer layer1, CollisionLayer layer2, bool shouldCollide);

    // コライダーのレイヤー設定
    void SetColliderLayer(ColliderBase* collider, CollisionLayer layer);

    // コライダーのレイヤー取得
    CollisionLayer GetColliderLayer(ColliderBase* collider) const;

    // コライダーに名前を設定（デバッグ用）
    void SetColliderName(ColliderBase* collider, const std::string& name);

    // コライダーの名前を取得（デバッグ用）
    const std::string& GetColliderName(ColliderBase* collider) const;

    // グローバル衝突コールバック設定
    void SetGlobalCollisionCallback(const std::function<void(ColliderBase*, ColliderBase*, const CollisionInfo&)>& callback) {
        globalCollisionCallback_ = callback;
    }

    // 特定のレイヤーのコライダーを取得
    std::vector<ColliderBase*> GetCollidersInLayer(CollisionLayer layer);

    // コライダーのリストを取得
    const std::vector<ColliderBase*>& GetColliders() const { return colliders_; }

private:
    // レイヤー同士が衝突するかチェック
    bool ShouldLayersCollide(CollisionLayer layer1, CollisionLayer layer2) const;
};