#pragma once
#include "ColliderBase.h"

class SphereCollider : public ColliderBase {
public:
    // コンストラクタ
    SphereCollider() = default;

    // デストラクタ
    ~SphereCollider() override = default;

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 他のコライダーとの衝突判定
    bool CheckCollision(ColliderBase* other, CollisionInfo& info) override;

    // コライダーの種類を取得
    ColliderType GetType() const override { return ColliderType::Sphere; }

    // 中心座標の取得
    const Vector3& GetCenter() const { return center_; }

    // 半径の設定
    void SetRadius(float radius) { radius_ = radius; }

    // 半径の取得
    float GetRadius() const { return radius_; }

    // オフセットの設定
    void SetOffset(const Vector3& offset) { offset_ = offset; }

    // オフセットの取得
    const Vector3& GetOffset() const { return offset_; }

private:
    Vector3 center_ = { 0.0f, 0.0f, 0.0f };  // 中心座標
    Vector3 offset_ = { 0.0f, 0.0f, 0.0f };  // 親オブジェクトからのオフセット
    float radius_ = 1.0f;                  // 半径

    // 球対球の衝突判定
    bool CheckSphereToSphere(SphereCollider* other, CollisionInfo& info);

    // 球対ボックスの衝突判定
    bool CheckSphereToBox(class BoxCollider* box, CollisionInfo& info);
};