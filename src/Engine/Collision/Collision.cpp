#include "Collision.h"
#include <algorithm>

namespace Collision {

    // 球と球の衝突判定
    CollisionResult CollisionDetector::CheckSphereToSphere(const Sphere& sphere1, const Sphere& sphere2) {
        CollisionResult result;

        // 球の中心間の距離を計算
        Vector3 direction = Utility::Subtract(sphere2.center, sphere1.center);
        float distanceSquared = Utility::LengthSquared(direction);
        float radiusSum = sphere1.radius + sphere2.radius;

        // 衝突判定
        if (distanceSquared <= radiusSum * radiusSum) {
            // 衝突している場合
            result.isColliding = true;

            // 衝突点などの詳細情報を計算
            float distance = std::sqrt(distanceSquared);

            // 方向ベクトルを正規化
            if (distance > 0.0001f) {
                result.normal = Utility::Multiply(direction, 1.0f / distance);
            }
            else {
                // 中心が重なっている場合はY軸上方向をデフォルトとする
                result.normal = { 0.0f, 1.0f, 0.0f };
            }

            // めり込み量
            result.penetration = radiusSum - distance;

            // 衝突点（球1の表面上の点）
            result.collisionPoint = Utility::Add(
                sphere1.center,
                Utility::Multiply(result.normal, sphere1.radius)
            );
        }

        return result;
    }

    // 点と球の衝突判定
    CollisionResult CollisionDetector::CheckPointToSphere(const Vector3& point, const Sphere& sphere) {
        CollisionResult result;

        // 点と球の中心間の距離を計算
        Vector3 direction = Utility::Subtract(point, sphere.center);
        float distanceSquared = Utility::LengthSquared(direction);

        // 衝突判定
        if (distanceSquared <= sphere.radius * sphere.radius) {
            // 衝突している場合
            result.isColliding = true;

            // 衝突点などの詳細情報を計算
            float distance = std::sqrt(distanceSquared);

            // 方向ベクトルを正規化
            if (distance > 0.0001f) {
                result.normal = Utility::Multiply(direction, 1.0f / distance);
            }
            else {
                // 中心と点が重なっている場合はY軸上方向をデフォルトとする
                result.normal = { 0.0f, 1.0f, 0.0f };
            }

            // めり込み量
            result.penetration = sphere.radius - distance;

            // 衝突点（点そのもの）
            result.collisionPoint = point;
        }

        return result;
    }

    // 線分と球の衝突判定
    CollisionResult CollisionDetector::CheckSegmentToSphere(const Segment& segment, const Sphere& sphere) {
        CollisionResult result;

        // 線分上の最近接点を計算
        Vector3 closestPoint = Utility::ClosestPointOnSegment(
            sphere.center, segment.start, segment.end
        );

        // 最近接点と球の中心との距離を計算
        Vector3 direction = Utility::Subtract(closestPoint, sphere.center);
        float distanceSquared = Utility::LengthSquared(direction);

        // 衝突判定
        if (distanceSquared <= sphere.radius * sphere.radius) {
            // 衝突している場合
            result.isColliding = true;

            // 衝突点などの詳細情報を計算
            float distance = std::sqrt(distanceSquared);

            // 方向ベクトルを正規化
            if (distance > 0.0001f) {
                result.normal = Utility::Multiply(direction, 1.0f / distance);
            }
            else {
                // 中心と最近接点が重なっている場合
                // 線分の方向に垂直な方向を求める
                Vector3 segmentDir = Utility::Subtract(segment.end, segment.start);
                float segmentLength = Utility::Length(segmentDir);

                if (segmentLength > 0.0001f) {
                    segmentDir = Utility::Multiply(segmentDir, 1.0f / segmentLength);
                    // 適当な軸との外積で垂直ベクトルを作る
                    Vector3 tempAxis = std::abs(segmentDir.y) < 0.9f ? Vector3{ 0, 1, 0 } : Vector3{ 1, 0, 0 };
                    result.normal = Utility::Normalize(Utility::Cross(segmentDir, tempAxis));
                }
                else {
                    // 線分が点状の場合はY軸上方向をデフォルトとする
                    result.normal = { 0.0f, 1.0f, 0.0f };
                }
            }

            // めり込み量
            result.penetration = sphere.radius - distance;

            // 衝突点（線分上の最近接点）
            result.collisionPoint = closestPoint;
        }

        return result;
    }

