#pragma once
#include "ColliderBase.h"

class BoxCollider : public ColliderBase {
public:
    // コンストラクタ
    BoxCollider() = default;

    // デストラクタ
    ~BoxCollider() override = default;

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 他のコライダーとの衝突判定
    bool CheckCollision(ColliderBase* other, CollisionInfo& info) override;

    // コライダーの種類を取得
    ColliderType GetType() const override { return ColliderType::Box; }

    // サイズの設定（半分のサイズ = ボックスの中心から各辺までの距離）
    void SetSize(const Vector3& size) { size_ = size; }

    // サイズの取得
    const Vector3& GetSize() const { return size_; }

    // オフセットの設定
    void SetOffset(const Vector3& offset) { offset_ = offset; }

    // オフセットの取得
    const Vector3& GetOffset() const { return offset_; }

    // 最小座標の取得
    const Vector3& GetMin() const { return min_; }

    // 最大座標の取得
    const Vector3& GetMax() const { return max_; }

    // 中心座標の取得
    const Vector3& GetCenter() const { return center_; }

private:
    Vector3 center_ = { 0.0f, 0.0f, 0.0f };  // 中心座標
    Vector3 offset_ = { 0.0f, 0.0f, 0.0f };  // 親オブジェクトからのオフセット
    Vector3 size_ = { 1.0f, 1.0f, 1.0f };    // サイズ（半分のサイズ）
    Vector3 min_ = { -1.0f, -1.0f, -1.0f };  // 最小座標
    Vector3 max_ = { 1.0f, 1.0f, 1.0f };     // 最大座標

    // ボックス対ボックスの衝突判定
    bool CheckBoxToBox(BoxCollider* other, CollisionInfo& info);

    // ボックス対球の衝突判定
    bool CheckBoxToSphere(class SphereCollider* sphere, CollisionInfo& info);
};