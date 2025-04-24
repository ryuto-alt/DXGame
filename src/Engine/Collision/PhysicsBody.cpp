#include "PhysicsBody.h"
#include "Object3d.h"
#include <cmath>

PhysicsBody::PhysicsBody() {
    // 初期設定
    properties_.mass = 1.0f;
    properties_.restitution = 0.5f; // 反発係数（0〜1）
    properties_.friction = 0.3f;    // 摩擦係数
    properties_.isStatic = false;   // 動的オブジェクト
    properties_.useGravity = true;  // 重力を使う
    properties_.gravity = { 0.0f, -9.8f, 0.0f }; // 下向きの重力
}

void PhysicsBody::Initialize(Object3d* parent) {
    parent_ = parent;
}

void PhysicsBody::Update(float deltaTime) {
    // 静的オブジェクトは物理更新しない
    if (properties_.isStatic) {
        return;
    }

    // 親オブジェクトがなければ更新しない
    if (!parent_) {
        return;
    }

    // 重力を適用
    if (properties_.useGravity) {
        // F = mg
        Vector3 gravityForce = {
            properties_.gravity.x * properties_.mass,
            properties_.gravity.y * properties_.mass,
            properties_.gravity.z * properties_.mass
        };

        AddForce(gravityForce);
    }

    // 加速度を計算（F = ma → a = F/m）
    acceleration_.x = force_.x / properties_.mass;
    acceleration_.y = force_.y / properties_.mass;
    acceleration_.z = force_.z / properties_.mass;

    // 速度を更新（v = v0 + at）
    velocity_.x += acceleration_.x * deltaTime;
    velocity_.y += acceleration_.y * deltaTime;
    velocity_.z += acceleration_.z * deltaTime;

    // 位置を更新（p = p0 + vt）
    Vector3 position = parent_->GetPosition();
    position.x += velocity_.x * deltaTime;
    position.y += velocity_.y * deltaTime;
    position.z += velocity_.z * deltaTime;

    // 親オブジェクトの座標を更新
    parent_->SetPosition(position);

    // 回転を更新（簡易版 - オイラー角使用）
    Vector3 rotation = parent_->GetRotation();
    rotation.x += angularVelocity_.x * deltaTime;
    rotation.y += angularVelocity_.y * deltaTime;
    rotation.z += angularVelocity_.z * deltaTime;

    // 親オブジェクトの回転を更新
    parent_->SetRotation(rotation);

    // 力をリセット（1フレームのみ適用）
    force_ = { 0.0f, 0.0f, 0.0f };
}

