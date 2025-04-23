// SceneFactory.cpp
#include "SceneFactory.h"
#include "GamePlayScene.h"
#include "TitleScene.h"
#include <cassert>

std::unique_ptr<IScene> GameSceneFactory::CreateScene(const std::string& sceneName) {
    // シーン名によって対応するシーンインスタンスを生成
    if (sceneName == "Title") {
        return std::make_unique<TitleScene>();
    }
    else if (sceneName == "GamePlay") {
        return std::make_unique<GamePlayScene>();
    }

    // 対応するシーンが見つからない場合はエラー
    assert(0 && "指定されたシーンが存在しません");
    return nullptr;
}