#pragma once
#include "Vector3.h"
#include "CollisionTypes.h"
#include <functional>

// 物理ボディーの設定
struct PhysicsProperties {
    float mass = 1.0f;               // 質量
    float restitution = 0.5f;        // 反発係数（0.0f〜1.0f）
    float friction = 0.3f;           // 摩擦係数
    bool isStatic = false;           // 静的オブジェクトかどうか
    bool useGravity = true;          // 重力の影響を受けるか
    Vector3 gravity = { 0.0f, -9.8f, 0.0f }; // 重力
};

// 物理応答用クラス（オブジェクトにアタッチする）
class PhysicsBody {
public:
    // コンストラクタ
    PhysicsBody();

    // デストラクタ
    ~PhysicsBody() = default;

    // 初期化
    void Initialize(class Object3d* parent);

    // 更新
    void Update(float deltaTime);

    // 衝突応答処理
    void ResolveCollision(const CollisionInfo& info, PhysicsBody* otherBody);

    // 力を加える
    void AddForce(const Vector3& force);

    // 速度をセット
    void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }

    // 速度を取得
    const Vector3& GetVelocity() const { return velocity_; }

    // 角速度をセット
    void SetAngularVelocity(const Vector3& angularVelocity) { angularVelocity_ = angularVelocity; }

    // 角速度を取得
    const Vector3& GetAngularVelocity() const { return angularVelocity_; }

    // 物理プロパティの設定
    void SetPhysicsProperties(const PhysicsProperties& properties) { properties_ = properties; }

    // 物理プロパティの取得
    const PhysicsProperties& GetPhysicsProperties() const { return properties_; }

    // 親オブジェクトの取得
    class Object3d* GetParent() const { return parent_; }

    // 衝突コールバックの設定
    void SetCollisionCallback(const CollisionCallback& callback) { onCollision_ = callback; }

    // 衝突処理
    void OnCollision(const CollisionInfo& info);

private:
    // 速度
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };

    // 角速度
    Vector3 angularVelocity_ = { 0.0f, 0.0f, 0.0f };

    // 加速度
    Vector3 acceleration_ = { 0.0f, 0.0f, 0.0f };

    // 力
    Vector3 force_ = { 0.0f, 0.0f, 0.0f };

    // 物理プロパティ
    PhysicsProperties properties_;

    // 親オブジェクト
    class Object3d* parent_ = nullptr;

    // 衝突コールバック
    CollisionCallback onCollision_;
};