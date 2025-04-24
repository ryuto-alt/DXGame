// src/Engine/Collision/Collision.h
#pragma once

#include "CollisionPrimitive.h"
#include "BoundingVolume.h"
#include "MeshCollider.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <unordered_set>

namespace Collision {

    // 前方宣言
    class CollisionManager;

    // コライダー型
    enum class ColliderType {
        None,
        Sphere,
        AABB,
        OBB,
        Mesh,
        ConvexMesh,
        Heightfield
    };

    // コリジョンイベント
    enum class CollisionEvent {
        Enter,   // 衝突開始
        Stay,    // 衝突中
        Exit     // 衝突終了
    };

    // コリジョン情報
    struct CollisionInfo {
        void* pCollider1;             // コライダー1のポインタ
        void* pCollider2;             // コライダー2のポインタ
        uint32_t collider1ID;         // コライダー1のID
        uint32_t collider2ID;         // コライダー2のID
        std::string collider1Name;    // コライダー1の名前
        std::string collider2Name;    // コライダー2の名前
        CollisionResult result;       // 衝突結果
        CollisionEvent event;         // 衝突イベント
    };

    // 衝突フィルター
    struct CollisionFilter {
        uint32_t categoryBits;    // 自身のカテゴリ
        uint32_t maskBits;        // 衝突するカテゴリ

        CollisionFilter(uint32_t category = 0x0001, uint32_t mask = 0xFFFF)
            : categoryBits(category), maskBits(mask) {
        }

        // 衝突判定
        bool CanCollide(const CollisionFilter& other) const {
            return (categoryBits & other.maskBits) != 0 && (maskBits & other.categoryBits) != 0;
        }
    };

    // コリジョンコールバック関数の型定義
    using CollisionCallback = std::function<void(const CollisionInfo&)>;

    // コライダー基底クラス
    class ICollider {
    public:
        // 仮想デストラクタ
        virtual ~ICollider() = default;

        // コライダー型を取得
        virtual ColliderType GetType() const = 0;

        // 衝突判定
        virtual bool CheckCollision(ICollider* other, CollisionResult* result = nullptr) = 0;

        // 有効/無効
        bool IsEnabled() const { return isEnabled_; }
        void SetEnabled(bool enabled) { isEnabled_ = enabled; }

        // 名前
        const std::string& GetName() const { return name_; }
        void SetName(const std::string& name) { name_ = name; }

        // ID
        uint32_t GetID() const { return id_; }

        // フィルター
        const CollisionFilter& GetFilter() const { return filter_; }
        void SetFilter(const CollisionFilter& filter) { filter_ = filter; }

        // コールバック
        void SetCallback(CollisionCallback callback) { callback_ = callback; }

        // 衝突イベント発生
        void OnCollision(const CollisionInfo& info) {
            if (callback_) {
                callback_(info);
            }
        }

    protected:
        friend class CollisionManager;

        // 管理用データ
        bool isEnabled_ = true;              // 有効フラグ
        std::string name_ = "Collider";      // 名前
        uint32_t id_ = 0;                    // ID
        CollisionFilter filter_;             // フィルター
        CollisionCallback callback_ = nullptr; // コールバック
    };

    // 球コライダー
    class SphereCollider : public ICollider {
    public:
        // コンストラクタ
        SphereCollider() = default;
        SphereCollider(const Vector3& center, float radius);

        // コライダー型を取得
        ColliderType GetType() const override { return ColliderType::Sphere; }

        // 衝突判定
        bool CheckCollision(ICollider* other, CollisionResult* result = nullptr) override;

        // スフィアデータ
        const Sphere& GetSphere() const { return sphere_; }
        void SetSphere(const Sphere& sphere) { sphere_ = sphere; }

        // 中心座標
        const Vector3& GetCenter() const { return sphere_.center; }
        void SetCenter(const Vector3& center) { sphere_.center = center; }

        // 半径
        float GetRadius() const { return sphere_.radius; }
        void SetRadius(float radius) { sphere_.radius = radius; }