    // 球とカプセルの衝突判定
    CollisionResult CollisionDetector::CheckSphereToCapusle(const Sphere& sphere, const Capsule& capsule) {
        CollisionResult result;

        // カプセルの中心線分上の最近接点を計算
        Vector3 closestPoint = Utility::ClosestPointOnSegment(
            sphere.center, capsule.segment.start, capsule.segment.end
        );

        // 最近接点から球に向かうベクトル
        Vector3 direction = Utility::Subtract(sphere.center, closestPoint);
        float distanceSquared = Utility::LengthSquared(direction);
        float radiusSum = sphere.radius + capsule.radius;

        // 衝突判定
        if (distanceSquared <= radiusSum * radiusSum) {
            // 衝突している場合
            result.isColliding = true;

            // 衝突点などの詳細情報を計算
            float distance = std::sqrt(distanceSquared);

            // 方向ベクトルを正規化
            if (distance > 0.0001f) {
                result.normal = Utility::Multiply(direction, 1.0f / distance);
            }
            else {
                // カプセルの中心線分と球の中心が重なっている場合
                // カプセルの方向に垂直な方向を求める
                Vector3 capsuleDir = Utility::Subtract(capsule.segment.end, capsule.segment.start);
                float capsuleLength = Utility::Length(capsuleDir);

                if (capsuleLength > 0.0001f) {
                    capsuleDir = Utility::Multiply(capsuleDir, 1.0f / capsuleLength);
                    // 適当な軸との外積で垂直ベクトルを作る
                    Vector3 tempAxis = std::abs(capsuleDir.y) < 0.9f ? Vector3{ 0, 1, 0 } : Vector3{ 1, 0, 0 };
                    result.normal = Utility::Normalize(Utility::Cross(capsuleDir, tempAxis));
                }
                else {
                    // カプセルが点状の場合はY軸上方向をデフォルトとする
                    result.normal = { 0.0f, 1.0f, 0.0f };
                }
            }

            // めり込み量
            result.penetration = radiusSum - distance;

            // 衝突点（カプセルの表面上の点）
            if (distance > 0.0001f) {
                result.collisionPoint = Utility::Add(
                    closestPoint,
                    Utility::Multiply(result.normal, capsule.radius)
                );
            }
            else {
                // 距離が0の場合
                result.collisionPoint = closestPoint;
            }
        }

        return result;
    }

    // カプセルとカプセルの衝突判定
    CollisionResult CollisionDetector::CheckCapsuleToCapsule(const Capsule& capsule1, const Capsule& capsule2) {
        CollisionResult result;

        // 2つの線分間の最近接点を計算するためのパラメータ
        Vector3 d1 = Utility::Subtract(capsule1.segment.end, capsule1.segment.start);
        Vector3 d2 = Utility::Subtract(capsule2.segment.end, capsule2.segment.start);
        Vector3 r = Utility::Subtract(capsule1.segment.start, capsule2.segment.start);

        float a = Utility::Dot(d1, d1);
        float b = Utility::Dot(d1, d2);
        float c = Utility::Dot(d2, d2);
        float d = Utility::Dot(d1, r);
        float e = Utility::Dot(d2, r);

        float f = a * c - b * b;

        // 平行でない場合
        float s = 0.0f;
        float t = 0.0f;

        if (f > 0.0001f) {
            // 最近接点のパラメータを計算
            s = std::clamp((b * e - c * d) / f, 0.0f, 1.0f);
            t = std::clamp((a * e - b * d) / f, 0.0f, 1.0f);
        }
        else {
            // 線分がほぼ平行の場合、片方の始点と相手の線分との距離を比較して決める
            s = std::clamp(d / a, 0.0f, 1.0f);
            t = 0.0f;
        }

        // カプセル1上の最近接点
        Vector3 p1 = Utility::Add(
            capsule1.segment.start,
            Utility::Multiply(d1, s)
        );

        // カプセル2上の最近接点
        Vector3 p2 = Utility::Add(
            capsule2.segment.start,
            Utility::Multiply(d2, t)
        );

        // 最近接点間の距離を計算
        Vector3 direction = Utility::Subtract(p2, p1);
        float distanceSquared = Utility::LengthSquared(direction);
        float radiusSum = capsule1.radius + capsule2.radius;

        // 衝突判定
        if (distanceSquared <= radiusSum * radiusSum) {
            // 衝突している場合
            result.isColliding = true;

            // 衝突点などの詳細情報を計算
            float distance = std::sqrt(distanceSquared);

            // 方向ベクトルを正規化
            if (distance > 0.0001f) {
                result.normal = Utility::Multiply(direction, 1.0f / distance);
            }
            else {
                // 最近接点が重なっている場合
                // 2つのカプセルの方向の外積を使って法線を求める
                Vector3 normal = Utility::Cross(d1, d2);
                float normalLength = Utility::Length(normal);

                if (normalLength > 0.0001f) {
                    result.normal = Utility::Multiply(normal, 1.0f / normalLength);
                }
                else {
                    // 2つのカプセルがほぼ平行の場合はY軸上方向をデフォルトとする
                    result.normal = { 0.0f, 1.0f, 0.0f };
                }
            }

            // めり込み量
            result.penetration = radiusSum - distance;

            // 衝突点（2つの最近接点の中間）
            result.collisionPoint = Utility::Add(
                p1,
                Utility::Multiply(direction, 0.5f)
            );
        }

        return result;
    }

