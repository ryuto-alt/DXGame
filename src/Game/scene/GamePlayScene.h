#pragma once

#include "IScene.h"
#include <memory>

// ゲームプレイシーンクラス - 基本実装
class GamePlayScene : public IScene {
public:
    // コンストラクタ
    GamePlayScene();

    // デストラクタ
    ~GamePlayScene() override;

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 終了処理
    void Finalize() override;

private:
    // カメラ操作（最小限）
    void ControlCamera();

private:
    // 初期化フラグ
    bool initialized_ = false;

    // ESCキーでタイトルに戻るための機能を残す
    bool showCursor_ = true;
};