#include "CollisionDetection.h"
#include <algorithm>
#include <cassert>

// 静的メンバの初期化
BoundingBox CollisionDetection::stageBounds;
std::vector<BoundingBox> CollisionDetection::obstacles;

// ステージモデルからの境界抽出
void CollisionDetection::ExtractStageBoundaries(Model* stageModel) {
    // モデルが有効か確認
    assert(stageModel);

    // モデルから頂点データを取得
    const std::vector<VertexData>& vertices = stageModel->GetVertices();

    // 頂点が空の場合は処理しない
    if (vertices.empty()) {
        return;
    }

    // 最初の頂点で初期化
    Vector3 minPoint;
    Vector3 maxPoint;

    minPoint.x = vertices[0].position.x;
    minPoint.y = vertices[0].position.y;
    minPoint.z = vertices[0].position.z;

    maxPoint.x = vertices[0].position.x;
    maxPoint.y = vertices[0].position.y;
    maxPoint.z = vertices[0].position.z;

    // 全頂点から最小値と最大値を検索
    for (const auto& vertex : vertices) {
        // 最小値の更新
        if (vertex.position.x < minPoint.x) minPoint.x = vertex.position.x;
        if (vertex.position.y < minPoint.y) minPoint.y = vertex.position.y;
        if (vertex.position.z < minPoint.z) minPoint.z = vertex.position.z;

        // 最大値の更新
        if (vertex.position.x > maxPoint.x) maxPoint.x = vertex.position.x;
        if (vertex.position.y > maxPoint.y) maxPoint.y = vertex.position.y;
        if (vertex.position.z > maxPoint.z) maxPoint.z = vertex.position.z;
    }

    // ステージ境界を設定
    stageBounds.min = minPoint;
    stageBounds.max = maxPoint;

    // デフォルトのステージ境界を設定
    Vector3 stageMin, stageMax;
    stageMin.x = -100.0f;
    stageMin.y = -1.0f;
    stageMin.z = -100.0f;

    stageMax.x = 100.0f;
    stageMax.y = 100.0f;
    stageMax.z = 100.0f;

    stageBounds.min = stageMin;
    stageBounds.max = stageMax;

    // 既存の障害物をクリア
    ClearObstacles();

    // デバッグ出力
    OutputDebugStringA("CollisionDetection: ステージ境界を設定しました\n");
}

// OBJファイルからの境界抽出
void CollisionDetection::ExtractStageBoundaries(Model* stageModel, const std::string& objFilePath) {
    // ファイルを開く
    std::ifstream file(objFilePath);
    if (!file.is_open()) {
        OutputDebugStringA(("CollisionDetection: OBJファイルを開けませんでした: " + objFilePath + "\n").c_str());
        return;
    }

    // 障害物をクリア
    ClearObstacles();

    // OBJファイルからステージ境界とオブジェクトを抽出
    std::string line;
    uint32_t cornerNumber = 0;
    Vector3 max;
    Vector3 min;
    bool start = false;
    bool reverse = false;

    while (getline(file, line)) {
        std::istringstream line_stream(line);
        std::string word;
        getline(line_stream, word, ' ');

        if (word.find("vn") == 0) {
            break; // vn（法線ベクトル）を見つけたら終了
        }

        if (word.find("v") == 0) {
            cornerNumber++;
        }
        else {
            continue;
        }

        if (cornerNumber > 0) {
            getline(line_stream, word, ' ');
            float x = (float)std::atof(word.c_str());
            getline(line_stream, word, ' ');
            float y = (float)std::atof(word.c_str());
            getline(line_stream, word, ' ');
            float z = (float)std::atof(word.c_str());

            if (!start) {
                max = { x, y, z };
                min = { x, y, z };
                start = true;
            }
            else {
                // 前よりも大きいとき
                if (max.x <= x) max.x = x;
                // 前よりも小さいとき
                if (min.x > x) min.x = x;
                if (max.y <= y) max.y = y;
                if (min.y > y) min.y = y;
                if (max.z <= z) max.z = z;
                if (min.z > z) min.z = z;
            }
        }

        if (cornerNumber == 8) {
            if (!reverse) {
                // 基盤となるオブジェクトのAABBを追加
                AddObstacle({ min.x, min.y, min.z }, { max.x, max.y, max.z });
            }
            else {
                // 反転したオブジェクトのAABBを追加
                float minX = -(max.x);
                float maxX = -(min.x);
                min.x = minX;
                max.x = maxX;
                AddObstacle({ min.x, min.y, min.z }, { max.x, max.y, max.z });
            }
            cornerNumber = 0;
            start = false;
            reverse = true;
        }
    }

    file.close();

    // ステージ全体の境界の設定（すべての障害物を包含）
    if (!obstacles.empty()) {
        // 最初の障害物で初期化
        stageBounds = obstacles[0];

        // すべての障害物を包含するように拡張
        for (const auto& obstacle : obstacles) {
            // 最小値の更新
            if (obstacle.min.x < stageBounds.min.x) stageBounds.min.x = obstacle.min.x;
            if (obstacle.min.y < stageBounds.min.y) stageBounds.min.y = obstacle.min.y;
            if (obstacle.min.z < stageBounds.min.z) stageBounds.min.z = obstacle.min.z;

            // 最大値の更新
            if (obstacle.max.x > stageBounds.max.x) stageBounds.max.x = obstacle.max.x;
            if (obstacle.max.y > stageBounds.max.y) stageBounds.max.y = obstacle.max.y;
            if (obstacle.max.z > stageBounds.max.z) stageBounds.max.z = obstacle.max.z;
        }
    }
    else {
        // 障害物がない場合はデフォルト値を設定
        stageBounds.min = { -100.0f, -1.0f, -100.0f };
        stageBounds.max = { 100.0f, 100.0f, 100.0f };
    }

    OutputDebugStringA("CollisionDetection: ステージ境界とオブジェクトを抽出しました\n");
}

