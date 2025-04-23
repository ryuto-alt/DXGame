#pragma once

#include "UnoEngine.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// 衝突判定用の境界ボックス構造体
struct BoundingBox {
    Vector3 min; // 最小座標（左下奥の角）
    Vector3 max; // 最大座標（右上前の角）

    // 点が境界ボックス内にあるかチェック
    bool Contains(const Vector3& point) const {
        return (point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y &&
            point.z >= min.z && point.z <= max.z);
    }

    // この境界ボックスが別の境界ボックスと交差するかチェック
    bool Intersects(const BoundingBox& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x &&
            min.y <= other.max.y && max.y >= other.min.y &&
            min.z <= other.max.z && max.z >= other.min.z);
    }
};

// 衝突情報を保持する構造体
struct CollisionInfo {
    bool isColliding = false;
    Vector3 collisionPoint = { 0.0f, 0.0f, 0.0f };
    Vector3 normal = { 0.0f, 1.0f, 0.0f }; // デフォルトは上向き法線
};

class CollisionDetection {
public:
    // ステージモデルから衝突境界を抽出
    static void ExtractStageBoundaries(Model* stageModel);

    // OBJファイルから直接衝突境界を抽出
    static void ExtractStageBoundaries(Model* stageModel, const std::string& objFilePath);

    // プレイヤーとステージ地面の衝突判定
    static bool CheckGroundCollision(const Vector3& playerPosition, float playerRadius, float playerHeight, Vector3& adjustedPosition);

    // プレイヤーとステージ障害物の衝突判定
    static bool CheckObstacleCollision(const Vector3& playerPosition, float playerRadius, Vector3& adjustedPosition);

    // ステージ境界の取得
    static const BoundingBox& GetStageBounds() { return stageBounds; }

    // 障害物境界ボックスの追加
    static void AddObstacle(const BoundingBox& obstacle);

    // 座標から障害物境界ボックスを作成して追加
    static void AddObstacle(const Vector3& min, const Vector3& max);

    // 全障害物のクリア
    static void ClearObstacles();

    // 障害物数の取得
    static size_t GetObstacleCount();

    // インデックスから障害物の取得
    static const BoundingBox& GetObstacle(size_t index);

private:
    // ステージ全体の境界（プレイエリア）
    static BoundingBox stageBounds;

    // 障害物境界ボックスのリスト
    static std::vector<BoundingBox> obstacles;
};