    private:
        Sphere sphere_;
    };

    // AABBコライダー
    class AABBCollider : public ICollider {
    public:
        // コンストラクタ
        AABBCollider() = default;
        AABBCollider(const Vector3& min, const Vector3& max);

        // コライダー型を取得
        ColliderType GetType() const override { return ColliderType::AABB; }

        // 衝突判定
        bool CheckCollision(ICollider* other, CollisionResult* result = nullptr) override;

        // AABBデータ
        const AABB& GetAABB() const { return aabb_; }
        void SetAABB(const AABB& aabb) { aabb_ = aabb; }

        // 最小座標
        const Vector3& GetMin() const { return aabb_.min; }
        void SetMin(const Vector3& min) { aabb_.min = min; }

        // 最大座標
        const Vector3& GetMax() const { return aabb_.max; }
        void SetMax(const Vector3& max) { aabb_.max = max; }

    private:
        AABB aabb_;
    };

    // OBBコライダー
    class OBBCollider : public ICollider {
    public:
        // コンストラクタ
        OBBCollider() = default;
        OBBCollider(const Vector3& center, const Vector3 orientations[3], const Vector3& size);
        OBBCollider(const Matrix4x4& worldMatrix, const Vector3& size);

        // コライダー型を取得
        ColliderType GetType() const override { return ColliderType::OBB; }

        // 衝突判定
        bool CheckCollision(ICollider* other, CollisionResult* result = nullptr) override;

        // OBBデータ
        const OBB& GetOBB() const { return obb_; }
        void SetOBB(const OBB& obb) { obb_ = obb; }

        // ワールド行列から更新
        void UpdateFromMatrix(const Matrix4x4& worldMatrix, const Vector3& size);

    private:
        OBB obb_;
    };

    // メッシュコライダー
    class MeshCollider : public ICollider {
    public:
        // コンストラクタ
        MeshCollider() = default;

        // コライダー型を取得
        ColliderType GetType() const override { return ColliderType::Mesh; }

        // 衝突判定
        bool CheckCollision(ICollider* other, CollisionResult* result = nullptr) override;

        // メッシュコライダーデータ
        std::shared_ptr<IMeshCollider> GetMeshCollider() const { return meshCollider_; }
        void SetMeshCollider(std::shared_ptr<IMeshCollider> meshCollider) { meshCollider_ = meshCollider; }

        // 三角形メッシュコライダーを作成
        void CreateTriangleMeshCollider(const std::vector<Triangle>& triangles);
        void CreateTriangleMeshCollider(const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices);

        // 凸包メッシュコライダーを作成
        void CreateConvexMeshCollider(const std::vector<Vector3>& vertices);

        // 地形メッシュコライダーを作成
        void CreateHeightfieldCollider(int width, int height, const std::vector<float>& heights,
            float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f);

        // ワールド行列から更新
        void UpdateFromMatrix(const Matrix4x4& worldMatrix);

    private:
        std::shared_ptr<IMeshCollider> meshCollider_;
    };

    // 衝突マネージャー（シングルトン）
    class CollisionManager {
    public:
        // シングルトンインスタンスの取得
        static CollisionManager* GetInstance();

        // インスタンスの削除
        static void DeleteInstance();

        // コリジョンマネージャーの初期化
        void Initialize();

        // コリジョンマネージャーの終了
        void Finalize();

        // コライダーの登録
        void AddCollider(std::shared_ptr<ICollider> collider);

        // コライダーの削除
        void RemoveCollider(uint32_t colliderID);
        void RemoveCollider(const std::string& name);
        void RemoveCollider(ICollider* collider);

        // コライダーの検索
        std::shared_ptr<ICollider> FindCollider(uint32_t colliderID);
        std::shared_ptr<ICollider> FindCollider(const std::string& name);

        // 衝突判定の実行
        void CheckAllCollisions();

        // 衝突関係のクリア
        void ClearCollisions();

