#pragma once
#include "Collision.h"
#include <vector>
#include <memory>
#include <functional>

namespace Collision {
    // 衝突オブジェクトの基底クラス
    class CollisionObject {
    public:
        // 仮想デストラクタ
        virtual ~CollisionObject() = default;

        // 衝突形状の種類
        enum class ShapeType {
            Sphere,
            Capsule
        };

        // 形状種別の取得
        virtual ShapeType GetShapeType() const = 0;

        // 形状データの取得（キャスト必要）
        virtual void* GetShapeData() = 0;
        virtual const void* GetShapeData() const = 0;

        // 衝突時コールバック
        std::function<void(CollisionObject*, const CollisionResult&)> onCollisionEnter;

        // オブジェクトIDを取得
        uint32_t GetID() const { return id_; }

        // 有効・無効設定
        void SetEnabled(bool enabled) { isEnabled_ = enabled; }
        bool IsEnabled() const { return isEnabled_; }

        // 剛体フラグの設定
        void SetIsRigidbody(bool isRigidbody) { isRigidbody_ = isRigidbody; }
        bool IsRigidbody() const { return isRigidbody_; }

        // 速度の設定
        void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
        const Vector3& GetVelocity() const { return velocity_; }

    protected:
        // コンストラクタは派生クラスからのみ呼び出し可能
        CollisionObject() : id_(nextID_++), isEnabled_(true), isRigidbody_(false), velocity_({ 0, 0, 0 }) {}

    private:
        // オブジェクトID
        uint32_t id_;
        // 有効フラグ
        bool isEnabled_;
        // 剛体フラグ（押し出し処理の対象になるか）
        bool isRigidbody_;
        // 速度ベクトル
        Vector3 velocity_;

        // 次に割り当てるID
        static uint32_t nextID_;
    };

    // 球コリジョン
    class SphereCollider : public CollisionObject {
    public:
        // コンストラクタ
        SphereCollider(const Vector3& center, float radius)
            : sphere_({ center, radius }) {
        }

        // 形状種別の取得
        ShapeType GetShapeType() const override { return ShapeType::Sphere; }

        // 形状データの取得
        void* GetShapeData() override { return &sphere_; }
        const void* GetShapeData() const override { return &sphere_; }

        // 球データへの直接アクセス
        Sphere& GetSphere() { return sphere_; }
        const Sphere& GetSphere() const { return sphere_; }

    private:
        Sphere sphere_;
    };

    // カプセルコリジョン
    class CapsuleCollider : public CollisionObject {
    public:
        // コンストラクタ
        CapsuleCollider(const Vector3& start, const Vector3& end, float radius)
            : capsule_({ start, end }, radius) {
        }

        // 形状種別の取得
        ShapeType GetShapeType() const override { return ShapeType::Capsule; }

        // 形状データの取得
        void* GetShapeData() override { return &capsule_; }
        const void* GetShapeData() const override { return &capsule_; }

        // カプセルデータへの直接アクセス
        Capsule& GetCapsule() { return capsule_; }
        const Capsule& GetCapsule() const { return capsule_; }

    private:
        Capsule capsule_;
    };

    // 衝突マネージャー
    class CollisionManager {
    public:
        // シングルトンインスタンス取得
        static CollisionManager* GetInstance();

        // コリジョンの登録
        void AddCollider(std::shared_ptr<CollisionObject> collider);

        // コリジョンの削除
        void RemoveCollider(std::shared_ptr<CollisionObject> collider);
        void RemoveCollider(uint32_t id);

        // コリジョンのクリア
        void ClearColliders();

        // 衝突判定の更新
        void Update(float deltaTime);

        // デバッグ描画
        void DebugDraw();

    private:
        // シングルトンインスタンス
        static CollisionManager* instance_;

        // コリジョンリスト
        std::vector<std::shared_ptr<CollisionObject>> colliders_;

        // コンストラクタ（シングルトン）
        CollisionManager() = default;
        // デストラクタ（シングルトン）
        ~CollisionManager() = default;
        // コピー禁止
        CollisionManager(const CollisionManager&) = delete;
        CollisionManager& operator=(const CollisionManager&) = delete;

        // 2つのコライダー間の衝突判定
        CollisionResult CheckCollision(
            const CollisionObject* collider1,
            const CollisionObject* collider2
        );

        // 移動を考慮した衝突判定（スウィープテスト）
        CollisionResult CheckSweepCollision(
            const CollisionObject* movingCollider,
            const CollisionObject* staticCollider,
            float deltaTime
        );
    };

} // namespace Collision