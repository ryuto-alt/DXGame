// src/Engine/Collision/CollisionPrimitive.h
#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Mymath.h"
#include <cmath>
#include <vector>

namespace Collision {

    // 球
    struct Sphere {
        Vector3 center;  // 中心座標
        float radius;    // 半径

        // デフォルトコンストラクタ
        Sphere() : center({ 0, 0, 0 }), radius(1.0f) {}

        // パラメータ付きコンストラクタ
        Sphere(const Vector3& center, float radius) : center(center), radius(radius) {}
    };

    // 軸平行境界ボックス (AABB: Axis-Aligned Bounding Box)
    struct AABB {
        Vector3 min;  // 最小座標
        Vector3 max;  // 最大座標

        // デフォルトコンストラクタ
        AABB() : min({ -1, -1, -1 }), max({ 1, 1, 1 }) {}

        // パラメータ付きコンストラクタ
        AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}

        // 中心座標を取得
        Vector3 GetCenter() const {
            return {
                (min.x + max.x) * 0.5f,
                (min.y + max.y) * 0.5f,
                (min.z + max.z) * 0.5f
            };
        }

        // サイズを取得
        Vector3 GetSize() const {
            return {
                max.x - min.x,
                max.y - min.y,
                max.z - min.z
            };
        }

        // 頂点を取得
        std::vector<Vector3> GetVertices() const {
            std::vector<Vector3> vertices(8);
            vertices[0] = { min.x, min.y, min.z };
            vertices[1] = { max.x, min.y, min.z };
            vertices[2] = { min.x, max.y, min.z };
            vertices[3] = { max.x, max.y, min.z };
            vertices[4] = { min.x, min.y, max.z };
            vertices[5] = { max.x, min.y, max.z };
            vertices[6] = { min.x, max.y, max.z };
            vertices[7] = { max.x, max.y, max.z };
            return vertices;
        }
    };

    // 有向境界ボックス (OBB: Oriented Bounding Box)
    struct OBB {
        Vector3 center;              // 中心座標
        Vector3 orientations[3];     // 各軸の傾き（正規化済み）
        Vector3 size;                // 各軸方向の幅/2（中心から面までの距離）

        // デフォルトコンストラクタ
        OBB() : center({ 0, 0, 0 }), size({ 1, 1, 1 }) {
            orientations[0] = { 1, 0, 0 };  // X軸
            orientations[1] = { 0, 1, 0 };  // Y軸
            orientations[2] = { 0, 0, 1 };  // Z軸
        }

        // パラメータ付きコンストラクタ
        OBB(const Vector3& center, const Vector3 orientations[3], const Vector3& size)
            : center(center), size(size) {
            this->orientations[0] = orientations[0];
            this->orientations[1] = orientations[1];
            this->orientations[2] = orientations[2];
        }

        // 行列から作成
        static OBB CreateFromMatrix(const Matrix4x4& matrix, const Vector3& size) {
            OBB result;

            // 中心座標
            result.center = {
                matrix.m[3][0],
                matrix.m[3][1],
                matrix.m[3][2]
            };

            // 軸の向き（正規化）
            Vector3 xAxis = { matrix.m[0][0], matrix.m[0][1], matrix.m[0][2] };
            Vector3 yAxis = { matrix.m[1][0], matrix.m[1][1], matrix.m[1][2] };
            Vector3 zAxis = { matrix.m[2][0], matrix.m[2][1], matrix.m[2][2] };

            // 正規化
            float xLength = std::sqrt(xAxis.x * xAxis.x + xAxis.y * xAxis.y + xAxis.z * xAxis.z);
            float yLength = std::sqrt(yAxis.x * yAxis.x + yAxis.y * yAxis.y + yAxis.z * yAxis.z);
            float zLength = std::sqrt(zAxis.x * zAxis.x + zAxis.y * zAxis.y + zAxis.z * zAxis.z);

            if (xLength > 0.0f) {
                result.orientations[0] = { xAxis.x / xLength, xAxis.y / xLength, xAxis.z / xLength };
            }

            if (yLength > 0.0f) {
                result.orientations[1] = { yAxis.x / yLength, yAxis.y / yLength, yAxis.z / yLength };
            }

            if (zLength > 0.0f) {
                result.orientations[2] = { zAxis.x / zLength, zAxis.y / zLength, zAxis.z / zLength };
            }

            result.size = size;

            return result;
        }
    };

    // 平面
    struct Plane {
        Vector3 normal;  // 法線ベクトル（正規化済み）
        float distance;  // 原点からの距離

        // デフォルトコンストラクタ
        Plane() : normal({ 0, 1, 0 }), distance(0) {}

        // パラメータ付きコンストラクタ
        Plane(const Vector3& normal, float distance) {
            // 法線ベクトルを正規化
            float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (length > 0) {
                this->normal = {
                    normal.x / length,
                    normal.y / length,
                    normal.z / length
                };
                this->distance = distance / length;
            }
            else {
                this->normal = { 0, 1, 0 };
                this->distance = 0;
            }
        }

        // 点と法線から平面を作成
        static Plane CreateFromPointNormal(const Vector3& point, const Vector3& normal) {
            // 法線ベクトルを正規化
            float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

            Vector3 normalizedNormal;
            if (length > 0) {
                normalizedNormal = {
                    normal.x / length,
                    normal.y / length,
                    normal.z / length
                };
            }
            else {
                normalizedNormal = { 0, 1, 0 };
            }

            // 平面の方程式: ax + by + cz + d = 0 の d を計算
            float d = -(normalizedNormal.x * point.x +
                normalizedNormal.y * point.y +
                normalizedNormal.z * point.z);

            return Plane(normalizedNormal, d);
        }
    };

    // 線分
    struct Line {
        Vector3 start;   // 始点
        Vector3 end;     // 終点

        // デフォルトコンストラクタ
        Line() : start({ 0, 0, 0 }), end({ 1, 0, 0 }) {}

        // パラメータ付きコンストラクタ
        Line(const Vector3& start, const Vector3& end) : start(start), end(end) {}

        // 方向ベクトルを取得
        Vector3 GetDirection() const {
            return {
                end.x - start.x,
                end.y - start.y,
                end.z - start.z
            };
        }

        // 長さを取得
        float GetLength() const {
            Vector3 dir = GetDirection();
            return std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        }
    };

    // 線分（始点と方向）
    struct Ray {
        Vector3 origin;      // 始点
        Vector3 direction;   // 方向ベクトル（正規化済み）

        // デフォルトコンストラクタ
        Ray() : origin({ 0, 0, 0 }), direction({ 1, 0, 0 }) {}

        // パラメータ付きコンストラクタ
        Ray(const Vector3& origin, const Vector3& direction) : origin(origin) {
            // 方向ベクトルを正規化
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
            if (length > 0) {
                this->direction = {
                    direction.x / length,
                    direction.y / length,
                    direction.z / length
                };
            }
            else {
                this->direction = { 1, 0, 0 };
            }
        }

        // 線分から作成
        static Ray CreateFromLine(const Line& line) {
            Vector3 direction = line.GetDirection();
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

            if (length > 0) {
                direction.x /= length;
                direction.y /= length;
                direction.z /= length;
            }
            else {
                direction = { 1, 0, 0 };
            }

            return Ray(line.start, direction);
        }
    };

    // 三角形
    struct Triangle {
        Vector3 vertices[3];  // 頂点座標

        // デフォルトコンストラクタ
        Triangle() {
            vertices[0] = { 0, 0, 0 };
            vertices[1] = { 1, 0, 0 };
            vertices[2] = { 0, 1, 0 };
        }

        // パラメータ付きコンストラクタ
        Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2) {
            vertices[0] = v0;
            vertices[1] = v1;
            vertices[2] = v2;
        }

        // 法線を計算
        Vector3 ComputeNormal() const {
            // 辺ベクトル
            Vector3 edge1 = {
                vertices[1].x - vertices[0].x,
                vertices[1].y - vertices[0].y,
                vertices[1].z - vertices[0].z
            };

            Vector3 edge2 = {
                vertices[2].x - vertices[0].x,
                vertices[2].y - vertices[0].y,
                vertices[2].z - vertices[0].z
            };

            // 外積で法線を計算
            Vector3 normal = {
                edge1.y * edge2.z - edge1.z * edge2.y,
                edge1.z * edge2.x - edge1.x * edge2.z,
                edge1.x * edge2.y - edge1.y * edge2.x
            };

            // 正規化
            float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (length > 0) {
                normal.x /= length;
                normal.y /= length;
                normal.z /= length;
            }

            return normal;
        }
    };

    // 最接近点情報
    struct ClosestPointResult {
        Vector3 point;      // 最接近点の座標
        float distance;     // 最接近点までの距離
        float parameter;    // パラメータ（線分の場合は0〜1）
    };

} // namespace Collision