#pragma once
#include "Vector3.h"
#include <functional>

// 衝突情報を格納する構造体
struct CollisionInfo {
    bool isColliding = false;    // 衝突しているかどうか
    Vector3 collisionPoint;      // 衝突点
    Vector3 normal;              // 衝突面の法線
    float penetration = 0.0f;    // めり込み量
};

// 衝突時のコールバック関数の型定義
using CollisionCallback = std::function<void(const CollisionInfo&)>;

// コライダーの種類
enum class ColliderType {
    None,
    Sphere,
    Box,
    // 将来的に追加するコライダータイプ
};