#pragma once

#include "UnoEngine.h"

class TitleScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    TitleScene();
    ~TitleScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // 初期化済みフラグ
    bool initialized_ = false;

    // タイトルロゴ
    std::unique_ptr<Sprite> titleLogo_;

    // 3Dオブジェクト（背景用）
    std::unique_ptr<Model> sphereModel_;
    std::unique_ptr<Object3d> sphereObject_;

    // 回転アニメーション用
    float rotationAngle_ = 0.0f;
};