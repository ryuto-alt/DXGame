// src/Engine/Collision/BoundingVolume.cpp
#include "BoundingVolume.h"
#include <algorithm>
#include <limits>
#include <cmath>
float GetVectorComponent(const Vector3& vec, int index) {
    switch (index) {
    case 0: return vec.x;
    case 1: return vec.y;
    case 2: return vec.z;
    default: return 0.0f; // エラー防止用
    }
}

namespace Collision {

    //-----------------------------------------------------------------------------
    // BoundingSphere実装
    //-----------------------------------------------------------------------------

    BoundingSphere::BoundingSphere(const Vector3& center, float radius) {
        sphere_.center = center;
        sphere_.radius = radius;
    }

    BoundingSphere::BoundingSphere(const std::vector<Vector3>& points) {
        // 点群から境界球を計算（シンプルな実装）

        // 1. 中心座標の計算（点の平均）
        Vector3 center = { 0, 0, 0 };

        if (points.empty()) {
            sphere_ = Sphere({ 0, 0, 0 }, 1.0f);
            return;
        }

        for (const auto& point : points) {
            center.x += point.x;
            center.y += point.y;
            center.z += point.z;
        }

        center.x /= static_cast<float>(points.size());
        center.y /= static_cast<float>(points.size());
        center.z /= static_cast<float>(points.size());

        // 2. 半径の計算（中心から最も遠い点までの距離）
        float maxDistSq = 0.0f;

        for (const auto& point : points) {
            float dx = point.x - center.x;
            float dy = point.y - center.y;
            float dz = point.z - center.z;

            float distSq = dx * dx + dy * dy + dz * dz;
            maxDistSq = std::max(maxDistSq, distSq);
        }

        sphere_.center = center;
        sphere_.radius = std::sqrt(maxDistSq);
    }

    void BoundingSphere::ApplyTransform(const Matrix4x4& worldMatrix) {
        // 中心点の変換
        Vector4 center = { sphere_.center.x, sphere_.center.y, sphere_.center.z, 1.0f };
        Vector4 transformedCenter;

        transformedCenter.x =
            center.x * worldMatrix.m[0][0] +
            center.y * worldMatrix.m[1][0] +
            center.z * worldMatrix.m[2][0] +
            center.w * worldMatrix.m[3][0];

        transformedCenter.y =
            center.x * worldMatrix.m[0][1] +
            center.y * worldMatrix.m[1][1] +
            center.z * worldMatrix.m[2][1] +
            center.w * worldMatrix.m[3][1];

        transformedCenter.z =
            center.x * worldMatrix.m[0][2] +
            center.y * worldMatrix.m[1][2] +
            center.z * worldMatrix.m[2][2] +
            center.w * worldMatrix.m[3][2];

        transformedCenter.w =
            center.x * worldMatrix.m[0][3] +
            center.y * worldMatrix.m[1][3] +
            center.z * worldMatrix.m[2][3] +
            center.w * worldMatrix.m[3][3];

        if (transformedCenter.w != 0.0f) {
            transformedCenter.x /= transformedCenter.w;
            transformedCenter.y /= transformedCenter.w;
            transformedCenter.z /= transformedCenter.w;
        }

        sphere_.center = { transformedCenter.x, transformedCenter.y, transformedCenter.z };

        // スケール係数の計算
        // 簡易版：最大スケール係数を使用
        float scaleX = std::sqrt(
            worldMatrix.m[0][0] * worldMatrix.m[0][0] +
            worldMatrix.m[0][1] * worldMatrix.m[0][1] +
            worldMatrix.m[0][2] * worldMatrix.m[0][2]);

        float scaleY = std::sqrt(
            worldMatrix.m[1][0] * worldMatrix.m[1][0] +
            worldMatrix.m[1][1] * worldMatrix.m[1][1] +
            worldMatrix.m[1][2] * worldMatrix.m[1][2]);

        float scaleZ = std::sqrt(
            worldMatrix.m[2][0] * worldMatrix.m[2][0] +
            worldMatrix.m[2][1] * worldMatrix.m[2][1] +
            worldMatrix.m[2][2] * worldMatrix.m[2][2]);

        float maxScale = std::max({ scaleX, scaleY, scaleZ });

        // 半径の更新
        sphere_.radius *= maxScale;
    }

