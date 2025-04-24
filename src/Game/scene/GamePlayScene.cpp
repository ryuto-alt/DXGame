#include "GamePlayScene.h"
#include <functional>

// プレイヤーのコリジョンハンドラー定義
void GamePlayScene::PlayerCollisionHandler(Collision::CollisionObject* other, const Collision::CollisionResult& result) {
    // コリジョン情報をデバッグ出力
    OutputDebugStringA("Player collided with an object!\n");

    // 衝突フラグを立てる
    playerCollisionDetected = true;

    // めり込み分だけ押し戻し
    Vector3 pushbackVector = {
        result.normal.x * result.penetration,
        result.normal.y * result.penetration,
        result.normal.z * result.penetration
    };

    // プレイヤーの位置を更新
    Vector3 currentPos = playerCollider->GetSphere().center;
    Vector3 newPos = Collision::Utility::Add(currentPos, pushbackVector);
    playerCollider->GetSphere().center = newPos;

    // 3Dオブジェクトの位置も更新
    playerObject->SetPosition(newPos);
}

// 敵のコリジョンハンドラー定義
void GamePlayScene::EnemyCollisionHandler(Collision::CollisionObject* other, const Collision::CollisionResult& result) {
    // プレイヤーとの衝突時の処理
    OutputDebugStringA("Enemy collided with an object!\n");

    // 衝突フラグを立てる
    enemyCollisionDetected = true;

    // プレイヤーと衝突した場合の処理
    if (other == playerCollider.get()) {
        OutputDebugStringA("Enemy hit the player!\n");
        // ここにプレイヤーへのダメージ処理などを追加
    }
}

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    // コリジョンマネージャーから自分のコライダーを削除
    if (playerCollider) {
        Collision::CollisionManager::GetInstance()->RemoveCollider(playerCollider);
    }
    if (enemyCollider) {
        Collision::CollisionManager::GetInstance()->RemoveCollider(enemyCollider);
    }
    for (auto& obstacle : obstacles) {
        Collision::CollisionManager::GetInstance()->RemoveCollider(obstacle);
    }
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期設定
    camera_->SetTranslate({ 0.0f, 5.0f, -10.0f });
    camera_->SetRotate({ cameraPitch, cameraYaw, 0.0f });
    camera_->Update();

    // コリジョンマネージャーを取得
    auto collisionManager = Collision::CollisionManager::GetInstance();

    // プレイヤーの初期化
    // 3Dモデルの読み込み - Player.objを使用
    playerModel = std::make_unique<Model>();
    playerModel->Initialize(dxCommon_);
    playerModel->LoadFromObj("Resources/models", "Player.obj"); // Player.objに変更

    // 3Dオブジェクトの初期化
    playerObject = std::make_unique<Object3d>();
    playerObject->Initialize(dxCommon_, spriteCommon_);
    playerObject->SetModel(playerModel.get());
    playerObject->SetScale({ 1.0f, 1.0f, 1.0f });
    playerObject->SetPosition({ 0.0f, 0.5f, 0.0f }); // 初期位置を原点近くに配置

    // プレイヤーの球コライダーを作成
    playerCollider = std::make_shared<Collision::SphereCollider>(
        Vector3{ 0.0f, 0.5f, 0.0f }, // 初期位置
        1.0f                        // 半径
    );

    // プレイヤーを剛体として設定
    playerCollider->SetIsRigidbody(true);

    // 衝突時のコールバック関数を設定
    using namespace std::placeholders;
    playerCollider->onCollisionEnter = std::bind(&GamePlayScene::PlayerCollisionHandler, this, _1, _2);

    // コリジョンマネージャーに登録
    collisionManager->AddCollider(playerCollider);

    // 敵の初期化
    // 3Dモデルの読み込み
    enemyModel = std::make_unique<Model>();
    enemyModel->Initialize(dxCommon_);
    enemyModel->LoadFromObj("Resources/models/cube", "cube.obj");

    // 3Dオブジェクトの初期化
    enemyObject = std::make_unique<Object3d>();
    enemyObject->Initialize(dxCommon_, spriteCommon_);
    enemyObject->SetModel(enemyModel.get());
    enemyObject->SetScale({ 1.0f, 1.0f, 1.0f });
    enemyObject->SetPosition({ 5.0f, 1.0f, 0.0f });

    // 敵のカプセルコライダーを作成
    enemyCollider = std::make_shared<Collision::CapsuleCollider>(
        Vector3{ 5.0f, 0.0f, 0.0f },  // 始点
        Vector3{ 5.0f, 2.0f, 0.0f },  // 終点
        0.5f                         // 半径
    );

    // 衝突時のコールバック関数を設定
    enemyCollider->onCollisionEnter = std::bind(&GamePlayScene::EnemyCollisionHandler, this, _1, _2);

    // コリジョンマネージャーに登録
    collisionManager->AddCollider(enemyCollider);

    // 障害物の作成
    // 静的な障害物1（球）
    auto obstacle1 = std::make_shared<Collision::SphereCollider>(
        Vector3{ -5.0f, 1.0f, 0.0f }, // 位置
        1.5f                         // 半径
    );
    collisionManager->AddCollider(obstacle1);
    obstacles.push_back(obstacle1);

    // 障害物1の3Dモデル
    auto obstacle1Model = std::make_unique<Model>();
    obstacle1Model->Initialize(dxCommon_);
    obstacle1Model->LoadFromObj("Resources/models", "sphere.obj");
    obstacleModels.push_back(std::move(obstacle1Model));

    // 障害物1の3Dオブジェクト
    auto obstacle1Object = std::make_unique<Object3d>();
    obstacle1Object->Initialize(dxCommon_, spriteCommon_);
    obstacle1Object->SetModel(obstacleModels.back().get());
    obstacle1Object->SetScale({ 1.0f, 1.0f, 1.0f });
    obstacle1Object->SetPosition({ -5.0f, 1.0f, 0.0f });
    obstacleObjects.push_back(std::move(obstacle1Object));

    // 静的な障害物2（カプセル）
    auto obstacle2 = std::make_shared<Collision::CapsuleCollider>(
        Vector3{ 0.0f, 0.0f, 5.0f },  // 始点
        Vector3{ 0.0f, 3.0f, 5.0f },  // 終点
        0.7f                         // 半径
    );
    collisionManager->AddCollider(obstacle2);
    obstacles.push_back(obstacle2);

    // 障害物2の3Dモデル
    auto obstacle2Model = std::make_unique<Model>();
    obstacle2Model->Initialize(dxCommon_);
    obstacle2Model->LoadFromObj("Resources/models/Cylinder", "cylinder.obj");
    obstacleModels.push_back(std::move(obstacle2Model));

    // 障害物2の3Dオブジェクト
    auto obstacle2Object = std::make_unique<Object3d>();
    obstacle2Object->Initialize(dxCommon_, spriteCommon_);
    obstacle2Object->SetModel(obstacleModels.back().get());
    obstacle2Object->SetScale({ 0.7f, 1.5f, 0.7f });
    obstacle2Object->SetPosition({ 0.0f, 1.5f, 5.0f });
    obstacleObjects.push_back(std::move(obstacle2Object));

    // 障害物の衝突フラグ配列を初期化
    obstacleCollisionDetected.resize(obstacles.size(), false);

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: Successfully initialized\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // 衝突フラグをリセット
    playerCollisionDetected = false;
    enemyCollisionDetected = false;
    for (size_t i = 0; i < obstacleCollisionDetected.size(); i++) {
        obstacleCollisionDetected[i] = false;
    }

    // カメラの更新
    UpdateCamera();

    // プレイヤーの移動処理
    UpdatePlayerMovement();

    // プレイヤーオブジェクトの更新
    playerObject->Update();

    // 敵オブジェクトの更新
    enemyObject->Update();

    // 障害物オブジェクトの更新
    for (auto& obstacleObject : obstacleObjects) {
        obstacleObject->Update();
    }

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // プレイヤーの描画
    playerObject->Draw();

    // 敵の描画
    enemyObject->Draw();

    // 障害物の描画
    for (auto& obstacleObject : obstacleObjects) {
        obstacleObject->Draw();
    }

    // ImGuiで操作説明と衝突判定状況を表示（英語表記に変更）
    ImGui::Begin("Game Controls & Collision Status");
    ImGui::Text("Controls:");
    ImGui::Text("WASD - Player Movement");
    ImGui::Text("Mouse - Camera Control");
    ImGui::Text("ESC - Return to Title");

    ImGui::Separator();

    ImGui::Text("Collision Status:");
    ImGui::Text("Player collision: %s", playerCollisionDetected ? "Detected" : "None");
    ImGui::Text("Enemy collision: %s", enemyCollisionDetected ? "Detected" : "None");

    for (size_t i = 0; i < obstacleCollisionDetected.size(); i++) {
        ImGui::Text("Obstacle %d collision: %s",
            static_cast<int>(i) + 1,
            obstacleCollisionDetected[i] ? "Detected" : "None");
    }

    ImGui::End();
}

