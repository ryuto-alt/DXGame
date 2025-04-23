#pragma once

#include"UnoEngine.h"

// Bounding box structure for collision detection
struct BoundingBox {
    Vector3 min; // Minimum coordinates (bottom-left-back corner)
    Vector3 max; // Maximum coordinates (top-right-front corner)

    // Check if point is inside this bounding box
    bool Contains(const Vector3& point) const {
        return (point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y &&
            point.z >= min.z && point.z <= max.z);
    }

    // Check if this bounding box intersects with another
    bool Intersects(const BoundingBox& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x &&
            min.y <= other.max.y && max.y >= other.min.y &&
            min.z <= other.max.z && max.z >= other.min.z);
    }
};

// Structure to hold collision information
struct CollisionInfo {
    bool isColliding = false;
    Vector3 collisionPoint = { 0.0f, 0.0f, 0.0f };
    Vector3 normal = { 0.0f, 1.0f, 0.0f }; // Default to upward normal
};

class CollisionDetection {
public:
    // Extract collision boundaries from the stage model
    static void ExtractStageBoundaries(Model* stageModel);

    // Detect collision between player and stage ground
    static bool CheckGroundCollision(const Vector3& playerPosition, float playerRadius, float playerHeight, Vector3& adjustedPosition);

    // Detect collision between player and stage obstacles
    static bool CheckObstacleCollision(const Vector3& playerPosition, float playerRadius, Vector3& adjustedPosition);

    // Get stage boundaries
    static const BoundingBox& GetStageBounds() { return stageBounds; }

    // Add an obstacle bounding box
    static void AddObstacle(const BoundingBox& obstacle);

    // Clear all obstacles
    static void ClearObstacles();

    // Get obstacle count
    static size_t GetObstacleCount();

    // Get obstacle at index
    static const BoundingBox& GetObstacle(size_t index);

private:
    // Overall stage boundaries (play area)
    static BoundingBox stageBounds;

    // List of obstacle bounding boxes
    static std::vector<BoundingBox> obstacles;
};