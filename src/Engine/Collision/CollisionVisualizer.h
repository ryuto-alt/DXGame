#pragma once
#include "ColliderBase.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Model.h"
#include <memory>
#include <vector>

// 衝突判定の可視化クラス
class CollisionVisualizer {
private:
    // シングルトンインスタンス
    static CollisionVisualizer* instance_;

    // コンストラクタ（シングルトン）
    CollisionVisualizer() = default;
    // デストラクタ（シングルトン）
    ~CollisionVisualizer() = default;
    // コピー禁止
    CollisionVisualizer(const CollisionVisualizer&) = delete;
    CollisionVisualizer& operator=(const CollisionVisualizer&) = delete;

    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;
    // SpriteCommon
    SpriteCommon* spriteCommon_ = nullptr;

    // デバッグ表示用モデル
    std::unique_ptr<Model> sphereModel_; // 球体表示用
    std::unique_ptr<Model> boxModel_;    // ボックス表示用
    std::unique_ptr<Model> lineModel_;   // 線表示用

    // デバッグ表示のON/OFF
    bool isVisible_ = true;

    // 衝突点の記録
    struct CollisionPoint {
        Vector3 position;
        Vector3 normal;
        float lifetime;
    };
    std::vector<CollisionPoint> collisionPoints_;

public:
    // シングルトンインスタンスの取得
    static CollisionVisualizer* GetInstance();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    // 更新
    void Update();

    // 描画
    void Draw();

    // コライダーを描画
    void DrawCollider(ColliderBase* collider);

    // 表示/非表示の切り替え
    void SetVisible(bool visible) { isVisible_ = visible; }
    bool IsVisible() const { return isVisible_; }

    // 衝突点の記録
    void AddCollisionPoint(const Vector3& position, const Vector3& normal);

    // 衝突点の描画
    void DrawCollisionPoints();
};