#pragma once

#include <string>
#include <memory>
#include "IScene.h"

// シーンファクトリー抽象クラス
class SceneFactory {
public:
    // 仮想デストラクタ
    virtual ~SceneFactory() = default;

    // シーン生成メソッド（純粋仮想関数）
    virtual std::unique_ptr<IScene> CreateScene(const std::string& sceneName) = 0;
};