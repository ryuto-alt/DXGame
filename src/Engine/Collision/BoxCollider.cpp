#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Object3d.h"
#include <algorithm>

void BoxCollider::Initialize() {
    // 初期化時の処理
}

void BoxCollider::Update() {
    // 親オブジェクトが存在する場合、その位置に合わせて中心座標を更新
    if (parentObject_) {
        // 親オブジェクトの座標を取得
        const Vector3& parentPos = parentObject_->GetPosition();

        // オフセットを考慮して中心座標を計算
        center_.x = parentPos.x + offset_.x;
        center_.y = parentPos.y + offset_.y;
        center_.z = parentPos.z + offset_.z;

        // 最小・最大座標の更新
        min_.x = center_.x - size_.x;
        min_.y = center_.y - size_.y;
        min_.z = center_.z - size_.z;

        max_.x = center_.x + size_.x;
        max_.y = center_.y + size_.y;
        max_.z = center_.z + size_.z;
    }
}

bool BoxCollider::CheckCollision(ColliderBase* other, CollisionInfo& info) {
    // 両方のコライダーが有効でない場合は衝突判定しない
    if (!IsEnabled() || !other->IsEnabled()) {
        return false;
    }

    // 相手のコライダータイプによって処理を分ける
    switch (other->GetType()) {
    case ColliderType::Sphere:
        return CheckBoxToSphere(static_cast<SphereCollider*>(other), info);

    case ColliderType::Box:
        return CheckBoxToBox(static_cast<BoxCollider*>(other), info);

    default:
        return false;
    }
}

bool BoxCollider::CheckBoxToBox(BoxCollider* other, CollisionInfo& info) {
    // AABB同士の衝突判定
    const Vector3& min1 = this->GetMin();
    const Vector3& max1 = this->GetMax();
    const Vector3& min2 = other->GetMin();
    const Vector3& max2 = other->GetMax();

    // 各軸で重なりがあるか確認
    bool overlapX = (min1.x <= max2.x && max1.x >= min2.x);
    bool overlapY = (min1.y <= max2.y && max1.y >= min2.y);
    bool overlapZ = (min1.z <= max2.z && max1.z >= min2.z);

    // 全ての軸で重なりがある場合に衝突
    if (overlapX && overlapY && overlapZ) {
        // 衝突情報を設定
        info.isColliding = true;

        // めり込み量が最小の軸を特定
        float overlapDistX = std::min(max1.x - min2.x, max2.x - min1.x);
        float overlapDistY = std::min(max1.y - min2.y, max2.y - min1.y);
        float overlapDistZ = std::min(max1.z - min2.z, max2.z - min1.z);

        // 最もめり込みが小さい軸を使用して法線と衝突点を計算
        if (overlapDistX <= overlapDistY && overlapDistX <= overlapDistZ) {
            // X軸方向の衝突
            info.penetration = overlapDistX;
            if (this->GetCenter().x < other->GetCenter().x) {
                info.normal = { -1.0f, 0.0f, 0.0f }; // 負のX方向
                info.collisionPoint.x = max1.x;
            }
            else {
                info.normal = { 1.0f, 0.0f, 0.0f };  // 正のX方向
                info.collisionPoint.x = min1.x;
            }
            // Y, Z座標は中間点を使用
            info.collisionPoint.y = (std::max(min1.y, min2.y) + std::min(max1.y, max2.y)) * 0.5f;
            info.collisionPoint.z = (std::max(min1.z, min2.z) + std::min(max1.z, max2.z)) * 0.5f;
        }
        else if (overlapDistY <= overlapDistX && overlapDistY <= overlapDistZ) {
            // Y軸方向の衝突
            info.penetration = overlapDistY;
            if (this->GetCenter().y < other->GetCenter().y) {
                info.normal = { 0.0f, -1.0f, 0.0f }; // 負のY方向
                info.collisionPoint.y = max1.y;
            }
            else {
                info.normal = { 0.0f, 1.0f, 0.0f };  // 正のY方向
                info.collisionPoint.y = min1.y;
            }
            // X, Z座標は中間点を使用
            info.collisionPoint.x = (std::max(min1.x, min2.x) + std::min(max1.x, max2.x)) * 0.5f;
            info.collisionPoint.z = (std::max(min1.z, min2.z) + std::min(max1.z, max2.z)) * 0.5f;
        }
        else {
            // Z軸方向の衝突
            info.penetration = overlapDistZ;
            if (this->GetCenter().z < other->GetCenter().z) {
                info.normal = { 0.0f, 0.0f, -1.0f }; // 負のZ方向
                info.collisionPoint.z = max1.z;
            }
            else {
                info.normal = { 0.0f, 0.0f, 1.0f };  // 正のZ方向
                info.collisionPoint.z = min1.z;
            }
            // X, Y座標は中間点を使用
            info.collisionPoint.x = (std::max(min1.x, min2.x) + std::min(max1.x, max2.x)) * 0.5f;
            info.collisionPoint.y = (std::max(min1.y, min2.y) + std::min(max1.y, max2.y)) * 0.5f;
        }

        return true;
    }

    // 衝突していない
    info.isColliding = false;
    return false;
}

bool BoxCollider::CheckBoxToSphere(SphereCollider* sphere, CollisionInfo& info) {
    // 球対ボックスの衝突判定をそのまま流用（結果は同じになる）
    bool isColliding = sphere->CheckCollision(this, info);

    // 法線の向きを反転（ボックスから見た法線に変更）
    if (isColliding) {
        info.normal.x = -info.normal.x;
        info.normal.y = -info.normal.y;
        info.normal.z = -info.normal.z;
    }

    return isColliding;
}