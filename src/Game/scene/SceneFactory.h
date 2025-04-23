// SceneFactory.h
// シーン生成ファクトリー（Factory Methodパターン）
#pragma once

#include <string>
#include <memory>
#include "IScene.h"

// シーンファクトリー抽象クラス
class SceneFactory {
public:
    // 仮想デストラクタ
    virtual ~SceneFactory() = default;

    // シーン生成メソッド
    virtual std::unique_ptr<IScene> CreateScene(const std::string& sceneName) = 0;
};

// 具体的なシーンファクトリー
class GameSceneFactory : public SceneFactory {
public:
    // シーン生成メソッドのオーバーライド
    std::unique_ptr<IScene> CreateScene(const std::string& sceneName) override;
};