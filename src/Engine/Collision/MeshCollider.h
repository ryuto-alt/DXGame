// src/Engine/Collision/MeshCollider.h
#pragma once

#include "CollisionPrimitive.h"
#include "BoundingVolume.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <vector>
#include <memory>

namespace Collision {

    // 衝突結果構造体
    struct CollisionResult {
        bool hasCollision;       // 衝突したかどうか
        Vector3 collisionPoint;  // 衝突点
        Vector3 normal;          // 衝突点での法線ベクトル
        float penetration;       // めり込み量
    };

    // メッシュコライダー基底クラス
    class IMeshCollider {
    public:
        // 仮想デストラクタ
        virtual ~IMeshCollider() = default;

        // 境界ボリュームの取得
        virtual const IBoundingVolume* GetBoundingVolume() const = 0;

        // ワールド変換行列を適用
        virtual void ApplyTransform(const Matrix4x4& worldMatrix) = 0;

        // 点との衝突判定
        virtual bool Intersects(const Vector3& point, CollisionResult* result = nullptr) const = 0;

        // 線分との衝突判定
        virtual bool Intersects(const Line& line, CollisionResult* result = nullptr) const = 0;

        // レイとの衝突判定
        virtual bool Intersects(const Ray& ray, float maxDistance, CollisionResult* result = nullptr) const = 0;

        // 球との衝突判定
        virtual bool Intersects(const Sphere& sphere, CollisionResult* result = nullptr) const = 0;

        // AABBとの衝突判定
        virtual bool Intersects(const AABB& aabb, CollisionResult* result = nullptr) const = 0;

        // OBBとの衝突判定
        virtual bool Intersects(const OBB& obb, CollisionResult* result = nullptr) const = 0;

        // メッシュコライダーとの衝突判定
        virtual bool Intersects(const IMeshCollider& other, CollisionResult* result = nullptr) const = 0;
    };

    // 三角形メッシュコライダー
    class TriangleMeshCollider : public IMeshCollider {
    public:
        // コンストラクタ
        TriangleMeshCollider() = default;
        TriangleMeshCollider(const std::vector<Triangle>& triangles);
        TriangleMeshCollider(const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices);

        // デストラクタ
        ~TriangleMeshCollider() override = default;

        // 境界ボリュームの取得
        const IBoundingVolume* GetBoundingVolume() const override { return boundingVolume.get(); }

        // ワールド変換行列を適用
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点との衝突判定
        bool Intersects(const Vector3& point, CollisionResult* result = nullptr) const override;

        // 線分との衝突判定
        bool Intersects(const Line& line, CollisionResult* result = nullptr) const override;

        // レイとの衝突判定
        bool Intersects(const Ray& ray, float maxDistance, CollisionResult* result = nullptr) const override;

        // 球との衝突判定
        bool Intersects(const Sphere& sphere, CollisionResult* result = nullptr) const override;

        // AABBとの衝突判定
        bool Intersects(const AABB& aabb, CollisionResult* result = nullptr) const override;

        // OBBとの衝突判定
        bool Intersects(const OBB& obb, CollisionResult* result = nullptr) const override;

        // メッシュコライダーとの衝突判定
        bool Intersects(const IMeshCollider& other, CollisionResult* result = nullptr) const override;

        // 三角形の追加
        void AddTriangle(const Triangle& triangle);

        // 三角形メッシュの設定
        void SetTriangles(const std::vector<Triangle>& triangles);

        // 三角形メッシュの取得
        const std::vector<Triangle>& GetTriangles() const { return triangles_; }

        // 境界ボリュームを再計算
        void RecalculateBoundingVolume();

        // 三角形のためのレイキャスト
        bool RaycastTriangle(const Ray& ray, const Triangle& triangle, float maxDistance, CollisionResult* result) const;

    private:
        std::vector<Triangle> triangles_;                     // 三角形リスト
        std::shared_ptr<IBoundingVolume> boundingVolume;      // 境界ボリューム
    };

    // 凸包メッシュコライダー
    class ConvexMeshCollider : public IMeshCollider {
    public:
        // コンストラクタ
        ConvexMeshCollider() = default;
        ConvexMeshCollider(const std::vector<Vector3>& vertices);