void PhysicsBody::ResolveCollision(const CollisionInfo& info, PhysicsBody* otherBody) {
    // 静的オブジェクト同士の場合は処理しない
    if (properties_.isStatic && otherBody->properties_.isStatic) {
        return;
    }

    // 衝突応答

    // 反発係数（2つのオブジェクトの反発係数の平均）
    float restitution = (properties_.restitution + otherBody->properties_.restitution) * 0.5f;

    // 静的オブジェクトの場合は位置補正のみ
    if (properties_.isStatic) {
        // 静的（自分）vs 動的（相手）
        // 相手を押し出す
        Vector3 otherPos = otherBody->parent_->GetPosition();
        otherPos.x += info.normal.x * info.penetration;
        otherPos.y += info.normal.y * info.penetration;
        otherPos.z += info.normal.z * info.penetration;
        otherBody->parent_->SetPosition(otherPos);

        // 相手の速度を反射
        float dotProduct =
            otherBody->velocity_.x * info.normal.x +
            otherBody->velocity_.y * info.normal.y +
            otherBody->velocity_.z * info.normal.z;

        if (dotProduct < 0) {
            // めり込み方向の速度成分を反転
            Vector3 reflectVelocity = {
                otherBody->velocity_.x - (1.0f + restitution) * dotProduct * info.normal.x,
                otherBody->velocity_.y - (1.0f + restitution) * dotProduct * info.normal.y,
                otherBody->velocity_.z - (1.0f + restitution) * dotProduct * info.normal.z
            };

            otherBody->SetVelocity(reflectVelocity);
        }
    }
    else if (otherBody->properties_.isStatic) {
        // 動的（自分）vs 静的（相手）
        // 自分を押し出す
        Vector3 myPos = parent_->GetPosition();
        myPos.x -= info.normal.x * info.penetration;
        myPos.y -= info.normal.y * info.penetration;
        myPos.z -= info.normal.z * info.penetration;
        parent_->SetPosition(myPos);

        // 自分の速度を反射
        float dotProduct =
            velocity_.x * (-info.normal.x) +
            velocity_.y * (-info.normal.y) +
            velocity_.z * (-info.normal.z);

        if (dotProduct < 0) {
            // めり込み方向の速度成分を反転
            Vector3 reflectVelocity = {
                velocity_.x - (1.0f + restitution) * dotProduct * (-info.normal.x),
                velocity_.y - (1.0f + restitution) * dotProduct * (-info.normal.y),
                velocity_.z - (1.0f + restitution) * dotProduct * (-info.normal.z)
            };

            SetVelocity(reflectVelocity);
        }
    }
    else {
        // 動的 vs 動的の場合の処理
        // 質量比率に基づいて位置補正
        float totalMass = properties_.mass + otherBody->properties_.mass;
        float ratio1 = otherBody->properties_.mass / totalMass;
        float ratio2 = properties_.mass / totalMass;

        // 押し出し
        Vector3 myPos = parent_->GetPosition();
        Vector3 otherPos = otherBody->parent_->GetPosition();

        myPos.x -= info.normal.x * info.penetration * ratio1;
        myPos.y -= info.normal.y * info.penetration * ratio1;
        myPos.z -= info.normal.z * info.penetration * ratio1;

        otherPos.x += info.normal.x * info.penetration * ratio2;
        otherPos.y += info.normal.y * info.penetration * ratio2;
        otherPos.z += info.normal.z * info.penetration * ratio2;

        parent_->SetPosition(myPos);
        otherBody->parent_->SetPosition(otherPos);

        // 運動量保存の法則に基づく速度変化
        Vector3 relativeVelocity = {
            velocity_.x - otherBody->velocity_.x,
            velocity_.y - otherBody->velocity_.y,
            velocity_.z - otherBody->velocity_.z
        };

        float dotProduct =
            relativeVelocity.x * info.normal.x +
            relativeVelocity.y * info.normal.y +
            relativeVelocity.z * info.normal.z;

        // 離れる方向に動いている場合は無視
        if (dotProduct >= 0) {
            return;
        }

        // 衝撃力の計算
        float j = -(1.0f + restitution) * dotProduct;
        j /= (1.0f / properties_.mass) + (1.0f / otherBody->properties_.mass);

        Vector3 impulse = {
            j * info.normal.x,
            j * info.normal.y,
            j * info.normal.z
        };

        // 速度の更新
        Vector3 myNewVelocity = {
            velocity_.x + impulse.x / properties_.mass,
            velocity_.y + impulse.y / properties_.mass,
            velocity_.z + impulse.z / properties_.mass
        };

        Vector3 otherNewVelocity = {
            otherBody->velocity_.x - impulse.x / otherBody->properties_.mass,
            otherBody->velocity_.y - impulse.y / otherBody->properties_.mass,
            otherBody->velocity_.z - impulse.z / otherBody->properties_.mass
        };

        SetVelocity(myNewVelocity);
        otherBody->SetVelocity(otherNewVelocity);

        // 摩擦力の適用（接線方向の速度を減衰）
        // 省略 - 必要に応じて実装
    }
}

void PhysicsBody::AddForce(const Vector3& force) {
    // 力を加算
    force_.x += force.x;
    force_.y += force.y;
    force_.z += force.z;
}

void PhysicsBody::OnCollision(const CollisionInfo& info) {
    // コールバックが設定されていれば呼び出す
    if (onCollision_) {
        onCollision_(info);
    }
}