void GamePlayScene::Finalize() {
    // 特に何もする必要がない
    OutputDebugStringA("GamePlayScene: Finalized\n");
}

void GamePlayScene::UpdatePlayerMovement() {
    // 移動量
    Vector3 movement = { 0.0f, 0.0f, 0.0f };

    // WASDキーによる移動入力（修正済み - 正しい方向に）
    if (input_->PushKey(DIK_W)) movement.z += playerSpeed;  // 前方向
    if (input_->PushKey(DIK_S)) movement.z -= playerSpeed;  // 後方向
    if (input_->PushKey(DIK_A)) movement.x -= playerSpeed;  // 左方向
    if (input_->PushKey(DIK_D)) movement.x += playerSpeed;  // 右方向

    // 移動があれば位置を更新
    if (movement.x != 0.0f || movement.y != 0.0f || movement.z != 0.0f) {
        // カメラの向きに合わせて移動方向を変換
        Matrix4x4 rotationY = MakeRotateYMatrix(cameraYaw);

        Vector3 rotatedMovement;
        rotatedMovement.x = movement.x * rotationY.m[0][0] + movement.z * rotationY.m[0][2];
        rotatedMovement.y = movement.y;
        rotatedMovement.z = movement.x * rotationY.m[2][0] + movement.z * rotationY.m[2][2];

        // 現在の位置を取得
        Vector3 currentPos = playerCollider->GetSphere().center;

        // 新しい位置を計算
        Vector3 newPos = Collision::Utility::Add(currentPos, rotatedMovement);

        // コライダーの位置を更新
        playerCollider->GetSphere().center = newPos;

        // 3Dオブジェクトの位置も更新
        playerObject->SetPosition(newPos);

        // 速度も設定（当たり判定のスウィープテスト用）
        playerCollider->SetVelocity(rotatedMovement);
    }
    else {
        // 移動がなければ速度をゼロに
        playerCollider->SetVelocity({ 0.0f, 0.0f, 0.0f });
    }
}

