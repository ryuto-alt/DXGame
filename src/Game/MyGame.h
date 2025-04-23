// MyGame.h
#pragma once

#include "Framework.h"
#include "UnoEngine.h"
#include "SceneFactory.h" // 抽象基底クラスをインクルード

// GameSceneFactoryは前方宣言
class GameSceneFactory;

class MyGame : public Framework {
public:
    // コンストラクタ・デストラクタ
    MyGame();
    ~MyGame() override;

    // 初期化
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // UnoEngineのインスタンス
    UnoEngine* engine_ = nullptr;

    // SceneFactoryを基底クラス型として保持
    std::unique_ptr<SceneFactory> sceneFactory_;
};