        // 衝突判定をスキップするペアの追加
        void AddIgnorePair(uint32_t collider1ID, uint32_t collider2ID);
        void AddIgnorePair(const std::string& collider1Name, const std::string& collider2Name);

        // 衝突判定をスキップするペアの削除
        void RemoveIgnorePair(uint32_t collider1ID, uint32_t collider2ID);
        void RemoveIgnorePair(const std::string& collider1Name, const std::string& collider2Name);

        // デバッグ描画
        void DebugDraw();

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

        // コライダーリスト
        std::vector<std::shared_ptr<ICollider>> colliders_;
        // 名前からコライダーへのマップ
        std::unordered_map<std::string, std::shared_ptr<ICollider>> colliderMap_;
        // IDからコライダーへのマップ
        std::unordered_map<uint32_t, std::shared_ptr<ICollider>> colliderIDMap_;
        // 次のコライダーID
        uint32_t nextColliderID_ = 1;
        // 前回の衝突ペア
        std::unordered_map<uint64_t, bool> prevCollisions_;
        // 衝突スキップペア
        std::unordered_set<uint64_t> ignorePairs_;

        // コライダーペアのハッシュ値を計算
        uint64_t GetColliderPairHash(uint32_t collider1ID, uint32_t collider2ID) const;
    };

    // 衝突判定関数群
    namespace CollisionDetection {
        // 球と球の衝突判定
        bool SphereSphere(const Sphere& sphere1, const Sphere& sphere2, CollisionResult* result = nullptr);

        // 球とAABBの衝突判定
        bool SphereAABB(const Sphere& sphere, const AABB& aabb, CollisionResult* result = nullptr);

        // 球とOBBの衝突判定
        bool SphereOBB(const Sphere& sphere, const OBB& obb, CollisionResult* result = nullptr);

        // 球とメッシュの衝突判定
        bool SphereMesh(const Sphere& sphere, const IMeshCollider& mesh, CollisionResult* result = nullptr);

        // AABBとAABBの衝突判定
        bool AABBAABB(const AABB& aabb1, const AABB& aabb2, CollisionResult* result = nullptr);

        // AABBとOBBの衝突判定
        bool AABBOBB(const AABB& aabb, const OBB& obb, CollisionResult* result = nullptr);

        // AABBとメッシュの衝突判定
        bool AABBMesh(const AABB& aabb, const IMeshCollider& mesh, CollisionResult* result = nullptr);

        // OBBとOBBの衝突判定
        bool OBBOBB(const OBB& obb1, const OBB& obb2, CollisionResult* result = nullptr);

        // OBBとメッシュの衝突判定
        bool OBBMesh(const OBB& obb, const IMeshCollider& mesh, CollisionResult* result = nullptr);

        // レイとメッシュの衝突判定
        bool RayMesh(const Ray& ray, const IMeshCollider& mesh, float maxDistance, CollisionResult* result = nullptr);

        // レイと球の衝突判定
        bool RaySphere(const Ray& ray, const Sphere& sphere, float maxDistance, CollisionResult* result = nullptr);

        // レイとAABBの衝突判定
        bool RayAABB(const Ray& ray, const AABB& aabb, float maxDistance, CollisionResult* result = nullptr);

        // レイとOBBの衝突判定
        bool RayOBB(const Ray& ray, const OBB& obb, float maxDistance, CollisionResult* result = nullptr);

        // 点と球の衝突判定
        bool PointSphere(const Vector3& point, const Sphere& sphere, CollisionResult* result = nullptr);

        // 点とAABBの衝突判定
        bool PointAABB(const Vector3& point, const AABB& aabb, CollisionResult* result = nullptr);

        // 点とOBBの衝突判定
        bool PointOBB(const Vector3& point, const OBB& obb, CollisionResult* result = nullptr);

        // 点とメッシュの衝突判定
        bool PointMesh(const Vector3& point, const IMeshCollider& mesh, CollisionResult* result = nullptr);
    }

} // namespace Collision