    bool BoundingSphere::Contains(const Vector3& point) const {
        float dx = point.x - sphere_.center.x;
        float dy = point.y - sphere_.center.y;
        float dz = point.z - sphere_.center.z;

        float distSquared = dx * dx + dy * dy + dz * dz;

        return distSquared <= (sphere_.radius * sphere_.radius);
    }

    std::vector<Vector3> BoundingSphere::GetVisualizationVertices() const {
        // 簡易的な球の頂点を生成（均等に分布した点を使用）
        std::vector<Vector3> vertices;
        const int segments = 16;
        const int rings = 8;

        for (int i = 0; i <= rings; i++) {
            float phi = static_cast<float>(i) / rings * 3.14159f;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            for (int j = 0; j < segments; j++) {
                float theta = static_cast<float>(j) / segments * 3.14159f * 2.0f;
                float sinTheta = std::sin(theta);
                float cosTheta = std::cos(theta);

                float x = sphere_.center.x + sphere_.radius * sinPhi * cosTheta;
                float y = sphere_.center.y + sphere_.radius * cosPhi;
                float z = sphere_.center.z + sphere_.radius * sinPhi * sinTheta;

                vertices.push_back({ x, y, z });
            }
        }

        return vertices;
    }

    //-----------------------------------------------------------------------------
    // BoundingAABB実装
    //-----------------------------------------------------------------------------

    BoundingAABB::BoundingAABB(const Vector3& min, const Vector3& max) {
        aabb_.min = min;
        aabb_.max = max;
    }

    BoundingAABB::BoundingAABB(const std::vector<Vector3>& points) {
        // 点群からAABBを計算

        if (points.empty()) {
            aabb_ = AABB({ -1, -1, -1 }, { 1, 1, 1 });
            return;
        }

        // 最初の点で初期化
        Vector3 min = points[0];
        Vector3 max = points[0];

        // 残りの点を走査して最小・最大値を更新
        for (size_t i = 1; i < points.size(); i++) {
            // X座標
            if (points[i].x < min.x) min.x = points[i].x;
            if (points[i].x > max.x) max.x = points[i].x;

            // Y座標
            if (points[i].y < min.y) min.y = points[i].y;
            if (points[i].y > max.y) max.y = points[i].y;

            // Z座標
            if (points[i].z < min.z) min.z = points[i].z;
            if (points[i].z > max.z) max.z = points[i].z;
        }

        aabb_.min = min;
        aabb_.max = max;
    }

    void BoundingAABB::ApplyTransform(const Matrix4x4& worldMatrix) {
        // AABBは回転に対応できないため、元のAABBの8頂点を変換し、
        // 変換後の点から新しいAABBを計算する

        // 元のAABBの8頂点を取得
        std::vector<Vector3> corners = aabb_.GetVertices();
        std::vector<Vector3> transformedCorners;
        transformedCorners.reserve(8);

        // 各頂点にワールド変換を適用
        for (const auto& corner : corners) {
            Vector4 point = { corner.x, corner.y, corner.z, 1.0f };
            Vector4 transformed;

            transformed.x =
                point.x * worldMatrix.m[0][0] +
                point.y * worldMatrix.m[1][0] +
                point.z * worldMatrix.m[2][0] +
                point.w * worldMatrix.m[3][0];

            transformed.y =
                point.x * worldMatrix.m[0][1] +
                point.y * worldMatrix.m[1][1] +
                point.z * worldMatrix.m[2][1] +
                point.w * worldMatrix.m[3][1];

            transformed.z =
                point.x * worldMatrix.m[0][2] +
                point.y * worldMatrix.m[1][2] +
                point.z * worldMatrix.m[2][2] +
                point.w * worldMatrix.m[3][2];

            transformed.w =
                point.x * worldMatrix.m[0][3] +
                point.y * worldMatrix.m[1][3] +
                point.z * worldMatrix.m[2][3] +
                point.w * worldMatrix.m[3][3];

            if (transformed.w != 0.0f) {
                transformed.x /= transformed.w;
                transformed.y /= transformed.w;
                transformed.z /= transformed.w;
            }

            transformedCorners.push_back({ transformed.x, transformed.y, transformed.z });
        }

        // 変換後の点から新しいAABBを計算
        *this = BoundingAABB(transformedCorners);
    }

