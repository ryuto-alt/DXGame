#pragma once
#include "CollisionPrimitive.h"
#include "CollisionUtility.h"

namespace Collision {
    // 衝突判定結果
    struct CollisionResult {
        bool isColliding;       // 衝突しているかどうか
        Vector3 collisionPoint; // 衝突点
        Vector3 normal;         // 衝突面の法線（衝突した場合のみ有効）
        float penetration;      // めり込み量（衝突した場合のみ有効）

        // コンストラクタ
        CollisionResult() : isColliding(false), collisionPoint({ 0,0,0 }), normal({ 0,0,0 }), penetration(0) {}
    };

    class CollisionDetector {
    public:
        // 球と球の衝突判定
        static CollisionResult CheckSphereToSphere(const Sphere& sphere1, const Sphere& sphere2);

        // 点と球の衝突判定
        static CollisionResult CheckPointToSphere(const Vector3& point, const Sphere& sphere);

        // 線分と球の衝突判定
        static CollisionResult CheckSegmentToSphere(const Segment& segment, const Sphere& sphere);

        // 球とカプセルの衝突判定
        static CollisionResult CheckSphereToCapusle(const Sphere& sphere, const Capsule& capsule);

        // カプセルとカプセルの衝突判定
        static CollisionResult CheckCapsuleToCapsule(const Capsule& capsule1, const Capsule& capsule2);

        // 移動する球と球の衝突判定（スウィープテスト）
        static CollisionResult CheckSphereSweepToSphere(
            const Sphere& movingSphere, const Vector3& velocity,
            const Sphere& staticSphere, float deltaTime);

        // 移動する球とカプセルの衝突判定（スウィープテスト）
        static CollisionResult CheckSphereSweepToCapsule(
            const Sphere& movingSphere, const Vector3& velocity,
            const Capsule& staticCapsule, float deltaTime);
    };
} // namespace Collision