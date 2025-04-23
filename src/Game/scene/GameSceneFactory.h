#pragma once

#include "SceneFactory.h"

// ゲーム用シーンファクトリークラス
class GameSceneFactory : public SceneFactory {
public:
    // コンストラクタ
    GameSceneFactory() = default;

    // デストラクタ
    ~GameSceneFactory() override = default;

    // シーン生成メソッドのオーバーライド
    std::unique_ptr<IScene> CreateScene(const std::string& sceneName) override;
};