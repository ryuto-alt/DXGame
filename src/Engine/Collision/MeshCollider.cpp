// src/Engine/Collision/MeshCollider.cpp
#include "MeshCollider.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace Collision {

//-----------------------------------------------------------------------------
// TriangleMeshCollider実装
//-----------------------------------------------------------------------------

TriangleMeshCollider::TriangleMeshCollider(const std::vector<Triangle>& triangles) {
    SetTriangles(triangles);
}

TriangleMeshCollider::TriangleMeshCollider(const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices) {
    // 頂点データとインデックスデータから三角形を構築
    triangles_.reserve(indices.size() / 3);
    
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 < indices.size()) {
            Triangle triangle;
            triangle.vertices[0] = vertices[indices[i]];
            triangle.vertices[1] = vertices[indices[i + 1]];
            triangle.vertices[2] = vertices[indices[i + 2]];
            
            triangles_.push_back(triangle);
        }
    }
    
    // 境界ボリュームの計算
    RecalculateBoundingVolume();
}

void TriangleMeshCollider::ApplyTransform(const Matrix4x4& worldMatrix) {
    // 各三角形に変換を適用
    for (auto& triangle : triangles_) {
        for (int i = 0; i < 3; i++) {
            Vector4 vertex = { 
                triangle.vertices[i].x, 
                triangle.vertices[i].y, 
                triangle.vertices[i].z, 
                1.0f 
            };
            
            Vector4 transformed;
            transformed.x = 
                vertex.x * worldMatrix.m[0][0] + 
                vertex.y * worldMatrix.m[1][0] + 
                vertex.z * worldMatrix.m[2][0] + 
                vertex.w * worldMatrix.m[3][0];
            
            transformed.y = 
                vertex.x * worldMatrix.m[0][1] + 
                vertex.y * worldMatrix.m[1][1] + 
                vertex.z * worldMatrix.m[2][1] + 
                vertex.w * worldMatrix.m[3][1];
            
            transformed.z = 
                vertex.x * worldMatrix.m[0][2] + 
                vertex.y * worldMatrix.m[1][2] + 
                vertex.z * worldMatrix.m[2][2] + 
                vertex.w * worldMatrix.m[3][2];
            
            transformed.w = 
                vertex.x * worldMatrix.m[0][3] + 
                vertex.y * worldMatrix.m[1][3] + 
                vertex.z * worldMatrix.m[2][3] + 
                vertex.w * worldMatrix.m[3][3];
            
            if (transformed.w != 0.0f) {
                transformed.x /= transformed.w;
                transformed.y /= transformed.w;
                transformed.z /= transformed.w;
            }
            
            triangle.vertices[i] = { transformed.x, transformed.y, transformed.z };
        }
    }
    
    // 境界ボリュームも変換
    if (boundingVolume) {
        boundingVolume->ApplyTransform(worldMatrix);
    }
}