void GamePlayScene::UpdateCamera() {
    // マウス入力の処理
    DIMOUSESTATE mouseState;
    if (SUCCEEDED(input_->GetMouseState(&mouseState))) {
        // マウスの現在位置を取得
        Vector2 currentMousePos;
        currentMousePos.x = static_cast<float>(mouseState.lX);
        currentMousePos.y = static_cast<float>(mouseState.lY);

        // 初回入力時は比較対象がないのでスキップ
        if (!isFirstMouseInput) {
            // マウスの移動量を計算
            float xOffset = currentMousePos.x - lastMousePos.x;
            float yOffset = currentMousePos.y - lastMousePos.y;

            // 感度調整
            const float sensitivity = 0.001f;
            xOffset *= sensitivity;
            yOffset *= sensitivity;

            // カメラの向きを更新
            cameraYaw += xOffset;
            cameraPitch -= yOffset; // Y軸は反転

            // ピッチの制限（-89度から89度）
            const float pitchLimit = 3.14159f * 0.49f; // 約89度
            if (cameraPitch > pitchLimit) {
                cameraPitch = pitchLimit;
            }
            if (cameraPitch < -pitchLimit) {
                cameraPitch = -pitchLimit;
            }
        }

        // 現在のマウス位置を保存
        lastMousePos = currentMousePos;
        isFirstMouseInput = false;

        // マウスカーソルを非表示に
        input_->SetMouseCursor(false);
    }

    // プレイヤーの位置を取得
    Vector3 playerPos = playerObject->GetPosition();

    // カメラの位置を計算（プレイヤーの後ろ上に配置）
    float camX = playerPos.x - cameraDistance * sin(cameraYaw) * cos(cameraPitch);
    float camY = playerPos.y + cameraDistance * sin(cameraPitch);
    float camZ = playerPos.z - cameraDistance * cos(cameraYaw) * cos(cameraPitch);

    // カメラの設定を更新
    camera_->SetTranslate({ camX, camY, camZ });
    camera_->SetRotate({ cameraPitch, cameraYaw, 0.0f });
    camera_->Update();
}