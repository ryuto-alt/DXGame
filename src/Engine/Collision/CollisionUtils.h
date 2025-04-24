#pragma once
#include "Model.h"
#include "Object3d.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include <memory>

// 衝突判定ユーティリティクラス
class CollisionUtils {
public:
    // モデルの形状に合わせた球体コライダーを自動生成
    static SphereCollider* CreateSphereColliderFromModel(const Model* model, Object3d* parentObject = nullptr) {
        // nullptrチェック
        if (!model) {
            return nullptr;
        }

        // 球体コライダーを作成
        SphereCollider* collider = new SphereCollider();

        // 頂点データから境界球を計算
        const std::vector<VertexData>& vertices = model->GetVertices();
        if (vertices.empty()) {
            // 頂点がなければデフォルト値で初期化して返す
            collider->SetRadius(1.0f);
            return collider;
        }

        // 3D境界球を計算（すべての頂点を包む最小の球）

        // まず中心点を計算（頂点の平均位置）
        Vector3 center = { 0.0f, 0.0f, 0.0f };
        for (const auto& vertex : vertices) {
            center.x += vertex.position.x;
            center.y += vertex.position.y;
            center.z += vertex.position.z;
        }
        center.x /= static_cast<float>(vertices.size());
        center.y /= static_cast<float>(vertices.size());
        center.z /= static_cast<float>(vertices.size());

        // 中心点からの最大距離を計算（半径）
        float maxDistSquared = 0.0f;
        for (const auto& vertex : vertices) {
            float dx = vertex.position.x - center.x;
            float dy = vertex.position.y - center.y;
            float dz = vertex.position.z - center.z;
            float distSquared = dx * dx + dy * dy + dz * dz;
            if (distSquared > maxDistSquared) {
                maxDistSquared = distSquared;
            }
        }
        float radius = std::sqrt(maxDistSquared);

        // コライダーに設定
        collider->SetRadius(radius);
        collider->SetOffset(center);

        // 親オブジェクトがあれば設定
        if (parentObject) {
            parentObject->AddCollider(collider);
        }

        return collider;
    }

    // モデルの形状に合わせたボックスコライダーを自動生成
    static BoxCollider* CreateBoxColliderFromModel(const Model* model, Object3d* parentObject = nullptr) {
        // nullptrチェック
        if (!model) {
            return nullptr;
        }

        // ボックスコライダーを作成
        BoxCollider* collider = new BoxCollider();

        // 頂点データから境界ボックスを計算
        const std::vector<VertexData>& vertices = model->GetVertices();
        if (vertices.empty()) {
            // 頂点がなければデフォルト値で初期化して返す
            collider->SetSize({ 1.0f, 1.0f, 1.0f });
            return collider;
        }

        // AABBを計算（軸に揃えられた境界ボックス）
        Vector3 min = {
            vertices[0].position.x,
            vertices[0].position.y,
            vertices[0].position.z
        };
        Vector3 max = min;

        // 最小・最大座標を見つける
        for (const auto& vertex : vertices) {
            // X座標
            if (vertex.position.x < min.x) min.x = vertex.position.x;
            if (vertex.position.x > max.x) max.x = vertex.position.x;

            // Y座標
            if (vertex.position.y < min.y) min.y = vertex.position.y;
            if (vertex.position.y > max.y) max.y = vertex.position.y;

            // Z座標
            if (vertex.position.z < min.z) min.z = vertex.position.z;
            if (vertex.position.z > max.z) max.z = vertex.position.z;
        }

        // 中心座標とサイズを計算
        Vector3 center = {
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        };

        Vector3 size = {
            (max.x - min.x) * 0.5f,
            (max.y - min.y) * 0.5f,
            (max.z - min.z) * 0.5f
        };

        // コライダーに設定
        collider->SetSize(size);
        collider->SetOffset(center); // モデルの中心からオフセット

        // 親オブジェクトがあれば設定
        if (parentObject) {
            parentObject->AddCollider(collider);
        }

        return collider;
    }
};