bool TriangleMeshCollider::Intersects(const Vector3& point, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    if (boundingVolume && !boundingVolume->Contains(point)) {
        return false;
    }
    
    // 各三角形との衝突判定
    for (const auto& triangle : triangles_) {
        // 三角形の法線を計算
        Vector3 normal = triangle.ComputeNormal();
        
        // 三角形の平面を作成
        Plane plane = Plane::CreateFromPointNormal(triangle.vertices[0], normal);
        
        // 点と平面の距離
        float distance = 
            normal.x * point.x + 
            normal.y * point.y + 
            normal.z * point.z + 
            plane.distance;
        
        // 平面上に点があるか確認（わずかな浮動小数点誤差を許容）
        if (std::abs(distance) > 0.0001f) {
            continue;
        }
        
        // 三角形内に点があるか確認（重心座標を使用）
        Vector3 edge1 = {
            triangle.vertices[1].x - triangle.vertices[0].x,
            triangle.vertices[1].y - triangle.vertices[0].y,
            triangle.vertices[1].z - triangle.vertices[0].z
        };
        
        Vector3 edge2 = {
            triangle.vertices[2].x - triangle.vertices[0].x,
            triangle.vertices[2].y - triangle.vertices[0].y,
            triangle.vertices[2].z - triangle.vertices[0].z
        };
        
        Vector3 vp = {
            point.x - triangle.vertices[0].x,
            point.y - triangle.vertices[0].y,
            point.z - triangle.vertices[0].z
        };
        
        // 内積を計算
        float d00 = 
            edge1.x * edge1.x + 
            edge1.y * edge1.y + 
            edge1.z * edge1.z;
        
        float d01 = 
            edge1.x * edge2.x + 
            edge1.y * edge2.y + 
            edge1.z * edge2.z;
        
        float d11 = 
            edge2.x * edge2.x + 
            edge2.y * edge2.y + 
            edge2.z * edge2.z;
        
        float d20 = 
            vp.x * edge1.x + 
            vp.y * edge1.y + 
            vp.z * edge1.z;
        
        float d21 = 
            vp.x * edge2.x + 
            vp.y * edge2.y + 
            vp.z * edge2.z;
        
        float denom = d00 * d11 - d01 * d01;
        
        // 重心座標を計算
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        
        // 三角形内であれば、すべての重心座標は0〜1の範囲内
        if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f && w >= 0.0f && w <= 1.0f) {
            if (result) {
                result->hasCollision = true;
                result->collisionPoint = point;
                result->normal = normal;
                result->penetration = std::abs(distance);
            }
            return true;
        }
    }
    
    return false;
}

bool TriangleMeshCollider::Intersects(const Line& line, CollisionResult* result) const {
    // レイを作成
    Ray ray = Ray::CreateFromLine(line);
    
    // レイの長さを計算
    float maxDistance = line.GetLength();
    
    // レイとの衝突判定を実行
    bool hit = Intersects(ray, maxDistance, result);
    
    return hit;
}

bool TriangleMeshCollider::Intersects(const Ray& ray, float maxDistance, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    // 現在はシンプル化のため省略
    
    bool hasHit = false;
    float closestT = maxDistance;
    CollisionResult tempResult;
    
    // 各三角形との交差判定
    for (const auto& triangle : triangles_) {
        // レイと三角形の交差判定
        if (RaycastTriangle(ray, triangle, maxDistance, &tempResult)) {
            // 最も近い交差点を記録
            float t = 
                (tempResult.collisionPoint.x - ray.origin.x) * ray.direction.x +
                (tempResult.collisionPoint.y - ray.origin.y) * ray.direction.y +
                (tempResult.collisionPoint.z - ray.origin.z) * ray.direction.z;
            
            if (t < closestT) {
                hasHit = true;
                closestT = t;
                
                if (result) {
                    *result = tempResult;
                }
            }
        }
    }
    
    return hasHit;
}

