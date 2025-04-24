#pragma once
#include "CollisionTypes.h"
#include <memory>
#include <string>

// 前方宣言
class Object3d;

class ColliderBase {
public:
    // コンストラクタ
    ColliderBase() = default;

    // 仮想デストラクタ
    virtual ~ColliderBase() = default;

    // 初期化
    virtual void Initialize() = 0;

    // 更新（オブジェクトの位置などに合わせて更新）
    virtual void Update() = 0;

    // 他のコライダーとの衝突判定
    virtual bool CheckCollision(ColliderBase* other, CollisionInfo& info) = 0;

    // コライダーの種類を取得
    virtual ColliderType GetType() const = 0;

    // 衝突時のコールバック設定
    void SetCollisionCallback(const CollisionCallback& callback) { onCollision_ = callback; }

    // 衝突時のコールバック実行
    void OnCollision(const CollisionInfo& info) {
        if (onCollision_) {
            onCollision_(info);
        }
    }

    // 親オブジェクトの設定
    void SetParentObject(Object3d* parent) { parentObject_ = parent; }

    // 親オブジェクトの取得
    Object3d* GetParentObject() const { return parentObject_; }

    // コライダー名の設定
    void SetName(const std::string& name) { name_ = name; }

    // コライダー名の取得
    const std::string& GetName() const { return name_; }

    // 有効/無効設定
    void SetEnabled(bool enabled) { isEnabled_ = enabled; }

    // 有効かどうか取得
    bool IsEnabled() const { return isEnabled_; }

protected:
    Object3d* parentObject_ = nullptr;  // 親オブジェクト
    std::string name_ = "Collider";     // コライダー名
    bool isEnabled_ = true;             // 有効/無効
    CollisionCallback onCollision_;     // 衝突時のコールバック
};