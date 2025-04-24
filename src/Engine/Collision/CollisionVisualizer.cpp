#include "CollisionVisualizer.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Object3d.h"
#include <algorithm>
#include <CollisionManager.h>

// 静的メンバ変数の実体化
CollisionVisualizer* CollisionVisualizer::instance_ = nullptr;

CollisionVisualizer* CollisionVisualizer::GetInstance() {
    if (!instance_) {
        instance_ = new CollisionVisualizer();
    }
    return instance_;
}

void CollisionVisualizer::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;

    // デバッグ表示用モデルの読み込み
    sphereModel_ = std::make_unique<Model>();
    sphereModel_->Initialize(dxCommon_);
    sphereModel_->LoadFromObj("Resources/models", "sphere.obj");

    boxModel_ = std::make_unique<Model>();
    boxModel_->Initialize(dxCommon_);
    boxModel_->LoadFromObj("Resources/models", "cube.obj");
}

void CollisionVisualizer::Update() {
    // 衝突点の寿命を減らす
    for (auto it = collisionPoints_.begin(); it != collisionPoints_.end();) {
        it->lifetime -= 1.0f / 60.0f; // 60FPS想定
        if (it->lifetime <= 0.0f) {
            it = collisionPoints_.erase(it);
        }
        else {
            ++it;
        }
    }
}

void CollisionVisualizer::Draw() {
    // 表示設定がOFFなら描画しない
    if (!isVisible_) {
        return;
    }

    // CollisionManagerからすべてのコライダーを取得して描画
    auto* collisionManager = CollisionManager::GetInstance();
    const auto& colliders = collisionManager->GetColliders();

    for (auto* collider : colliders) {
        if (collider && collider->IsEnabled()) {
            DrawCollider(collider);
        }
    }

    // 衝突点を描画
    DrawCollisionPoints();
}

void CollisionVisualizer::DrawCollider(ColliderBase* collider) {
    if (!collider) return;

    // コライダーの種類によって描画を分岐
    switch (collider->GetType()) {
    case ColliderType::Sphere: {
        // 球体コライダーの描画
        SphereCollider* sphereCollider = static_cast<SphereCollider*>(collider);

        // 仮のObject3dを作成して描画
        Object3d sphereObj;
        sphereObj.Initialize(dxCommon_, spriteCommon_);
        sphereObj.SetModel(sphereModel_.get());

        // 座標とスケールを設定
        const Vector3& center = sphereCollider->GetCenter();
        float radius = sphereCollider->GetRadius();

        sphereObj.SetPosition(center);
        sphereObj.SetScale({ radius, radius, radius });

        // ワイヤーフレーム表示のために半透明に
        sphereObj.SetColor({ 0.0f, 1.0f, 0.0f, 0.3f }); // 緑色半透明
        sphereObj.SetEnableLighting(false); // ライティングを無効化

        // 更新と描画
        sphereObj.Update();
        sphereObj.Draw();
        break;
    }

    case ColliderType::Box: {
        // ボックスコライダーの描画
        BoxCollider* boxCollider = static_cast<BoxCollider*>(collider);

        // 仮のObject3dを作成して描画
        Object3d boxObj;
        boxObj.Initialize(dxCommon_, spriteCommon_);
        boxObj.SetModel(boxModel_.get());

        // 座標とスケールを設定
        const Vector3& center = boxCollider->GetCenter();
        const Vector3& size = boxCollider->GetSize();

        boxObj.SetPosition(center);
        boxObj.SetScale(size);

        // ワイヤーフレーム表示のために半透明に
        boxObj.SetColor({ 0.0f, 0.0f, 1.0f, 0.3f }); // 青色半透明
        boxObj.SetEnableLighting(false); // ライティングを無効化

        // 更新と描画
        boxObj.Update();
        boxObj.Draw();
        break;
    }

    default:
        break;
    }
}

void CollisionVisualizer::AddCollisionPoint(const Vector3& position, const Vector3& normal) {
    // 衝突点の記録
    CollisionPoint point;
    point.position = position;
    point.normal = normal;
    point.lifetime = 2.0f; // 2秒間表示

    collisionPoints_.push_back(point);
}

void CollisionVisualizer::DrawCollisionPoints() {
    // 衝突点の描画
    for (const auto& point : collisionPoints_) {
        // 仮のObject3dを作成して描画（衝突点を小さな球体で表現）
        Object3d pointObj;
        pointObj.Initialize(dxCommon_, spriteCommon_);
        pointObj.SetModel(sphereModel_.get());

        // 座標とスケール
        pointObj.SetPosition(point.position);
        pointObj.SetScale({ 0.1f, 0.1f, 0.1f }); // 小さな球

        // 色（赤色）
        pointObj.SetColor({ 1.0f, 0.0f, 0.0f, 0.8f });
        pointObj.SetEnableLighting(false);

        // 更新と描画
        pointObj.Update();
        pointObj.Draw();

        // 法線の表示（線分で表示）
        // 法線表示はやや複雑なので、今回は省略
    }
}