bool TriangleMeshCollider::Intersects(const Sphere& sphere, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    if (boundingVolume && !boundingVolume->Contains(sphere.center)) {
        // 球の半径も考慮する必要がある
        // 簡易的な判定のためここでは省略
    }
    
    bool hasCollision = false;
    float minPenetration = std::numeric_limits<float>::max();
    CollisionResult tempResult;
    
    // 各三角形との衝突判定
    for (const auto& triangle : triangles_) {
        // 三角形の法線を計算
        Vector3 normal = triangle.ComputeNormal();
        
        // 三角形の平面を作成
        Plane plane = Plane::CreateFromPointNormal(triangle.vertices[0], normal);
        
        // 球の中心と平面の距離
        float distance = 
            normal.x * sphere.center.x + 
            normal.y * sphere.center.y + 
            normal.z * sphere.center.z + 
            plane.distance;
        
        // 距離が半径より大きければ衝突していない
        if (std::abs(distance) > sphere.radius) {
            continue;
        }
        
        // 衝突点を計算（球の中心から平面に下ろした垂線の足）
        Vector3 pointOnPlane = {
            sphere.center.x - normal.x * distance,
            sphere.center.y - normal.y * distance,
            sphere.center.z - normal.z * distance
        };
        
        // 点が三角形内にあるか確認
        Vector3 edge1 = {
            triangle.vertices[1].x - triangle.vertices[0].x,
            triangle.vertices[1].y - triangle.vertices[0].y,
            triangle.vertices[1].z - triangle.vertices[0].z
        };
        
        Vector3 edge2 = {
            triangle.vertices[2].x - triangle.vertices[0].x,
            triangle.vertices[2].y - triangle.vertices[0].y,
            triangle.vertices[2].z - triangle.vertices[0].z
        };
        
        Vector3 vp = {
            pointOnPlane.x - triangle.vertices[0].x,
            pointOnPlane.y - triangle.vertices[0].y,
            pointOnPlane.z - triangle.vertices[0].z
        };
        
        // 内積を計算
        float d00 = edge1.x * edge1.x + edge1.y * edge1.y + edge1.z * edge1.z;
        float d01 = edge1.x * edge2.x + edge1.y * edge2.y + edge1.z * edge2.z;
        float d11 = edge2.x * edge2.x + edge2.y * edge2.y + edge2.z * edge2.z;
        float d20 = vp.x * edge1.x + vp.y * edge1.y + vp.z * edge1.z;
        float d21 = vp.x * edge2.x + vp.y * edge2.y + vp.z * edge2.z;
        
        float denom = d00 * d11 - d01 * d01;
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
        
        // 三角形内の場合の処理
        if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f && w >= 0.0f && w <= 1.0f) {
            hasCollision = true;
            float penetration = sphere.radius - std::abs(distance);
            
            if (penetration < minPenetration) {
                minPenetration = penetration;
                
                if (result) {
                    result->hasCollision = true;
                    result->collisionPoint = pointOnPlane;
                    result->normal = normal;
                    result->penetration = penetration;
                }
            }
        }
        else {
            // 三角形の頂点または辺に最も近い点を見つける
            
            // TODO: 辺や頂点との最近接点を計算して追加
            // 簡略化のため省略
        }
    }
    
    return hasCollision;
}

bool TriangleMeshCollider::Intersects(const AABB& aabb, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    // 現在はシンプル化のため省略
    
    // TODO: AABBとの詳細な衝突判定
    // 簡略化のため省略
    
    return false;
}

bool TriangleMeshCollider::Intersects(const OBB& obb, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    // 現在はシンプル化のため省略
    
    // TODO: OBBとの詳細な衝突判定
    // 簡略化のため省略
    
    return false;
}

bool TriangleMeshCollider::Intersects(const IMeshCollider& other, CollisionResult* result) const {
    // 両方の境界ボリュームで高速判定
    // 現在はシンプル化のため省略
    
    // TODO: メッシュコライダー同士の詳細な衝突判定
    // 簡略化のため省略
    
    return false;
}

void TriangleMeshCollider::AddTriangle(const Triangle& triangle) {
    triangles_.push_back(triangle);
    
    // 境界ボリュームを更新
    RecalculateBoundingVolume();
}

void TriangleMeshCollider::SetTriangles(const std::vector<Triangle>& triangles) {
    triangles_ = triangles;
    
    // 境界ボリュームを計算
    RecalculateBoundingVolume();
}

void TriangleMeshCollider::RecalculateBoundingVolume() {
    if (triangles_.empty()) {
        // デフォルトのバウンディングボックスを作成
        boundingVolume = std::make_shared<BoundingAABB>(
            Vector3{-1.0f, -1.0f, -1.0f},
            Vector3{1.0f, 1.0f, 1.0f}
        );
        return;
    }
    
    // すべての頂点を集める
    std::vector<Vector3> vertices;
    vertices.reserve(triangles_.size() * 3);
    
    for (const auto& triangle : triangles_) {
        vertices.push_back(triangle.vertices[0]);
        vertices.push_back(triangle.vertices[1]);
        vertices.push_back(triangle.vertices[2]);
    }
    
    // 境界ボリュームを作成（AABBを使用）
    boundingVolume = std::make_shared<BoundingAABB>(vertices);
}

