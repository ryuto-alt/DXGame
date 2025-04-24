#pragma once
// 基本システム
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Camera.h"

// グラフィックス関連
#include "SpriteCommon.h"
#include "Sprite.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Object3d.h"
#include "Model.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"

// 衝突判定関連
#include "Collision.h"
#include "CollisionPrimitive.h"
#include "CollisionUtility.h"
#include "CollisionManager.h"

// オーディオ関連
#include "AudioManager.h"

// シーン管理
#include "SceneManager.h"
#include "SceneFactory.h" // 抽象クラスのみをインクルード
#include "IScene.h"

// 数学・ユーティリティ関連
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Mymath.h"
#include "Logger.h"
#include "StringUtility.h"

// ImGui関連
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <memory>
#include <string>

// UnoEngineクラス - DirectX12ゲームエンジン統合クラス
class UnoEngine {
public:
    // シングルトンインスタンスを取得
    static UnoEngine* GetInstance();

    // コピー禁止
    UnoEngine(const UnoEngine&) = delete;
    UnoEngine& operator=(const UnoEngine&) = delete;

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // 終了処理
    void Finalize();

    // ゲームループ実行
    void Run();

    // 終了リクエスト
    bool IsEndRequested() const { return endRequest_; }
    void RequestEnd() { endRequest_ = true; }

    // 各コンポーネントのアクセサ
    WinApp* GetWinApp() const { return winApp_.get(); }
    DirectXCommon* GetDirectXCommon() const { return dxCommon_.get(); }
    Input* GetInput() const { return input_.get(); }
    Camera* GetCamera() const { return camera_.get(); }
    SpriteCommon* GetSpriteCommon() const { return spriteCommon_.get(); }
    SrvManager* GetSrvManager() const { return srvManager_.get(); }
    SceneManager* GetSceneManager() const { return SceneManager::GetInstance(); }

    // テクスチャマネージャーのアクセサ（シングルトン）
    TextureManager* GetTextureManager() const { return TextureManager::GetInstance(); }

    // パーティクルマネージャーのアクセサ（シングルトン）
    ParticleManager* GetParticleManager() const { return ParticleManager::GetInstance(); }

    // オーディオマネージャーのアクセサ（シングルトン）
    AudioManager* GetAudioManager() const { return AudioManager::GetInstance(); }

    // 衝突判定マネージャーのアクセサ（シングルトン）（追加）
    Collision::CollisionManager* GetCollisionManager() const { return Collision::CollisionManager::GetInstance(); }

    // シーンファクトリーのセッター
    void SetSceneFactory(SceneFactory* sceneFactory);

private:
    // シングルトンインスタンス
    static UnoEngine* instance_;

    // コンストラクタ（シングルトン）
    UnoEngine() = default;
    // デストラクタ（シングルトン）
    ~UnoEngine() = default;

    // 基本コンポーネント
    std::unique_ptr<WinApp> winApp_;
    std::unique_ptr<DirectXCommon> dxCommon_;
    std::unique_ptr<Input> input_;
    std::unique_ptr<Camera> camera_;

    // グラフィックス関連コンポーネント
    std::unique_ptr<SpriteCommon> spriteCommon_;
    std::unique_ptr<SrvManager> srvManager_;

    // シーンファクトリー (SceneFactoryへのポインタとして保存)
    std::unique_ptr<SceneFactory> sceneFactory_;

    // ImGuiの初期化
    void InitializeImGui();

    // 終了リクエストフラグ
    bool endRequest_ = false;
};