    bool BoundingAABB::Contains(const Vector3& point) const {
        return (
            point.x >= aabb_.min.x && point.x <= aabb_.max.x &&
            point.y >= aabb_.min.y && point.y <= aabb_.max.y &&
            point.z >= aabb_.min.z && point.z <= aabb_.max.z
            );
    }

    std::vector<Vector3> BoundingAABB::GetVisualizationVertices() const {
        // AABBの8頂点を返す
        return aabb_.GetVertices();
    }

    //-----------------------------------------------------------------------------
    // BoundingOBB実装
    //-----------------------------------------------------------------------------

    BoundingOBB::BoundingOBB(const Vector3& center, const Vector3 orientations[3], const Vector3& size) {
        obb_.center = center;
        obb_.orientations[0] = orientations[0];
        obb_.orientations[1] = orientations[1];
        obb_.orientations[2] = orientations[2];
        obb_.size = size;
    }

    BoundingOBB::BoundingOBB(const Matrix4x4& worldMatrix, const Vector3& size) {
        obb_ = OBB::CreateFromMatrix(worldMatrix, size);
    }

    void BoundingOBB::ApplyTransform(const Matrix4x4& worldMatrix) {
        // 中心点の変換
        Vector4 center = { obb_.center.x, obb_.center.y, obb_.center.z, 1.0f };
        Vector4 transformedCenter;

        transformedCenter.x =
            center.x * worldMatrix.m[0][0] +
            center.y * worldMatrix.m[1][0] +
            center.z * worldMatrix.m[2][0] +
            center.w * worldMatrix.m[3][0];

        transformedCenter.y =
            center.x * worldMatrix.m[0][1] +
            center.y * worldMatrix.m[1][1] +
            center.z * worldMatrix.m[2][1] +
            center.w * worldMatrix.m[3][1];

        transformedCenter.z =
            center.x * worldMatrix.m[0][2] +
            center.y * worldMatrix.m[1][2] +
            center.z * worldMatrix.m[2][2] +
            center.w * worldMatrix.m[3][2];

        transformedCenter.w =
            center.x * worldMatrix.m[0][3] +
            center.y * worldMatrix.m[1][3] +
            center.z * worldMatrix.m[2][3] +
            center.w * worldMatrix.m[3][3];

        if (transformedCenter.w != 0.0f) {
            transformedCenter.x /= transformedCenter.w;
            transformedCenter.y /= transformedCenter.w;
            transformedCenter.z /= transformedCenter.w;
        }

        obb_.center = { transformedCenter.x, transformedCenter.y, transformedCenter.z };

        // 軸の向きの変換（回転部分のみ適用）
        for (int i = 0; i < 3; i++) {
            Vector3 axis = obb_.orientations[i];

            Vector3 transformedAxis;
            transformedAxis.x =
                axis.x * worldMatrix.m[0][0] +
                axis.y * worldMatrix.m[1][0] +
                axis.z * worldMatrix.m[2][0];

            transformedAxis.y =
                axis.x * worldMatrix.m[0][1] +
                axis.y * worldMatrix.m[1][1] +
                axis.z * worldMatrix.m[2][1];

            transformedAxis.z =
                axis.x * worldMatrix.m[0][2] +
                axis.y * worldMatrix.m[1][2] +
                axis.z * worldMatrix.m[2][2];

            // 正規化
            float length = std::sqrt(
                transformedAxis.x * transformedAxis.x +
                transformedAxis.y * transformedAxis.y +
                transformedAxis.z * transformedAxis.z);

            if (length > 0.0f) {
                transformedAxis.x /= length;
                transformedAxis.y /= length;
                transformedAxis.z /= length;
            }

            obb_.orientations[i] = transformedAxis;
        }

        // サイズの変換（スケール係数を適用）
        float scaleX = std::sqrt(
            worldMatrix.m[0][0] * worldMatrix.m[0][0] +
            worldMatrix.m[0][1] * worldMatrix.m[0][1] +
            worldMatrix.m[0][2] * worldMatrix.m[0][2]);

        float scaleY = std::sqrt(
            worldMatrix.m[1][0] * worldMatrix.m[1][0] +
            worldMatrix.m[1][1] * worldMatrix.m[1][1] +
            worldMatrix.m[1][2] * worldMatrix.m[1][2]);

        float scaleZ = std::sqrt(
            worldMatrix.m[2][0] * worldMatrix.m[2][0] +
            worldMatrix.m[2][1] * worldMatrix.m[2][1] +
            worldMatrix.m[2][2] * worldMatrix.m[2][2]);

        obb_.size.x *= scaleX;
        obb_.size.y *= scaleY;
        obb_.size.z *= scaleZ;
    }