        // デストラクタ
        ~ConvexMeshCollider() override = default;

        // 境界ボリュームの取得
        const IBoundingVolume* GetBoundingVolume() const override { return boundingVolume.get(); }

        // ワールド変換行列を適用
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点との衝突判定
        bool Intersects(const Vector3& point, CollisionResult* result = nullptr) const override;

        // 線分との衝突判定
        bool Intersects(const Line& line, CollisionResult* result = nullptr) const override;

        // レイとの衝突判定
        bool Intersects(const Ray& ray, float maxDistance, CollisionResult* result = nullptr) const override;

        // 球との衝突判定
        bool Intersects(const Sphere& sphere, CollisionResult* result = nullptr) const override;

        // AABBとの衝突判定
        bool Intersects(const AABB& aabb, CollisionResult* result = nullptr) const override;

        // OBBとの衝突判定
        bool Intersects(const OBB& obb, CollisionResult* result = nullptr) const override;

        // メッシュコライダーとの衝突判定
        bool Intersects(const IMeshCollider& other, CollisionResult* result = nullptr) const override;

        // 頂点の追加
        void AddVertex(const Vector3& vertex);

        // 頂点群の設定
        void SetVertices(const std::vector<Vector3>& vertices);

        // 頂点群の取得
        const std::vector<Vector3>& GetVertices() const { return vertices_; }

        // 境界ボリュームを再計算
        void RecalculateBoundingVolume();

    private:
        std::vector<Vector3> vertices_;                      // 頂点リスト
        std::vector<Triangle> triangles_;                    // 三角形リスト（凸包計算結果）
        std::shared_ptr<IBoundingVolume> boundingVolume;     // 境界ボリューム

        // 凸包の計算
        void CalculateConvexHull();
    };

    // 地形メッシュコライダー（高さマップベース）
    class HeightfieldCollider : public IMeshCollider {
    public:
        // コンストラクタ
        HeightfieldCollider() = default;
        HeightfieldCollider(int width, int height, const std::vector<float>& heights, float scaleX, float scaleY, float scaleZ);

        // デストラクタ
        ~HeightfieldCollider() override = default;

        // 境界ボリュームの取得
        const IBoundingVolume* GetBoundingVolume() const override { return boundingVolume.get(); }

        // ワールド変換行列を適用
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点との衝突判定
        bool Intersects(const Vector3& point, CollisionResult* result = nullptr) const override;

        // 線分との衝突判定
        bool Intersects(const Line& line, CollisionResult* result = nullptr) const override;

        // レイとの衝突判定
        bool Intersects(const Ray& ray, float maxDistance, CollisionResult* result = nullptr) const override;

        // 球との衝突判定
        bool Intersects(const Sphere& sphere, CollisionResult* result = nullptr) const override;

        // AABBとの衝突判定
        bool Intersects(const AABB& aabb, CollisionResult* result = nullptr) const override;

        // OBBとの衝突判定
        bool Intersects(const OBB& obb, CollisionResult* result = nullptr) const override;

        // メッシュコライダーとの衝突判定
        bool Intersects(const IMeshCollider& other, CollisionResult* result = nullptr) const override;

        // 境界ボリュームを再計算
        void RecalculateBoundingVolume();

        // 高さを取得（ワールド座標ではなく、高さマップのローカル座標系）
        float GetHeight(float x, float z) const;

        // 指定された座標の三角形を取得（2つの三角形が返る可能性あり）
        bool GetTrianglesAt(float x, float z, Triangle* triangle1, Triangle* triangle2 = nullptr) const;

    private:
        int width_ = 0;                                       // 幅
        int height_ = 0;                                      // 高さ
        std::vector<float> heights_;                          // 高さデータ
        float scaleX_ = 1.0f;                                 // X軸スケール
        float scaleY_ = 1.0f;                                 // Y軸スケール（高さ方向）
        float scaleZ_ = 1.0f;                                 // Z軸スケール
        Matrix4x4 worldMatrix_ = MakeIdentity4x4();           // ワールド変換行列
        std::shared_ptr<IBoundingVolume> boundingVolume;      // 境界ボリューム
    };

} // namespace Collision