// 境界ボックスを新規追加するメソッド
void CollisionDetection::AddObstacle(const Vector3& min, const Vector3& max) {
    BoundingBox obstacle;
    obstacle.min = min;
    obstacle.max = max;
    obstacles.push_back(obstacle);
}

// 既存の AddObstacle メソッド
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

// 地面との衝突判定
bool CollisionDetection::CheckGroundCollision(const Vector3& playerPosition, float playerRadius, float playerHeight, Vector3& adjustedPosition) {
    // 調整後の位置を現在の位置で初期化
    adjustedPosition = playerPosition;

    // ステージ境界内かチェック
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

    // 地面との衝突判定
    // OBJ解析より、地面はY=0
    const float groundLevel = 0.0f;

    if (playerPosition.y < groundLevel) {
        adjustedPosition.y = groundLevel;
        return true; // 地面との衝突を検出
    }

    return false; // 地面との衝突なし
}

// 障害物との衝突判定
bool CollisionDetection::CheckObstacleCollision(const Vector3& playerPosition, float playerRadius, Vector3& adjustedPosition) {
    // 調整後の位置を現在の位置で初期化
    adjustedPosition = playerPosition;
    bool collisionDetected = false;

    // プレイヤーの円柱用の境界ボックスを作成（簡易的に箱として扱う）
    BoundingBox playerBox;

    // プレイヤーボックスの最小座標を設定
    playerBox.min.x = playerPosition.x - playerRadius;
    playerBox.min.y = playerPosition.y;
    playerBox.min.z = playerPosition.z - playerRadius;

    // プレイヤーボックスの最大座標を設定
    playerBox.max.x = playerPosition.x + playerRadius;
    playerBox.max.y = playerPosition.y + 2.0f; // プレイヤーの高さ
    playerBox.max.z = playerPosition.z + playerRadius;

    // 各障害物に対してチェック
    for (size_t i = 0; i < GetObstacleCount(); i++) {
        const BoundingBox& obstacle = GetObstacle(i);

        if (playerBox.Intersects(obstacle)) {
            collisionDetected = true;

            // 最適な押し出し方向を決定
            float overlapX1 = obstacle.max.x - playerBox.min.x;
            float overlapX2 = playerBox.max.x - obstacle.min.x;
            float overlapZ1 = obstacle.max.z - playerBox.min.z;
            float overlapZ2 = playerBox.max.z - obstacle.min.z;

            // 最小の重なりを探す
            float minOverlapX = (overlapX1 < overlapX2) ? overlapX1 : overlapX2;
            float minOverlapZ = (overlapZ1 < overlapZ2) ? overlapZ1 : overlapZ2;

            // 重なりが最小の軸を決定
            if (minOverlapX < minOverlapZ) {
                // X軸方向で解決
                if (overlapX1 < overlapX2) {
                    adjustedPosition.x = obstacle.max.x + playerRadius;
                }
                else {
                    adjustedPosition.x = obstacle.min.x - playerRadius;
                }
            }
            else {
                // Z軸方向で解決
                if (overlapZ1 < overlapZ2) {
                    adjustedPosition.z = obstacle.max.z + playerRadius;
                }
                else {
                    adjustedPosition.z = obstacle.min.z - playerRadius;
                }
            }

            // 一つの衝突を解決したらループを抜ける
            // より洗練されたシステムでは、全ての衝突が解決されるまで繰り返す
            break;
        }
    }

    return collisionDetected;
}