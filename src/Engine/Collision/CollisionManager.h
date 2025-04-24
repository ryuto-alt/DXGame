#pragma once

#include "Collision.h"
#include "BoundingVolume.h"
#include "MeshCollider.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <unordered_set>

namespace Collision {

    // 前方宣言
    class CollisionManager;

    // コリジョンマネージャクラス（シングルトン）
    class CollisionManager {
    private:
        // シングルトンインスタンス
        static CollisionManager* instance_;

        // コンストラクタ（シングルトン）
        CollisionManager() = default;
        // デストラクタ（シングルトン）
        ~CollisionManager() = default;
        // コピー禁止
        CollisionManager(const CollisionManager&) = delete;
        CollisionManager& operator=(const CollisionManager&) = delete;

    public:
        // シングルトンインスタンスの取得
        static CollisionManager* GetInstance();

        // コリジョンマネージャの初期化
        void Initialize();

        // コリジョンマネージャの終了
        void Finalize();

        // 衝突判定の実行
        void CheckAllCollisions();

        // 衝突関係のクリア
        void ClearCollisions();

        // コライダーの登録
        void AddCollider(std::shared_ptr<ICollider> collider);

        // コライダーの削除
        void RemoveCollider(uint32_t colliderID);
        void RemoveCollider(const std::string& name);
        void RemoveCollider(ICollider* collider);

        // コライダーの検索
        std::shared_ptr<ICollider> FindCollider(uint32_t colliderID);
        std::shared_ptr<ICollider> FindCollider(const std::string& name);

        // 衝突判定をスキップするペアの追加
        void AddIgnorePair(uint32_t collider1ID, uint32_t collider2ID);
        void AddIgnorePair(const std::string& collider1Name, const std::string& collider2Name);

        // 衝突判定をスキップするペアの削除
        void RemoveIgnorePair(uint32_t collider1ID, uint32_t collider2ID);
        void RemoveIgnorePair(const std::string& collider1Name, const std::string& collider2Name);

        // デバッグ描画
        void DebugDraw();

        // コライダー作成ヘルパー関数
        std::shared_ptr<SphereCollider> CreateSphereCollider(const std::string& name, const Vector3& center, float radius);
        std::shared_ptr<AABBCollider> CreateAABBCollider(const std::string& name, const Vector3& min, const Vector3& max);
        std::shared_ptr<OBBCollider> CreateOBBCollider(const std::string& name, const Vector3& center, const Vector3 orientations[3], const Vector3& size);
        std::shared_ptr<OBBCollider> CreateOBBCollider(const std::string& name, const Matrix4x4& worldMatrix, const Vector3& size);

        // メッシュコライダー作成ヘルパー関数
        std::shared_ptr<MeshCollider> CreateMeshCollider(const std::string& name);
        std::shared_ptr<MeshCollider> CreateTriangleMeshCollider(const std::string& name, const std::vector<Triangle>& triangles);
        std::shared_ptr<MeshCollider> CreateTriangleMeshCollider(const std::string& name, const std::vector<Vector3>& vertices, const std::vector<uint32_t>& indices);
        std::shared_ptr<MeshCollider> CreateConvexMeshCollider(const std::string& name, const std::vector<Vector3>& vertices);
        std::shared_ptr<MeshCollider> CreateHeightfieldCollider(const std::string& name, int width, int height, const std::vector<float>& heights, float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f);

    private:
        // コライダーリスト
        std::vector<std::shared_ptr<ICollider>> colliders_;
        // 名前からコライダーへのマップ
        std::unordered_map<std::string, std::shared_ptr<ICollider>> colliderMap_;
        // IDからコライダーへのマップ
        std::unordered_map<uint32_t, std::shared_ptr<ICollider>> colliderIDMap_;
        // 次のコライダーID
        uint32_t nextColliderID_ = 1;
        // 前回の衝突ペア
        std::unordered_map<uint64_t, bool> prevCollisions_;
        // 衝突スキップペア
        std::unordered_set<uint64_t> ignorePairs_;

        // コライダーペアのハッシュ値を計算
        uint64_t GetColliderPairHash(uint32_t collider1ID, uint32_t collider2ID) const;
    };

} // namespace Collision