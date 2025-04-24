#pragma once
#include "UnoEngine.h"

// シンプルなゲームプレイシーン（青い画面表示のみ）
class GamePlayScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    GamePlayScene();
    ~GamePlayScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

protected:
    // 初期化済みフラグ
    bool initialized_ = false;
};