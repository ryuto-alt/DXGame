// src/Engine/Collision/BoundingVolume.h
#pragma once

#include "CollisionPrimitive.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <vector>
#include <memory>

namespace Collision {

    // 境界ボリュームの種類
    enum class BoundingVolumeType {
        Sphere,     // 球
        AABB,       // 軸平行境界ボックス
        OBB         // 有向境界ボックス
    };

    // 境界ボリュームの基底クラス
    class IBoundingVolume {
    public:
        // 仮想デストラクタ
        virtual ~IBoundingVolume() = default;

        // 境界ボリュームの種類を取得
        virtual BoundingVolumeType GetType() const = 0;

        // ワールド変換行列を適用
        virtual void ApplyTransform(const Matrix4x4& worldMatrix) = 0;

        // 点が内部にあるかどうかを判定
        virtual bool Contains(const Vector3& point) const = 0;

        // 可視化用の面の頂点を取得
        virtual std::vector<Vector3> GetVisualizationVertices() const = 0;
    };

    // 境界球
    class BoundingSphere : public IBoundingVolume {
    public:
        // コンストラクタ
        BoundingSphere() = default;
        BoundingSphere(const Vector3& center, float radius);
        BoundingSphere(const std::vector<Vector3>& points);

        // 境界ボリュームの種類を取得
        BoundingVolumeType GetType() const override { return BoundingVolumeType::Sphere; }

        // ワールド変換行列を適用
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点が内部にあるかどうかを判定
        bool Contains(const Vector3& point) const override;

        // 可視化用の面の頂点を取得
        std::vector<Vector3> GetVisualizationVertices() const override;

        // Sphereを取得
        const Sphere& GetSphere() const { return sphere_; }

        // 中心座標を取得
        Vector3 GetCenter() const { return sphere_.center; }

        // 半径を取得
        float GetRadius() const { return sphere_.radius; }

    private:
        Sphere sphere_;  // 球
    };

    // 境界AABB
    class BoundingAABB : public IBoundingVolume {
    public:
        // コンストラクタ
        BoundingAABB() = default;
        BoundingAABB(const Vector3& min, const Vector3& max);
        BoundingAABB(const std::vector<Vector3>& points);

        // 境界ボリュームの種類を取得
        BoundingVolumeType GetType() const override { return BoundingVolumeType::AABB; }

        // ワールド変換行列を適用（注：AABBは回転に対応できないため、新しいAABBを計算します）
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点が内部にあるかどうかを判定
        bool Contains(const Vector3& point) const override;

        // 可視化用の面の頂点を取得
        std::vector<Vector3> GetVisualizationVertices() const override;

        // AABBを取得
        const AABB& GetAABB() const { return aabb_; }

        // 最小座標を取得
        Vector3 GetMin() const { return aabb_.min; }

        // 最大座標を取得
        Vector3 GetMax() const { return aabb_.max; }

        // 中心座標を取得
        Vector3 GetCenter() const { return aabb_.GetCenter(); }

        // サイズを取得
        Vector3 GetSize() const { return aabb_.GetSize(); }

    private:
        AABB aabb_;  // AABB
    };

    // 境界OBB
    class BoundingOBB : public IBoundingVolume {
    public:
        // コンストラクタ
        BoundingOBB() = default;
        BoundingOBB(const Vector3& center, const Vector3 orientations[3], const Vector3& size);
        BoundingOBB(const Matrix4x4& worldMatrix, const Vector3& size);

        // 境界ボリュームの種類を取得
        BoundingVolumeType GetType() const override { return BoundingVolumeType::OBB; }

        // ワールド変換行列を適用
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点が内部にあるかどうかを判定
        bool Contains(const Vector3& point) const override;

        // 可視化用の面の頂点を取得
        std::vector<Vector3> GetVisualizationVertices() const override;

        // OBBを取得
        const OBB& GetOBB() const { return obb_; }

        // 中心座標を取得
        Vector3 GetCenter() const { return obb_.center; }

        // 各軸方向のサイズを取得
        Vector3 GetSize() const { return obb_.size; }

    private:
        OBB obb_;  // OBB
    };

    // 複合境界ボリューム（複数の境界ボリュームを含む）
    class CompoundBoundingVolume : public IBoundingVolume {
    public:
        // コンストラクタ
        CompoundBoundingVolume() = default;

        // 境界ボリュームの種類を取得（便宜上AABBとして扱う）
        BoundingVolumeType GetType() const override { return BoundingVolumeType::AABB; }

        // ワールド変換行列を適用
        void ApplyTransform(const Matrix4x4& worldMatrix) override;

        // 点が内部にあるかどうかを判定
        bool Contains(const Vector3& point) const override;

        // 可視化用の面の頂点を取得
        std::vector<Vector3> GetVisualizationVertices() const override;

        // 境界ボリュームを追加
        void AddBoundingVolume(std::shared_ptr<IBoundingVolume> boundingVolume);

        // 境界ボリュームを取得
        const std::vector<std::shared_ptr<IBoundingVolume>>& GetBoundingVolumes() const { return boundingVolumes_; }

    private:
        std::vector<std::shared_ptr<IBoundingVolume>> boundingVolumes_;  // 境界ボリュームリスト
    };

} // namespace Collision