bool TriangleMeshCollider::RaycastTriangle(const Ray& ray, const Triangle& triangle, float maxDistance, CollisionResult* result) const {
    // 三角形の法線を計算
    Vector3 normal = triangle.ComputeNormal();
    
    // レイの方向と法線の内積
    float NdotD = 
        normal.x * ray.direction.x + 
        normal.y * ray.direction.y + 
        normal.z * ray.direction.z;
    
    // レイと三角形が平行なら交差しない
    if (std::abs(NdotD) < 0.0001f) {
        return false;
    }
    
    // 三角形の平面を作成
    Plane plane = Plane::CreateFromPointNormal(triangle.vertices[0], normal);
    
    // レイの始点から平面までの距離
    float t = -(
        normal.x * ray.origin.x + 
        normal.y * ray.origin.y + 
        normal.z * ray.origin.z + 
        plane.distance
    ) / NdotD;
    
    // 平面が遠すぎる、または後ろにある場合は交差しない
    if (t < 0 || t > maxDistance) {
        return false;
    }
    
    // レイと平面の交点
    Vector3 intersection = {
        ray.origin.x + ray.direction.x * t,
        ray.origin.y + ray.direction.y * t,
        ray.origin.z + ray.direction.z * t
    };
    
    // 交点が三角形内にあるか確認（重心座標を使用）
    Vector3 edge1 = {
        triangle.vertices[1].x - triangle.vertices[0].x,
        triangle.vertices[1].y - triangle.vertices[0].y,
        triangle.vertices[1].z - triangle.vertices[0].z
    };
    
    Vector3 edge2 = {
        triangle.vertices[2].x - triangle.vertices[0].x,
        triangle.vertices[2].y - triangle.vertices[0].y,
        triangle.vertices[2].z - triangle.vertices[0].z
    };
    
    Vector3 vp = {
        intersection.x - triangle.vertices[0].x,
        intersection.y - triangle.vertices[0].y,
        intersection.z - triangle.vertices[0].z
    };
    
    // 内積を計算
    float d00 = edge1.x * edge1.x + edge1.y * edge1.y + edge1.z * edge1.z;
    float d01 = edge1.x * edge2.x + edge1.y * edge2.y + edge1.z * edge2.z;
    float d11 = edge2.x * edge2.x + edge2.y * edge2.y + edge2.z * edge2.z;
    float d20 = vp.x * edge1.x + vp.y * edge1.y + vp.z * edge1.z;
    float d21 = vp.x * edge2.x + vp.y * edge2.y + vp.z * edge2.z;
    
    float denom = d00 * d11 - d01 * d01;
    
    // 重心座標を計算
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    
    // 三角形内であれば、すべての重心座標は0〜1の範囲内
    // 数値誤差を考慮して少し余裕を持たせる
    const float EPSILON = 0.0001f;
    if (u >= -EPSILON && u <= 1.0f + EPSILON && 
        v >= -EPSILON && v <= 1.0f + EPSILON && 
        w >= -EPSILON && w <= 1.0f + EPSILON) {
        
        if (result) {
            result->hasCollision = true;
            result->collisionPoint = intersection;
            
            // 法線の向きを調整（レイの方向と反対向きにする）
            if (NdotD > 0) {
                result->normal = {
                    -normal.x,
                    -normal.y,
                    -normal.z
                };
            } else {
                result->normal = normal;
            }
            
            result->penetration = t;
        }
        
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
// ConvexMeshCollider実装
//-----------------------------------------------------------------------------

ConvexMeshCollider::ConvexMeshCollider(const std::vector<Vector3>& vertices) {
    SetVertices(vertices);
}

void ConvexMeshCollider::ApplyTransform(const Matrix4x4& worldMatrix) {
    // 頂点に変換を適用
    for (auto& vertex : vertices_) {
        Vector4 v = { vertex.x, vertex.y, vertex.z, 1.0f };
        Vector4 transformed;
        
        transformed.x = 
            v.x * worldMatrix.m[0][0] + 
            v.y * worldMatrix.m[1][0] + 
            v.z * worldMatrix.m[2][0] + 
            v.w * worldMatrix.m[3][0];
        
        transformed.y = 
            v.x * worldMatrix.m[0][1] + 
            v.y * worldMatrix.m[1][1] + 
            v.z * worldMatrix.m[2][1] + 
            v.w * worldMatrix.m[3][1];
        
        transformed.z = 
            v.x * worldMatrix.m[0][2] + 
            v.y * worldMatrix.m[1][2] + 
            v.z * worldMatrix.m[2][2] + 
            v.w * worldMatrix.m[3][2];
        
        transformed.w = 
            v.x * worldMatrix.m[0][3] + 
            v.y * worldMatrix.m[1][3] + 
            v.z * worldMatrix.m[2][3] + 
            v.w * worldMatrix.m[3][3];
        
        if (transformed.w != 0.0f) {
            transformed.x /= transformed.w;
            transformed.y /= transformed.w;
            transformed.z /= transformed.w;
        }
        
        vertex = { transformed.x, transformed.y, transformed.z };
    }
    
    // 凸包を再計算
    CalculateConvexHull();
    
    // 境界ボリュームも更新
    RecalculateBoundingVolume();
}

bool ConvexMeshCollider::Intersects(const Vector3& point, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    if (boundingVolume && !boundingVolume->Contains(point)) {
        return false;
    }
    
    // 凸多面体内に点があるか確認
    // 各面に対して点がどの側にあるかをチェック
    
    // 簡易的な実装：TriangleMeshColliderの実装を流用
    for (const auto& triangle : triangles_) {
        Vector3 normal = triangle.ComputeNormal();
        Plane plane = Plane::CreateFromPointNormal(triangle.vertices[0], normal);
        
        float distance = 
            normal.x * point.x + 
            normal.y * point.y + 
            normal.z * point.z + 
            plane.distance;
        
        // 点が面の裏側にある場合、凸多面体の外側
        if (distance > 0.0001f) {
            return false;
        }
    }
    
    // すべての面の内側にある場合、凸多面体の内側
    if (result) {
        result->hasCollision = true;
        result->collisionPoint = point;
        // 最も近い面の法線を設定するべきだが、簡易的な実装では省略
        result->normal = {0, 1, 0};
        result->penetration = 0.0f;
    }
    
    return true;
}

bool ConvexMeshCollider::Intersects(const Line& line, CollisionResult* result) const {
    // レイに変換して処理
    Ray ray = Ray::CreateFromLine(line);
    float maxDistance = line.GetLength();
    
    return Intersects(ray, maxDistance, result);
}

bool ConvexMeshCollider::Intersects(const Ray& ray, float maxDistance, CollisionResult* result) const {
    // 簡易的な実装：TriangleMeshColliderのインターセクト処理を使用
    TriangleMeshCollider tempCollider(triangles_);
    return tempCollider.Intersects(ray, maxDistance, result);
}

bool ConvexMeshCollider::Intersects(const Sphere& sphere, CollisionResult* result) const {
    // 簡易的な実装：TriangleMeshColliderのインターセクト処理を使用
    TriangleMeshCollider tempCollider(triangles_);
    return tempCollider.Intersects(sphere, result);
}

bool ConvexMeshCollider::Intersects(const AABB& aabb, CollisionResult* result) const {
    // 簡易的な実装：TriangleMeshColliderのインターセクト処理を使用
    TriangleMeshCollider tempCollider(triangles_);
    return tempCollider.Intersects(aabb, result);
}

bool ConvexMeshCollider::Intersects(const OBB& obb, CollisionResult* result) const {
    // 簡易的な実装：TriangleMeshColliderのインターセクト処理を使用
    TriangleMeshCollider tempCollider(triangles_);
    return tempCollider.Intersects(obb, result);
}

bool ConvexMeshCollider::Intersects(const IMeshCollider& other, CollisionResult* result) const {
    // 簡易的な実装：TriangleMeshColliderのインターセクト処理を使用
    TriangleMeshCollider tempCollider(triangles_);
    return tempCollider.Intersects(other, result);
}

void ConvexMeshCollider::AddVertex(const Vector3& vertex) {
    vertices_.push_back(vertex);
    
    // 凸包を再計算
    CalculateConvexHull();
    
    // 境界ボリュームを更新
    RecalculateBoundingVolume();
}

void ConvexMeshCollider::SetVertices(const std::vector<Vector3>& vertices) {
    vertices_ = vertices;
    
    // 凸包を計算
    CalculateConvexHull();
    
    // 境界ボリュームを計算
    RecalculateBoundingVolume();
}

void ConvexMeshCollider::RecalculateBoundingVolume() {
    if (vertices_.empty()) {
        // デフォルトのバウンディングボックスを作成
        boundingVolume = std::make_shared<BoundingAABB>(
            Vector3{-1.0f, -1.0f, -1.0f},
            Vector3{1.0f, 1.0f, 1.0f}
        );
        return;
    }
    
    // 境界ボリュームを作成（AABBを使用）
    boundingVolume = std::make_shared<BoundingAABB>(vertices_);
}

void ConvexMeshCollider::CalculateConvexHull() {
    // 簡易的な凸包計算（適切な凸包算出アルゴリズムが必要）
    // ここでは三角形のリストを計算してtriangles_に格納する
    triangles_.clear();
    
    // 少なくとも4点が必要
    if (vertices_.size() < 4) {
        return;
    }
    
    // 最も単純な方法：すべての三角形を生成し、外向きの法線を持つ三角形だけを保持
    // 実用的には不十分で非効率なため、実際の実装では適切な凸包アルゴリズムを使用すべき
    
    // 3つの点から三角形を作成し、それが他のすべての点の「外側」にあるかをチェック
    for (size_t i = 0; i < vertices_.size(); i++) {
        for (size_t j = i + 1; j < vertices_.size(); j++) {
            for (size_t k = j + 1; k < vertices_.size(); k++) {
                Triangle triangle(vertices_[i], vertices_[j], vertices_[k]);
                Vector3 normal = triangle.ComputeNormal();
                
                // すべての他の点が三角形の裏側（内側）にあるかチェック
                bool isValid = true;
                for (size_t l = 0; l < vertices_.size(); l++) {
                    if (l != i && l != j && l != k) {
                        Vector3 toPoint = {
                            vertices_[l].x - triangle.vertices[0].x,
                            vertices_[l].y - triangle.vertices[0].y,
                            vertices_[l].z - triangle.vertices[0].z
                        };
                        
                        float dot = 
                            normal.x * toPoint.x + 
                            normal.y * toPoint.y + 
                            normal.z * toPoint.z;
                        
                        // 点が三角形の表側にある場合、この三角形は凸包の一部ではない
                        if (dot > 0.0001f) {
                            isValid = false;
                            break;
                        }
                    }
                }
                
                if (isValid) {
                    triangles_.push_back(triangle);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// HeightfieldCollider実装
//-----------------------------------------------------------------------------

HeightfieldCollider::HeightfieldCollider(int width, int height, const std::vector<float>& heights, float scaleX, float scaleY, float scaleZ) {
    width_ = width;
    height_ = height;
    heights_ = heights;
    scaleX_ = scaleX;
    scaleY_ = scaleY;
    scaleZ_ = scaleZ;
    
    // 境界ボリュームを計算
    RecalculateBoundingVolume();
}

void HeightfieldCollider::ApplyTransform(const Matrix4x4& worldMatrix) {
    // ワールド行列を保存
    worldMatrix_ = worldMatrix;
    
    // 境界ボリュームに変換を適用
    if (boundingVolume) {
        boundingVolume->ApplyTransform(worldMatrix);
    }
}

bool HeightfieldCollider::Intersects(const Vector3& point, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    if (boundingVolume && !boundingVolume->Contains(point)) {
        return false;
    }
    
    // ワールド座標から高さマップのローカル座標に変換
    // 逆行列が必要だが、簡易的な実装では省略
    
    // 高さマップの範囲内かチェック
    float x = point.x / scaleX_;
    float z = point.z / scaleZ_;
    
    if (x < 0 || x >= width_ - 1 || z < 0 || z >= height_ - 1) {
        return false;
    }
    
    // 高さを取得
    float terrainHeight = GetHeight(x, z);
    
    // 点の高さと比較
    float pointHeight = point.y / scaleY_;
    
    if (pointHeight <= terrainHeight) {
        if (result) {
            result->hasCollision = true;
            result->collisionPoint = point;
            
            // 法線を計算（近似）
            // 実際には高さマップから法線を計算する必要がある
            result->normal = {0, 1, 0};
            
            // めり込み量
            result->penetration = (terrainHeight - pointHeight) * scaleY_;
        }
        return true;
    }
    
    return false;
}

bool HeightfieldCollider::Intersects(const Line& line, CollisionResult* result) const {
    // レイに変換して処理
    Ray ray = Ray::CreateFromLine(line);
    float maxDistance = line.GetLength();
    
    return Intersects(ray, maxDistance, result);
}

bool HeightfieldCollider::Intersects(const Ray& ray, float maxDistance, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    // 現在はシンプル化のため省略
    
    // 指定された座標にある三角形を取得
    Triangle triangle1, triangle2;
    
    // 地形の各グリッドセルを調査
    for (int x = 0; x < width_ - 1; x++) {
        for (int z = 0; z < height_ - 1; z++) {
            if (GetTrianglesAt(x, z, &triangle1, &triangle2)) {
                // 各三角形との交差判定
                TriangleMeshCollider tempCollider;
                
                tempCollider.AddTriangle(triangle1);
                if (triangle2.vertices[0].x != triangle2.vertices[1].x || 
                    triangle2.vertices[0].y != triangle2.vertices[1].y || 
                    triangle2.vertices[0].z != triangle2.vertices[1].z) {
                    tempCollider.AddTriangle(triangle2);
                }
                
                if (tempCollider.Intersects(ray, maxDistance, result)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool HeightfieldCollider::Intersects(const Sphere& sphere, CollisionResult* result) const {
    // まず境界ボリュームで高速判定
    // 現在はシンプル化のため省略
    
    // 球の中心座標
    float x = sphere.center.x / scaleX_;
    float z = sphere.center.z / scaleZ_;
    
    // 球の影響範囲
    int startX = std::max(0, static_cast<int>(x - sphere.radius / scaleX_));
    int endX = std::min(width_ - 2, static_cast<int>(x + sphere.radius / scaleX_));
    int startZ = std::max(0, static_cast<int>(z - sphere.radius / scaleZ_));
    int endZ = std::min(height_ - 2, static_cast<int>(z + sphere.radius / scaleZ_));
    
    bool hasCollision = false;
    float minPenetration = std::numeric_limits<float>::max();
    CollisionResult tempResult;
    
    // 範囲内の各グリッドセルを調査
    for (int gx = startX; gx <= endX; gx++) {
        for (int gz = startZ; gz <= endZ; gz++) {
            Triangle triangle1, triangle2;
            
            if (GetTrianglesAt(gx, gz, &triangle1, &triangle2)) {
                // 各三角形との交差判定
                TriangleMeshCollider tempCollider;
                
                tempCollider.AddTriangle(triangle1);
                if (triangle2.vertices[0].x != triangle2.vertices[1].x || 
                    triangle2.vertices[0].y != triangle2.vertices[1].y || 
                    triangle2.vertices[0].z != triangle2.vertices[1].z) {
                    tempCollider.AddTriangle(triangle2);
                }
                
                if (tempCollider.Intersects(sphere, &tempResult)) {
                    hasCollision = true;
                    
                    if (tempResult.penetration < minPenetration) {
                        minPenetration = tempResult.penetration;
                        
                        if (result) {
                            *result = tempResult;
                        }
                    }
                }
            }
        }
    }
    
    return hasCollision;
}

bool HeightfieldCollider::Intersects(const AABB& aabb, CollisionResult* result) const {
    // TODO: AABBとの詳細な衝突判定
    // 簡略化のため省略
    
    return false;
}

bool HeightfieldCollider::Intersects(const OBB& obb, CollisionResult* result) const {
    // TODO: OBBとの詳細な衝突判定
    // 簡略化のため省略
    
    return false;
}

bool HeightfieldCollider::Intersects(const IMeshCollider& other, CollisionResult* result) const {
    // TODO: メッシュコライダーとの詳細な衝突判定
    // 簡略化のため省略
    
    return false;
}

void HeightfieldCollider::RecalculateBoundingVolume() {
    if (heights_.empty() || width_ <= 0 || height_ <= 0) {
        // デフォルトのバウンディングボックスを作成
        boundingVolume = std::make_shared<BoundingAABB>(
            Vector3{-1.0f, -1.0f, -1.0f},
            Vector3{1.0f, 1.0f, 1.0f}
        );
        return;
    }
    
    // 最小・最大高さを計算
    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::lowest();
    
    for (float height : heights_) {
        minHeight = std::min(minHeight, height);
        maxHeight = std::max(maxHeight, height);
    }
    
    // 境界ボックスを作成
    Vector3 min = {
        0.0f,
        minHeight * scaleY_,
        0.0f
    };
    
    Vector3 max = {
        (width_ - 1) * scaleX_,
        maxHeight * scaleY_,
        (height_ - 1) * scaleZ_
    };
    
    boundingVolume = std::make_shared<BoundingAABB>(min, max);
}

float HeightfieldCollider::GetHeight(float x, float z) const {
    // 座標をグリッド範囲内に制限
    x = std::max(0.0f, std::min(x, static_cast<float>(width_ - 1) - 0.001f));
    z = std::max(0.0f, std::min(z, static_cast<float>(height_ - 1) - 0.001f));
    
    // グリッドセルのインデックス
    int gridX = static_cast<int>(x);
    int gridZ = static_cast<int>(z);
    
    // セル内での相対座標
    float fracX = x - gridX;
    float fracZ = z - gridZ;
    
    // 4つの頂点の高さ
    float h00 = heights_[gridZ * width_ + gridX];
    float h10 = heights_[gridZ * width_ + (gridX + 1)];
    float h01 = heights_[(gridZ + 1) * width_ + gridX];
    float h11 = heights_[(gridZ + 1) * width_ + (gridX + 1)];
    
    // バイリニア補間
    if (fracX + fracZ <= 1.0f) {
        // 左下の三角形
        return h00 + (h10 - h00) * fracX + (h01 - h00) * fracZ;
    } else {
        // 右上の三角形
        return h11 + (h01 - h11) * (1.0f - fracX) + (h10 - h11) * (1.0f - fracZ);
    }
}

bool HeightfieldCollider::GetTrianglesAt(float x, float z, Triangle* triangle1, Triangle* triangle2) const {
    // 座標をグリッド範囲内に制限
    x = std::max(0.0f, std::min(x, static_cast<float>(width_ - 2)));
    z = std::max(0.0f, std::min(z, static_cast<float>(height_ - 2)));
    
    // グリッドセルのインデックス
    int gridX = static_cast<int>(x);
    int gridZ = static_cast<int>(z);
    
    // 4つの頂点の座標
    Vector3 v00 = {
        gridX * scaleX_,
        heights_[gridZ * width_ + gridX] * scaleY_,
        gridZ * scaleZ_
    };
    
    Vector3 v10 = {
        (gridX + 1) * scaleX_,
        heights_[gridZ * width_ + (gridX + 1)] * scaleY_,
        gridZ * scaleZ_
    };
    
    Vector3 v01 = {
        gridX * scaleX_,
        heights_[(gridZ + 1) * width_ + gridX] * scaleY_,
        (gridZ + 1) * scaleZ_
    };
    
    Vector3 v11 = {
        (gridX + 1) * scaleX_,
        heights_[(gridZ + 1) * width_ + (gridX + 1)] * scaleY_,
        (gridZ + 1) * scaleZ_
    };
    
    // 2つの三角形を構築
    if (triangle1) {
        *triangle1 = Triangle(v00, v10, v11);
    }
    
    if (triangle2) {
        *triangle2 = Triangle(v00, v11, v01);
    }
    
    return true;
}

} // namespace Collision