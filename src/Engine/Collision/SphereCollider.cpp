#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Object3d.h"
#include <cmath>

void SphereCollider::Initialize() {
    // 初期化時の処理
}

void SphereCollider::Update() {
    // 親オブジェクトが存在する場合、その位置に合わせて中心座標を更新
    if (parentObject_) {
        // 親オブジェクトの座標を取得
        const Vector3& parentPos = parentObject_->GetPosition();

        // オフセットを考慮して中心座標を計算
        center_.x = parentPos.x + offset_.x;
        center_.y = parentPos.y + offset_.y;
        center_.z = parentPos.z + offset_.z;
    }
}

bool SphereCollider::CheckCollision(ColliderBase* other, CollisionInfo& info) {
    // 両方のコライダーが有効でない場合は衝突判定しない
    if (!IsEnabled() || !other->IsEnabled()) {
        return false;
    }

    // 相手のコライダータイプによって処理を分ける
    switch (other->GetType()) {
    case ColliderType::Sphere:
        return CheckSphereToSphere(static_cast<SphereCollider*>(other), info);

    case ColliderType::Box:
        return CheckSphereToBox(static_cast<BoxCollider*>(other), info);

    default:
        return false;
    }
}

bool SphereCollider::CheckSphereToSphere(SphereCollider* other, CollisionInfo& info) {
    // 2つの球体の中心間の距離を計算
    const Vector3& center1 = this->GetCenter();
    const Vector3& center2 = other->GetCenter();

    float distX = center2.x - center1.x;
    float distY = center2.y - center1.y;
    float distZ = center2.z - center1.z;

    // 2点間の距離の二乗
    float distSquared = distX * distX + distY * distY + distZ * distZ;

    // 2つの半径の和
    float radiusSum = this->GetRadius() + other->GetRadius();

    // 衝突判定（距離 < 半径の和）
    if (distSquared < radiusSum * radiusSum) {
        // 衝突情報を設定
        info.isColliding = true;

        // 衝突点は2つの球体の中心を結ぶ線上
        float distance = std::sqrt(distSquared);

        // 距離が0の場合（同じ位置）は特別処理
        if (distance < 0.0001f) {
            info.normal = { 0.0f, 1.0f, 0.0f }; // 上向きの法線
            info.collisionPoint = center1;       // 中心を衝突点とする
            info.penetration = radiusSum;        // めり込み量は半径の和
        }
        else {
            // 衝突法線（正規化された方向ベクトル）
            info.normal.x = distX / distance;
            info.normal.y = distY / distance;
            info.normal.z = distZ / distance;

            // 衝突点（半径に沿った点）
            info.collisionPoint.x = center1.x + info.normal.x * this->GetRadius();
            info.collisionPoint.y = center1.y + info.normal.y * this->GetRadius();
            info.collisionPoint.z = center1.z + info.normal.z * this->GetRadius();

            // めり込み量
            info.penetration = radiusSum - distance;
        }

        return true;
    }

    // 衝突していない
    info.isColliding = false;
    return false;
}

bool SphereCollider::CheckSphereToBox(BoxCollider* box, CollisionInfo& info) {
    // BoxColliderの実装後に記述
    // ボックスの最近接点を求める
    const Vector3& boxMin = box->GetMin();
    const Vector3& boxMax = box->GetMax();
    const Vector3& sphereCenter = this->GetCenter();

    // 球の中心座標を、ボックスの範囲内に制限（最近接点を見つける）
    Vector3 closestPoint;

    // X軸方向の最近接点
    closestPoint.x = (sphereCenter.x < boxMin.x) ? boxMin.x :
        (sphereCenter.x > boxMax.x) ? boxMax.x : sphereCenter.x;

    // Y軸方向の最近接点
    closestPoint.y = (sphereCenter.y < boxMin.y) ? boxMin.y :
        (sphereCenter.y > boxMax.y) ? boxMax.y : sphereCenter.y;

    // Z軸方向の最近接点
    closestPoint.z = (sphereCenter.z < boxMin.z) ? boxMin.z :
        (sphereCenter.z > boxMax.z) ? boxMax.z : sphereCenter.z;

    // 球の中心と最近接点の距離の二乗
    float distX = closestPoint.x - sphereCenter.x;
    float distY = closestPoint.y - sphereCenter.y;
    float distZ = closestPoint.z - sphereCenter.z;
    float distSquared = distX * distX + distY * distY + distZ * distZ;

    // 半径の二乗
    float radiusSquared = this->GetRadius() * this->GetRadius();

    // 衝突判定（距離 < 半径）
    if (distSquared < radiusSquared) {
        // 衝突情報を設定
        info.isColliding = true;

        // 最近接点が球の中心と同じ場合（球がボックス内部）
        if (distSquared < 0.0001f) {
            // 仮の法線（+Y方向）
            info.normal = { 0.0f, 1.0f, 0.0f };
            info.collisionPoint = sphereCenter;
            info.penetration = this->GetRadius();
        }
        else {
            // 衝突法線（正規化された方向ベクトル）
            float distance = std::sqrt(distSquared);
            info.normal.x = distX / distance;
            info.normal.y = distY / distance;
            info.normal.z = distZ / distance;

            // 衝突点
            info.collisionPoint = closestPoint;

            // めり込み量
            info.penetration = this->GetRadius() - distance;
        }

        return true;
    }

    // 衝突していない
    info.isColliding = false;
    return false;
}