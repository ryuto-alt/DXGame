// IScene.h
// シーンインターフェース（抽象クラス）
#pragma once

#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Camera.h"
#include "SrvManager.h"
#include "AudioManager.h"
#include "WinApp.h"

// 前方宣言
class SceneManager;

// シーンインターフェース
class IScene {
public:
    // 仮想デストラクタ
    virtual ~IScene() = default;

    // 初期化
    virtual void Initialize() = 0;

    // 更新
    virtual void Update() = 0;

    // 描画
    virtual void Draw() = 0;

    // 終了処理
    virtual void Finalize() = 0;

    // シーンマネージャーの設定
    void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

    // 各種リソースの設定
    void SetDirectXCommon(DirectXCommon* dxCommon) { dxCommon_ = dxCommon; }
    void SetInput(Input* input) { input_ = input; }
    void SetSpriteCommon(SpriteCommon* spriteCommon) { spriteCommon_ = spriteCommon; }
    void SetSrvManager(SrvManager* srvManager) { srvManager_ = srvManager; }
    void SetCamera(Camera* camera) { camera_ = camera; }

protected:
    // シーンマネージャー（シーン切り替えに使用）
    SceneManager* sceneManager_ = nullptr;

    // 共通リソース（各シーンで使用）
    DirectXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    Camera* camera_ = nullptr;
};