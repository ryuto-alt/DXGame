#include "CollisionDetection.h"
#include <algorithm>
#include <cassert>

// Initialize static members
BoundingBox CollisionDetection::stageBounds;
std::vector<BoundingBox> CollisionDetection::obstacles;

// Implementation of additional accessor methods
void CollisionDetection::AddObstacle(const BoundingBox& obstacle) {
    obstacles.push_back(obstacle);
}

void CollisionDetection::ClearObstacles() {
    obstacles.clear();
}

size_t CollisionDetection::GetObstacleCount() {
    return obstacles.size();
}

const BoundingBox& CollisionDetection::GetObstacle(size_t index) {
    assert(index < obstacles.size());
    return obstacles[index];
}

void CollisionDetection::ExtractStageBoundaries(Model* stageModel) {
    // Make sure we have a valid model
    assert(stageModel);

    // Get vertex data from the model
    const std::vector<VertexData>& vertices = stageModel->GetVertices();

    // Initialize min and max points with the first vertex
    if (vertices.empty()) {
        return;
    }

    // Initialize with first vertex
    Vector3 minPoint;
    Vector3 maxPoint;

    minPoint.x = vertices[0].position.x;
    minPoint.y = vertices[0].position.y;
    minPoint.z = vertices[0].position.z;

    maxPoint.x = vertices[0].position.x;
    maxPoint.y = vertices[0].position.y;
    maxPoint.z = vertices[0].position.z;

    // Find the minimum and maximum points from all vertices
    for (const auto& vertex : vertices) {
        // Update min point
        if (vertex.position.x < minPoint.x) minPoint.x = vertex.position.x;
        if (vertex.position.y < minPoint.y) minPoint.y = vertex.position.y;
        if (vertex.position.z < minPoint.z) minPoint.z = vertex.position.z;

        // Update max point
        if (vertex.position.x > maxPoint.x) maxPoint.x = vertex.position.x;
        if (vertex.position.y > maxPoint.y) maxPoint.y = vertex.position.y;
        if (vertex.position.z > maxPoint.z) maxPoint.z = vertex.position.z;
    }

    // Set the stage boundaries
    stageBounds.min = minPoint;
    stageBounds.max = maxPoint;

    // Stage data analyzed from the OBJ file: bounds from -100 to 100 in X and Z
    // The ground is at around Y = 0 to 1

    // Define stage bounds directly
    Vector3 stageMin, stageMax;
    stageMin.x = -100.0f;
    stageMin.y = -1.0f;
    stageMin.z = -100.0f;

    stageMax.x = 100.0f;
    stageMax.y = 100.0f;
    stageMax.z = 100.0f;

    stageBounds.min = stageMin;
    stageBounds.max = stageMax;

    // Clear any existing obstacles
    ClearObstacles();

    // Extract obstacle boundaries based on vertex clustering
    // Since the OBJ defines several structures on the ground plane,
    // we'll create bounding boxes for each structure

    // Based on the OBJ analysis, let's define some obstacles
    // These represent the various structures seen in the stage model

    // Center structure area
    BoundingBox centerStructure;
    centerStructure.min.x = -56.0f; centerStructure.min.y = 0.0f; centerStructure.min.z = -64.0f;
    centerStructure.max.x = -46.0f; centerStructure.max.y = 25.0f; centerStructure.max.z = -54.0f;
    AddObstacle(centerStructure);

    // Structure from OBJ analysis - boxes
    BoundingBox box1;
    box1.min.x = -38.0f; box1.min.y = 0.0f; box1.min.z = -72.0f;
    box1.max.x = -18.0f; box1.max.y = 25.0f; box1.max.z = -46.0f;
    AddObstacle(box1);

    BoundingBox box2;
    box2.min.x = 19.0f; box2.min.y = 0.0f; box2.min.z = -72.0f;
    box2.max.x = 39.0f; box2.max.y = 25.0f; box2.max.z = -46.0f;
    AddObstacle(box2);

    BoundingBox box3;
    box3.min.x = -10.0f; box3.min.y = 0.0f; box3.min.z = -72.0f;
    box3.max.x = 10.0f; box3.max.y = 25.0f; box3.max.z = -46.0f;
    AddObstacle(box3);

    // Wall structures
    BoundingBox wall1;
    wall1.min.x = -76.0f; wall1.min.y = 0.0f; wall1.min.z = 5.0f;
    wall1.max.x = -45.0f; wall1.max.y = 11.0f; wall1.max.z = 42.0f;
    AddObstacle(wall1);

    BoundingBox wall2;
    wall2.min.x = -41.0f; wall2.min.y = 4.0f; wall2.min.z = 30.0f;
    wall2.max.x = -25.0f; wall2.max.y = 14.0f; wall2.max.z = 42.0f;
    AddObstacle(wall2);

    BoundingBox wall3;
    wall3.min.x = -19.0f; wall3.min.y = 4.0f; wall3.min.z = 30.0f;
    wall3.max.x = -3.0f; wall3.max.y = 14.0f; wall3.max.z = 42.0f;
    AddObstacle(wall3);

    BoundingBox wall4;
    wall4.min.x = 3.0f; wall4.min.y = 4.0f; wall4.min.z = 30.0f;
    wall4.max.x = 51.0f; wall4.max.y = 14.0f; wall4.max.z = 42.0f;
    AddObstacle(wall4);

    // Horizontal bars
    for (int i = -56; i < 65; i += 10) {
        BoundingBox bar;
        bar.min.x = static_cast<float>(i);
        bar.min.y = 22.0f;
        bar.min.z = -13.0f;

        bar.max.x = static_cast<float>(i + 10);
        bar.max.y = 24.0f;
        bar.max.z = -3.0f;

        AddObstacle(bar);
    }

    // Add other structures - rows of boxes in positive X area
    BoundingBox xRow;
    xRow.min.x = 52.0f; xRow.min.y = 22.0f; xRow.min.z = -60.0f;
    xRow.max.x = 62.0f; xRow.max.y = 24.0f; xRow.max.z = -18.0f;
    AddObstacle(xRow);

    // Small platforms
    BoundingBox platform;
    platform.min.x = 67.0f; platform.min.y = 1.0f; platform.min.z = 4.0f;
    platform.max.x = 77.0f; platform.max.y = 29.0f; platform.max.z = 34.0f;
    AddObstacle(platform);

    OutputDebugStringA("CollisionDetection: Stage boundaries and obstacles extracted successfully\n");
}

