#pragma once
#include "Vector3.h"
#include <cmath>

namespace Collision {
    // ベクトル演算ユーティリティ関数
    class Utility {
    public:
        // ベクトルの長さを計算
        static float Length(const Vector3& v) {
            return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        }

        // ベクトルの長さの2乗を計算
        static float LengthSquared(const Vector3& v) {
            return v.x * v.x + v.y * v.y + v.z * v.z;
        }

        // ベクトルを正規化
        static Vector3 Normalize(const Vector3& v) {
            float len = Length(v);
            if (len < 0.0001f) return v; // 長さがほぼ0の場合は元のベクトルを返す
            float invLen = 1.0f / len;
            return { v.x * invLen, v.y * invLen, v.z * invLen };
        }

        // ベクトルの加算
        static Vector3 Add(const Vector3& v1, const Vector3& v2) {
            return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
        }

        // ベクトルの減算
        static Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
            return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
        }

        // ベクトルのスカラー倍
        static Vector3 Multiply(const Vector3& v, float scalar) {
            return { v.x * scalar, v.y * scalar, v.z * scalar };
        }

        // ベクトルの内積
        static float Dot(const Vector3& v1, const Vector3& v2) {
            return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
        }

        // ベクトルの外積
        static Vector3 Cross(const Vector3& v1, const Vector3& v2) {
            return {
                v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x
            };
        }

        // 2点間の距離
        static float Distance(const Vector3& v1, const Vector3& v2) {
            return Length(Subtract(v2, v1));
        }

        // 2点間の距離の2乗
        static float DistanceSquared(const Vector3& v1, const Vector3& v2) {
            return LengthSquared(Subtract(v2, v1));
        }

        // 線分上の最近接点を求める
        static Vector3 ClosestPointOnSegment(const Vector3& point, const Vector3& segmentStart, const Vector3& segmentEnd) {
            Vector3 segment = Subtract(segmentEnd, segmentStart);
            float segmentLengthSq = LengthSquared(segment);

            // 線分の長さが0の場合は始点を返す
            if (segmentLengthSq < 0.0001f) {
                return segmentStart;
            }

            // 線分に対する投影比率を計算（内分比）
            Vector3 pointToStart = Subtract(point, segmentStart);
            float t = Dot(pointToStart, segment) / segmentLengthSq;

            // 始点より前なら始点を返す
            if (t < 0.0f) {
                return segmentStart;
            }
            // 終点より後なら終点を返す
            if (t > 1.0f) {
                return segmentEnd;
            }

            // 線分上の最近接点を計算
            return Add(segmentStart, Multiply(segment, t));
        }
    };
} // namespace Collision