    // 移動する球と静止した球の衝突判定（スウィープテスト）
    CollisionResult CollisionDetector::CheckSphereSweepToSphere(
        const Sphere& movingSphere, const Vector3& velocity,
        const Sphere& staticSphere, float deltaTime) {

        CollisionResult result;

        // 移動前の衝突判定
        CollisionResult initialCheck = CheckSphereToSphere(movingSphere, staticSphere);
        if (initialCheck.isColliding) {
            // 既に衝突している場合は初期状態の判定結果を返す
            return initialCheck;
        }

        // 相対位置
        Vector3 relativePos = Utility::Subtract(movingSphere.center, staticSphere.center);
        float radiusSum = movingSphere.radius + staticSphere.radius;

        // 放物線と球の方程式を立てて解く
        // a, b, c係数を計算
        float a = Utility::LengthSquared(velocity);
        if (a < 0.0001f) {
            // 速度がほぼ0の場合は衝突しない
            return result;
        }

        float b = 2.0f * Utility::Dot(velocity, relativePos);
        float c = Utility::LengthSquared(relativePos) - radiusSum * radiusSum;

        // 判別式
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f) {
            // 解なし（衝突しない）
            return result;
        }

        // 衝突時間を計算
        float t = (-b - std::sqrt(discriminant)) / (2.0f * a);

        if (t < 0.0f || t > deltaTime) {
            // 過去または未来のdeltaTime以降での衝突（衝突なし）
            return result;
        }

        // 衝突位置を計算
        Vector3 collisionCenter = Utility::Add(
            movingSphere.center,
            Utility::Multiply(velocity, t)
        );

        // 衝突時の球の中心から静止球の中心への方向
        Vector3 direction = Utility::Subtract(collisionCenter, staticSphere.center);
        float distance = Utility::Length(direction);

        // 衝突結果を設定
        result.isColliding = true;

        if (distance > 0.0001f) {
            result.normal = Utility::Multiply(direction, 1.0f / distance);
        }
        else {
            // 中心が重なる場合（起こりにくい）
            result.normal = Utility::Normalize(velocity);
            if (Utility::Length(result.normal) < 0.0001f) {
                result.normal = { 0.0f, 1.0f, 0.0f };
            }
        }

        // めり込み量は0（表面で接触した時点）
        result.penetration = 0.0f;

        // 衝突点（移動球の表面上の点）
        result.collisionPoint = Utility::Add(
            collisionCenter,
            Utility::Multiply(result.normal, -movingSphere.radius)
        );

        return result;
    }

    // 移動する球と静止したカプセルの衝突判定（スウィープテスト）
    CollisionResult CollisionDetector::CheckSphereSweepToCapsule(
        const Sphere& movingSphere, const Vector3& velocity,
        const Capsule& staticCapsule, float deltaTime) {

        CollisionResult result;

        // 移動前の衝突判定
        CollisionResult initialCheck = CheckSphereToCapusle(movingSphere, staticCapsule);
        if (initialCheck.isColliding) {
            // 既に衝突している場合は初期状態の判定結果を返す
            return initialCheck;
        }

        // カプセルを太さ0の線分と見なして最近接点を計算
        Vector3 closestPoint = Utility::ClosestPointOnSegment(
            movingSphere.center,
            staticCapsule.segment.start,
            staticCapsule.segment.end
        );

        // 最近接点を中心とする球と移動球のスウィープテスト
        Sphere tempSphere;
        tempSphere.center = closestPoint;
        tempSphere.radius = staticCapsule.radius;

        return CheckSphereSweepToSphere(movingSphere, velocity, tempSphere, deltaTime);
    }

} // namespace Collision