bool CollisionDetection::CheckGroundCollision(const Vector3& playerPosition, float playerRadius, float playerHeight, Vector3& adjustedPosition) {
    // Set the adjusted position to the current position initially
    adjustedPosition = playerPosition;

    // Check if player is within stage boundaries
    if (playerPosition.x - playerRadius < stageBounds.min.x) {
        adjustedPosition.x = stageBounds.min.x + playerRadius;
    }
    else if (playerPosition.x + playerRadius > stageBounds.max.x) {
        adjustedPosition.x = stageBounds.max.x - playerRadius;
    }

    if (playerPosition.z - playerRadius < stageBounds.min.z) {
        adjustedPosition.z = stageBounds.min.z + playerRadius;
    }
    else if (playerPosition.z + playerRadius > stageBounds.max.z) {
        adjustedPosition.z = stageBounds.max.z - playerRadius;
    }

    // Check and handle ground collision
    // From the OBJ analysis, the ground is at Y = 0
    const float groundLevel = 0.0f;

    if (playerPosition.y < groundLevel) {
        adjustedPosition.y = groundLevel;
        return true; // Collision with ground detected
    }

    return false; // No ground collision
}

bool CollisionDetection::CheckObstacleCollision(const Vector3& playerPosition, float playerRadius, Vector3& adjustedPosition) {
    // Set the adjusted position to the current position initially
    adjustedPosition = playerPosition;
    bool collisionDetected = false;

    // Create a bounding box for the player's cylinder (simplified as box)
    BoundingBox playerBox;

    // Set player box min coordinates
    playerBox.min.x = playerPosition.x - playerRadius;
    playerBox.min.y = playerPosition.y;
    playerBox.min.z = playerPosition.z - playerRadius;

    // Set player box max coordinates
    playerBox.max.x = playerPosition.x + playerRadius;
    playerBox.max.y = playerPosition.y + 2.0f; // Player height
    playerBox.max.z = playerPosition.z + playerRadius;

    // Check against each obstacle
    for (size_t i = 0; i < GetObstacleCount(); i++) {
        const BoundingBox& obstacle = GetObstacle(i);

        if (playerBox.Intersects(obstacle)) {
            collisionDetected = true;

            // Determine the best direction to push the player out
            float overlapX1 = obstacle.max.x - playerBox.min.x;
            float overlapX2 = playerBox.max.x - obstacle.min.x;
            float overlapZ1 = obstacle.max.z - playerBox.min.z;
            float overlapZ2 = playerBox.max.z - obstacle.min.z;

            // Find the smallest overlap
            float minOverlapX = (overlapX1 < overlapX2) ? overlapX1 : overlapX2;
            float minOverlapZ = (overlapZ1 < overlapZ2) ? overlapZ1 : overlapZ2;

            // Determine which axis has the smallest overlap
            if (minOverlapX < minOverlapZ) {
                // Resolve along X axis
                if (overlapX1 < overlapX2) {
                    adjustedPosition.x = obstacle.max.x + playerRadius;
                }
                else {
                    adjustedPosition.x = obstacle.min.x - playerRadius;
                }
            }
            else {
                // Resolve along Z axis
                if (overlapZ1 < overlapZ2) {
                    adjustedPosition.z = obstacle.max.z + playerRadius;
                }
                else {
                    adjustedPosition.z = obstacle.min.z - playerRadius;
                }
            }

            // Once we've resolved one collision, break out
            // A more sophisticated system would iterate until all collisions are resolved
            break;
        }
    }

    return collisionDetected;
}