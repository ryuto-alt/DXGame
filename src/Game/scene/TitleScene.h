#pragma once

#include "IScene.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Model.h"
#include <memory>

// タイトルシーンクラス
class TitleScene : public IScene {
public:
    // コンストラクタ
    TitleScene();

    // デストラクタ
    ~TitleScene() override;

    // 初期化
    void Initialize() override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 終了処理
    void Finalize() override;

private:
    // ImGuiの初期化
    void InitializeImGui();

    // 3Dモデルの初期化
    void Initialize3DModels();

    // スプライトの初期化
    void InitializeSprites();

    // デバッグ情報描画
    void DrawImGui();

private:
    // シーンの状態管理
    bool initialized_ = false;

    // タイトルロゴ
    std::unique_ptr<Sprite> titleLogo_;

    // 3Dモデル
    std::unique_ptr<Model> sphereModel_;
    std::unique_ptr<Object3d> sphereObject_;

    // 回転角度
    float rotationAngle_ = 0.0f;
};