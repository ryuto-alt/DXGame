// src/Engine/Physics/CollisionObject.h
#pragma once

#include <reactphysics3d/reactphysics3d.h>
#include <string>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Mymath.h"

// 前方宣言
class CollisionManager;
enum class CollisionType;

// 衝突オブジェクトクラス
class CollisionObject {
public:
    // コンストラクタ/デストラクタ
    CollisionObject(const std::string& name, CollisionType type);
    ~CollisionObject();

    // 初期化
    void Initialize(CollisionManager* manager,
        const Vector3& position,
        const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
        const Vector3& scale = { 1.0f, 1.0f, 1.0f });

    // 更新
    void Update();

    // 終了処理
    void Finalize();

    // 位置設定
    void SetPosition(const Vector3& position);
    const Vector3& GetPosition() const { return position_; }

    // 回転設定
    void SetRotation(const Vector3& rotation);
    const Vector3& GetRotation() const { return rotation_; }

    // スケール設定
    void SetScale(const Vector3& scale);
    const Vector3& GetScale() const { return scale_; }

    // 衝突タイプ取得
    CollisionType GetCollisionType() const { return type_; }

    // 名前取得
    const std::string& GetName() const { return name_; }

    // 衝突しているかどうか設定/取得
    void SetColliding(bool isColliding) { isColliding_ = isColliding; }
    bool IsColliding() const { return isColliding_; }

    // ReactPhysics3D用
    reactphysics3d::CollisionBody* GetCollisionBody() const { return collisionBody_; }
    reactphysics3d::Transform GetTransform() const;

    // デバッグ描画
    void DebugDraw();

private:
    // 球形状作成
    void CreateSphereShape(float radius);

    // 箱形状作成
    void CreateBoxShape(const Vector3& halfExtents);

    // 平面形状作成
    void CreatePlaneShape(const Vector3& normal, float offset);

private:
    // オブジェクト情報
    std::string name_;
    CollisionType type_;
    bool isColliding_;

    // トランスフォーム
    Vector3 position_;
    Vector3 rotation_;
    Vector3 scale_;

    // ReactPhysics3D関連
    CollisionManager* manager_;
    reactphysics3d::CollisionBody* collisionBody_;
    reactphysics3d::CollisionShape* collisionShape_;

    // 形状のパラメータ
    union {
        float sphereRadius_;
        struct { Vector3 boxHalfExtents_; };
        struct { Vector3 planeNormal_; float planeOffset_; };
    };
};