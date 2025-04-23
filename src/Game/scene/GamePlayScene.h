#pragma once

#include "UnoEngine.h"

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

private:
    // カメラ操作
    void ControlCamera();

private:
    // 初期化済みフラグ
    bool initialized_ = false;

    // プレイヤーキャラクター
    std::unique_ptr<Model> playerModel_;
    std::unique_ptr<Object3d> playerObject_;

    // 地面オブジェクト
    std::unique_ptr<Model> groundModel_;
    std::unique_ptr<Object3d> groundObject_;

    // パーティクルエミッター
    std::unique_ptr<ParticleEmitter> effectEmitter_;

    // UI要素
    std::unique_ptr<Sprite> uiSprite_;

    // カメラ制御用
    float cameraRotation_ = 0.0f;
    bool showCursor_ = false;
};