    bool BoundingOBB::Contains(const Vector3& point) const {
        // OBBのローカル座標系に変換
        Vector3 dir = {
            point.x - obb_.center.x,
            point.y - obb_.center.y,
            point.z - obb_.center.z
        };

        // 各軸に投影
        for (int i = 0; i < 3; i++) {
            // 内積で軸方向の座標を計算
            float distance =
                dir.x * obb_.orientations[i].x +
                dir.y * obb_.orientations[i].y +
                dir.z * obb_.orientations[i].z;

            // 範囲外ならfalse
            if (std::abs(distance) > GetVectorComponent(obb_.size, i)) {
                return false;
            }
        }

        return true;
    }

    std::vector<Vector3> BoundingOBB::GetVisualizationVertices() const {
        std::vector<Vector3> vertices(8);

        // OBBの8頂点を計算
        vertices[0] = {
            obb_.center.x - obb_.orientations[0].x * obb_.size.x - obb_.orientations[1].x * obb_.size.y - obb_.orientations[2].x * obb_.size.z,
            obb_.center.y - obb_.orientations[0].y * obb_.size.x - obb_.orientations[1].y * obb_.size.y - obb_.orientations[2].y * obb_.size.z,
            obb_.center.z - obb_.orientations[0].z * obb_.size.x - obb_.orientations[1].z * obb_.size.y - obb_.orientations[2].z * obb_.size.z
        };

        vertices[1] = {
            obb_.center.x + obb_.orientations[0].x * obb_.size.x - obb_.orientations[1].x * obb_.size.y - obb_.orientations[2].x * obb_.size.z,
            obb_.center.y + obb_.orientations[0].y * obb_.size.x - obb_.orientations[1].y * obb_.size.y - obb_.orientations[2].y * obb_.size.z,
            obb_.center.z + obb_.orientations[0].z * obb_.size.x - obb_.orientations[1].z * obb_.size.y - obb_.orientations[2].z * obb_.size.z
        };

        vertices[2] = {
            obb_.center.x - obb_.orientations[0].x * obb_.size.x + obb_.orientations[1].x * obb_.size.y - obb_.orientations[2].x * obb_.size.z,
            obb_.center.y - obb_.orientations[0].y * obb_.size.x + obb_.orientations[1].y * obb_.size.y - obb_.orientations[2].y * obb_.size.z,
            obb_.center.z - obb_.orientations[0].z * obb_.size.x + obb_.orientations[1].z * obb_.size.y - obb_.orientations[2].z * obb_.size.z
        };

        vertices[3] = {
            obb_.center.x + obb_.orientations[0].x * obb_.size.x + obb_.orientations[1].x * obb_.size.y - obb_.orientations[2].x * obb_.size.z,
            obb_.center.y + obb_.orientations[0].y * obb_.size.x + obb_.orientations[1].y * obb_.size.y - obb_.orientations[2].y * obb_.size.z,
            obb_.center.z + obb_.orientations[0].z * obb_.size.x + obb_.orientations[1].z * obb_.size.y - obb_.orientations[2].z * obb_.size.z
        };

        vertices[4] = {
            obb_.center.x - obb_.orientations[0].x * obb_.size.x - obb_.orientations[1].x * obb_.size.y + obb_.orientations[2].x * obb_.size.z,
            obb_.center.y - obb_.orientations[0].y * obb_.size.x - obb_.orientations[1].y * obb_.size.y + obb_.orientations[2].y * obb_.size.z,
            obb_.center.z - obb_.orientations[0].z * obb_.size.x - obb_.orientations[1].z * obb_.size.y + obb_.orientations[2].z * obb_.size.z
        };

        vertices[5] = {
            obb_.center.x + obb_.orientations[0].x * obb_.size.x - obb_.orientations[1].x * obb_.size.y + obb_.orientations[2].x * obb_.size.z,
            obb_.center.y + obb_.orientations[0].y * obb_.size.x - obb_.orientations[1].y * obb_.size.y + obb_.orientations[2].y * obb_.size.z,
            obb_.center.z + obb_.orientations[0].z * obb_.size.x - obb_.orientations[1].z * obb_.size.y + obb_.orientations[2].z * obb_.size.z
        };

        vertices[6] = {
            obb_.center.x - obb_.orientations[0].x * obb_.size.x + obb_.orientations[1].x * obb_.size.y + obb_.orientations[2].x * obb_.size.z,
            obb_.center.y - obb_.orientations[0].y * obb_.size.x + obb_.orientations[1].y * obb_.size.y + obb_.orientations[2].y * obb_.size.z,
            obb_.center.z - obb_.orientations[0].z * obb_.size.x + obb_.orientations[1].z * obb_.size.y + obb_.orientations[2].z * obb_.size.z
        };

        vertices[7] = {
            obb_.center.x + obb_.orientations[0].x * obb_.size.x + obb_.orientations[1].x * obb_.size.y + obb_.orientations[2].x * obb_.size.z,
            obb_.center.y + obb_.orientations[0].y * obb_.size.x + obb_.orientations[1].y * obb_.size.y + obb_.orientations[2].y * obb_.size.z,
            obb_.center.z + obb_.orientations[0].z * obb_.size.x + obb_.orientations[1].z * obb_.size.y + obb_.orientations[2].z * obb_.size.z
        };

        return vertices;
    }

    //-----------------------------------------------------------------------------
    // CompoundBoundingVolume実装
    //-----------------------------------------------------------------------------

    void CompoundBoundingVolume::ApplyTransform(const Matrix4x4& worldMatrix) {
        for (auto& volume : boundingVolumes_) {
            volume->ApplyTransform(worldMatrix);
        }
    }

    bool CompoundBoundingVolume::Contains(const Vector3& point) const {
        // いずれかのボリュームに含まれていればtrue
        for (const auto& volume : boundingVolumes_) {
            if (volume->Contains(point)) {
                return true;
            }
        }

        return false;
    }

    std::vector<Vector3> CompoundBoundingVolume::GetVisualizationVertices() const {
        std::vector<Vector3> vertices;

        // 各ボリュームの頂点を結合
        for (const auto& volume : boundingVolumes_) {
            std::vector<Vector3> volumeVertices = volume->GetVisualizationVertices();
            vertices.insert(vertices.end(), volumeVertices.begin(), volumeVertices.end());
        }

        return vertices;
    }

    void CompoundBoundingVolume::AddBoundingVolume(std::shared_ptr<IBoundingVolume> boundingVolume) {
        boundingVolumes_.push_back(boundingVolume);
    }

} // namespace Collision