#include "GamePlayScene.h"
#include <Windows.h>
#include <cmath>

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    // デストラクタ - コライダーは親オブジェクトによって削除される

    // OBJファイルストリームが開いていれば閉じる
    if (objFileStream_.is_open()) {
        objFileStream_.close();
    }

    // 障害物コライダーリストをクリア
    for (auto* collider : obstacleColliders_) {
        // コライダーの削除はObject3dが担当するので、リストだけクリアする
        if (collider && !collider->GetParentObject()) {
            delete collider; // 親オブジェクトがない場合は手動で削除
        }
    }
    obstacleColliders_.clear();
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);
    assert(winApp_); // winApp_が設定されていることを確認

    // カメラの初期設定
    camera_->SetTranslate({ 0.0f, 2.0f, -10.0f });
    camera_->SetFovY(0.45f); // 視野角を設定（約26度）
    camera_->SetAspectRatio(static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight));
    // ニアクリップとファークリップの設定を追加
    camera_->SetNearClip(0.1f);  // 小さすぎる値を避ける
    camera_->SetFarClip(1000.0f);
    camera_->Update();

    // プレイヤーモデルの読み込み
    playerModel_ = std::make_unique<Model>();
    playerModel_->Initialize(dxCommon_);
    playerModel_->LoadFromObj("Resources/models", "player.obj");

    // プレイヤーオブジェクトの初期化
    playerObject_ = std::make_unique<Object3d>();
    playerObject_->Initialize(dxCommon_, spriteCommon_);
    playerObject_->SetModel(playerModel_.get());
    playerObject_->SetPosition({ 0.0f, 3.0f, 0.0f });
    playerObject_->SetScale({ 1.0f, 1.0f, 1.0f });
    playerObject_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 不透明度を1.0に設定

    // ステージモデルの読み込み
    stageModel_ = std::make_unique<Model>();
    stageModel_->Initialize(dxCommon_);
    stageModel_->LoadFromObj("Resources/models/stage1", "stage1.obj");

    // ステージオブジェクトの初期化
    stageObject_ = std::make_unique<Object3d>();
    stageObject_->Initialize(dxCommon_, spriteCommon_);
    stageObject_->SetModel(stageModel_.get());
    stageObject_->SetPosition({ 0.0f, 0.0f, 0.0f });
    stageObject_->SetScale({ 1.0f, 1.0f, 1.0f }); // ステージのサイズ
    stageObject_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 不透明度を1.0に設定

    // プレイヤーの衝突判定用コライダーを追加
    playerCollider_ = new SphereCollider();
    playerCollider_->SetRadius(0.8f);  // プレイヤーサイズに合わせて調整
    playerCollider_->SetOffset({ 0.0f, 0.0f, 0.0f });
    playerObject_->AddCollider(playerCollider_);

    // プレイヤーに物理ボディを設定
    playerObject_->EnablePhysics(true);
    PhysicsBody* playerPhysics = playerObject_->GetPhysicsBody();
    PhysicsProperties props;
    props.mass = 1.0f;
    props.restitution = 0.2f;  // 反発係数を小さめに
    props.friction = 0.5f;     // 摩擦を大きめに
    props.isStatic = false;    // 動的オブジェクト
    props.useGravity = true;   // 重力を使用
    props.gravity = { 0.0f, -9.8f, 0.0f };
    playerPhysics->SetPhysicsProperties(props);

    // コールバックを設定
    playerObject_->SetOnCollisionCallback(
        [this](const CollisionInfo& info) {
            this->OnPlayerCollision(info);
        }
    );

    // OBJファイルを開いてAABBを解析
    objFileStream_.open("Resources/models/stage1/stage1.obj");
    if (objFileStream_.is_open()) {
        UpdateStageAABB();
        objFileStream_.close();
    }
    else {
        // OBJファイルが開けない場合は従来の方法でステージコライダーを設定
        stageCollider_ = new BoxCollider();
        stageCollider_->SetSize({ 20.0f, 1.0f, 20.0f });
        stageCollider_->SetOffset({ 0.0f, -0.5f, 0.0f });
        stageObject_->AddCollider(stageCollider_);
    }

    // 障害物コライダーの作成
    CreateObstacleColliders();

    // ステージは静的オブジェクト（動かない）として設定
    stageObject_->EnablePhysics(true);
    PhysicsBody* stagePhysics = stageObject_->GetPhysicsBody();
    PhysicsProperties stageProps;
    stageProps.isStatic = true;  // 静的オブジェクト
    stageProps.useGravity = false;  // 重力の影響を受けない
    stagePhysics->SetPhysicsProperties(stageProps);

    // オブジェクトの更新
    playerObject_->Update();
    stageObject_->Update();

    // 衝突判定を有効化
    CollisionManager::GetInstance()->Initialize();

    // カメラ初期位置をプレイヤー位置に合わせて設定
    UpdateCamera();

    // マウスカーソルを中央に設定
    SetCursorPos(WinApp::kClientWidth / 2, WinApp::kClientHeight / 2);
    prevMouseX_ = WinApp::kClientWidth / 2;
    prevMouseY_ = WinApp::kClientHeight / 2;

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: Successfully initialized with collision test\n");
}

void GamePlayScene::UpdateStageAABB() {
    std::string line;
    uint32_t cornerNumber = 0;
    Vector3 max;
    Vector3 min;
    bool start = false;
    bool reverse = false;

    // ファイルの先頭に戻す
    objFileStream_.clear();
    objFileStream_.seekg(0, std::ios::beg);

    while (std::getline(objFileStream_, line)) {
        std::istringstream line_stream(line);
        std::string word;
        std::getline(line_stream, word, ' ');
        if (word.find("vn") == 0) {
            break; // vn（法線ベクトル）を読み取ったら終了
        }
        if (word.find("v") == 0) {
            cornerNumber++;
        }
        else {
            continue;
        }
        if (cornerNumber > 0) {
            std::getline(line_stream, word, ' ');
            float x = (float)std::atof(word.c_str());
            std::getline(line_stream, word, ' ');
            float y = (float)std::atof(word.c_str());
            std::getline(line_stream, word, ' ');
            float z = (float)std::atof(word.c_str());
            if (!start) {
                max = { x, y, z };
                min = { x, y, z };
                start = true;
            }
            else {
                // 前よりも大きいとき
                if (max.x <= x) {
                    max.x = x;
                }
                // 前よりも小さいとき
                if (min.x > x) {
                    min.x = x;
                }
                if (max.y <= y) {
                    max.y = y;
                }
                if (min.y > y) {
                    min.y = y;
                }
                if (max.z <= z) {
                    max.z = z;
                }
                if (min.z > z) {
                    min.z = z;
                }
            }
        }
        if (cornerNumber == 8) {
            if (!reverse) {
                AddObstacle(allObstacles_, min, max); // 結合した基盤となるobj
            }
            else {
                float minX;
                float maxX;
                maxX = -(max.x);
                minX = -(min.x);
                min.x = maxX;
                max.x = minX;
                AddObstacle(allObstacles_, { min.x, min.y, min.z }, { max.x, max.y, max.z }); // それ以外のすべてobj
            }
            cornerNumber = 0;
            start = false;
            reverse = true;
        }
    }

    OutputDebugStringA(("GamePlayScene: AABB loaded - " + std::to_string(allObstacles_.size()) + " obstacles\n").c_str());
}

void GamePlayScene::AddObstacle(std::vector<AABB>& obstacles, const Vector3& min, const Vector3& max) {
    AABB newObstacle;
    newObstacle.min = min;
    newObstacle.max = max;
    obstacles.push_back(newObstacle);
}

void GamePlayScene::CreateObstacleColliders() {
    // 既存のコライダーをクリア
    for (auto* collider : obstacleColliders_) {
        if (collider && !collider->GetParentObject()) {
            delete collider;
        }
    }
    obstacleColliders_.clear();

    // 追加したAABBごとにBoxColliderを作成
    for (const auto& obstacle : allObstacles_) {
        // 中心点を計算
        Vector3 center = {
            (obstacle.min.x + obstacle.max.x) * 0.5f,
            (obstacle.min.y + obstacle.max.y) * 0.5f,
            (obstacle.min.z + obstacle.max.z) * 0.5f
        };

        // サイズを計算（半分のサイズ）
        Vector3 size = {
            (obstacle.max.x - obstacle.min.x) * 0.5f,
            (obstacle.max.y - obstacle.min.y) * 0.5f,
            (obstacle.max.z - obstacle.min.z) * 0.5f
        };

        // BoxColliderを作成
        BoxCollider* collider = new BoxCollider();
        collider->SetSize(size);

        // コライダーをステージオブジェクトに追加
        // 注意: 中心点との相対オフセットを計算
        Vector3 stagePos = stageObject_->GetPosition();
        Vector3 offset = {
            center.x - stagePos.x,
            center.y - stagePos.y,
            center.z - stagePos.z
        };
        collider->SetOffset(offset);
        stageObject_->AddCollider(collider);

        // 管理リストに追加
        obstacleColliders_.push_back(collider);
    }

    OutputDebugStringA(("GamePlayScene: Created " + std::to_string(obstacleColliders_.size()) + " colliders\n").c_str());
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // マウス入力処理
    ProcessMouseInput();

    // プレイヤー移動
    MovePlayer();

    // 視点切り替え
    if (input_->TriggerKey(DIK_V)) {
        thirdPersonCamera_ = !thirdPersonCamera_;
    }

    // カメラ追従速度調整
    if (input_->PushKey(DIK_UP) && cameraFollowSpeed_ < 1.0f) {
        cameraFollowSpeed_ += 0.01f;
    }
    if (input_->PushKey(DIK_DOWN) && cameraFollowSpeed_ > 0.01f) {
        cameraFollowSpeed_ -= 0.01f;
    }

    // カメラ距離調整
    if (input_->PushKey(DIK_PGUP) && cameraDistance_ > 2.0f) {
        cameraDistance_ -= 0.1f;
    }
    if (input_->PushKey(DIK_PGDN) && cameraDistance_ < 20.0f) {
        cameraDistance_ += 0.1f;
    }

    // カメラ更新
    UpdateCamera();

    // 物理アップデート（時間は1/60秒と仮定）
    const float deltaTime = 1.0f / 60.0f;

    // 各オブジェクトの物理更新
    playerObject_->PhysicsUpdate(deltaTime);
    stageObject_->PhysicsUpdate(deltaTime);

    // 普通の更新も行う
    playerObject_->Update();
    stageObject_->Update();

    // 衝突判定マネージャーの更新と衝突チェック
    CollisionManager::GetInstance()->Update();
    CollisionManager::GetInstance()->CheckAllCollisions();

    // 衝突フラグをリセット（次のフレームのために）
    isColliding_ = false;

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }

    // 1フレーム経過
    isFirstFrame_ = false;
}

void GamePlayScene::ProcessMouseInput() {
    // マウスの状態を取得
    DIMOUSESTATE mouseState = {};
    input_->GetMouseState(&mouseState);

    // マウス右ボタンの状態を更新
    bool currentRightPressed = (mouseState.rgbButtons[1] & 0x80) != 0;

    // マウス右ボタンが押されたらカーソルを非表示にする
    if (currentRightPressed && !isMouseRightPressed_) {
        ShowCursor(FALSE);
    }
    // マウス右ボタンが離されたらカーソルを表示する
    else if (!currentRightPressed && isMouseRightPressed_) {
        ShowCursor(TRUE);
    }

    isMouseRightPressed_ = currentRightPressed;

    // マウス右ボタンが押されている場合のみカメラ操作を有効化
    if (isMouseRightPressed_) {
        // 現在のマウス座標を取得
        POINT mousePos;
        GetCursorPos(&mousePos);

        // ウィンドウのクライアント座標に変換
        HWND hWnd = winApp_->GetHwnd();
        ScreenToClient(hWnd, &mousePos);

        // 初回フレームはスキップ（前回値がないため）
        if (!isFirstFrame_) {
            // マウスの移動量を計算
            int deltaX = mousePos.x - prevMouseX_;
            int deltaY = mousePos.y - prevMouseY_;

            // カメラの回転角度を更新
            cameraYaw_ += deltaX * mouseSensitivity_;
            cameraPitch_ -= deltaY * mouseSensitivity_;

            // ピッチ角の制限（真上や真下を向きすぎないように）
            const float maxPitch = 1.5f; // 約85度
            cameraPitch_ = (cameraPitch_ > maxPitch) ? maxPitch : cameraPitch_;
            cameraPitch_ = (cameraPitch_ < -maxPitch) ? -maxPitch : cameraPitch_;
        }

        // マウス位置を画面中央に戻す
        POINT centerPos = { WinApp::kClientWidth / 2, WinApp::kClientHeight / 2 };
        ClientToScreen(hWnd, &centerPos);
        SetCursorPos(centerPos.x, centerPos.y);

        // 画面中央の座標をクライアント座標に変換して記録
        ScreenToClient(hWnd, &centerPos);
        prevMouseX_ = centerPos.x;
        prevMouseY_ = centerPos.y;
    }
    else {
        // マウス座標を現在値で更新（動いていないものとして扱う）
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(winApp_->GetHwnd(), &mousePos);
        prevMouseX_ = mousePos.x;
        prevMouseY_ = mousePos.y;
    }
}

void GamePlayScene::MovePlayer() {
    // プレイヤーの位置を取得
    Vector3 playerPos = playerObject_->GetPosition();

    // カメラの向きに基づいた移動方向の計算
    float frontX = std::sin(cameraYaw_);
    float frontZ = std::cos(cameraYaw_);

    // 右方向ベクトルの計算（カメラの向きに対して垂直）
    float rightX = std::sin(cameraYaw_ + 3.14159f / 2.0f);
    float rightZ = std::cos(cameraYaw_ + 3.14159f / 2.0f);

    // 移動ベクトルの初期化
    float moveX = 0.0f;
    float moveZ = 0.0f;

    // WASD入力に応じた移動方向の決定
    if (input_->PushKey(DIK_W)) {
        moveX += frontX;
        moveZ += frontZ;
    }
    if (input_->PushKey(DIK_S)) {
        moveX -= frontX;
        moveZ -= frontZ;
    }
    if (input_->PushKey(DIK_A)) {
        moveX -= rightX;
        moveZ -= rightZ;
    }
    if (input_->PushKey(DIK_D)) {
        moveX += rightX;
        moveZ += rightZ;
    }

    // 移動ベクトルの正規化（斜め移動時に速度が速くならないように）
    float moveLength = std::sqrt(moveX * moveX + moveZ * moveZ);
    if (moveLength > 0.001f) {
        moveX /= moveLength;
        moveZ /= moveLength;
    }

    // 移動速度の調整
    if (input_->PushKey(DIK_LSHIFT)) {
        moveSpeed_ = 0.3f; // 速く
    }
    else {
        moveSpeed_ = 0.15f; // 通常速度
    }

    // 物理ボディがある場合は力を加える形で移動
    PhysicsBody* physicsBody = playerObject_->GetPhysicsBody();
    if (physicsBody) {
        // 地上移動力を加える（XZ平面）
        Vector3 moveForce = { moveX * moveSpeed_ * 20.0f, 0.0f, moveZ * moveSpeed_ * 20.0f };
        physicsBody->AddForce(moveForce);

        // ジャンプ処理
        if (input_->TriggerKey(DIK_SPACE) && !isJumping_) {
            // 上向きの力を加える
            Vector3 jumpForce = { 0.0f, 15.0f, 0.0f };
            physicsBody->AddForce(jumpForce);
            isJumping_ = true;
        }

        // 現在の速度を取得して減衰させる（摩擦の代わり）
        Vector3 velocity = physicsBody->GetVelocity();
        Vector3 dampedVelocity = {
            velocity.x * 0.9f,  // XZ平面の速度を減衰
            velocity.y,         // Y方向（重力）はそのまま
            velocity.z * 0.9f
        };
        physicsBody->SetVelocity(dampedVelocity);
    }
    else {
        // 物理ボディがない場合は直接位置を変更（従来の方法）
        playerPos.x += moveX * moveSpeed_;
        playerPos.z += moveZ * moveSpeed_;

        // ジャンプと落下の簡易実装
        if (input_->TriggerKey(DIK_SPACE) && !isJumping_) {
            verticalVelocity_ = 0.4f;  // 初速度
            isJumping_ = true;
        }

        // 重力効果
        verticalVelocity_ -= 0.02f;  // 重力加速度
        playerPos.y += verticalVelocity_;

        // 地面判定（簡易版）
        if (playerPos.y < 0.0f) {
            playerPos.y = 0.0f;
            verticalVelocity_ = 0.0f;
            isJumping_ = false;
        }

        playerObject_->SetPosition(playerPos);
    }

    // プレイヤーの向きをカメラの向きに合わせる（移動している場合のみ）
    if (moveLength > 0.001f) {
        playerObject_->SetRotation({ 0.0f, cameraYaw_, 0.0f });
    }
}

void GamePlayScene::UpdateCamera() {
    if (!camera_ || !playerObject_) return;

    // プレイヤーの位置を取得
    Vector3 playerPos = playerObject_->GetPosition();

    // 目標カメラ位置を計算
    Vector3 targetCameraPos;

    if (thirdPersonCamera_) {
        // 三人称視点 - カメラの角度に基づいた位置計算
        float camX = playerPos.x - std::sin(cameraYaw_) * std::cos(cameraPitch_) * cameraDistance_;
        float camY = playerPos.y + std::sin(cameraPitch_) * cameraDistance_ + cameraHeight_;
        float camZ = playerPos.z - std::cos(cameraYaw_) * std::cos(cameraPitch_) * cameraDistance_;

        targetCameraPos = { camX, camY, camZ };
    }
    else {
        // 一人称視点 - プレイヤーの目の位置
        targetCameraPos = {
            playerPos.x,
            playerPos.y + 2.0f, // 目の高さ
            playerPos.z
        };
    }

    // 現在のカメラ位置を取得
    Vector3 currentCameraPos = camera_->GetTranslate();

    // 線形補間で滑らかに移動
    Vector3 newCameraPos;
    newCameraPos.x = currentCameraPos.x + (targetCameraPos.x - currentCameraPos.x) * cameraFollowSpeed_;
    newCameraPos.y = currentCameraPos.y + (targetCameraPos.y - currentCameraPos.y) * cameraFollowSpeed_;
    newCameraPos.z = currentCameraPos.z + (targetCameraPos.z - currentCameraPos.z) * cameraFollowSpeed_;

    // カメラの位置を更新
    camera_->SetTranslate(newCameraPos);

    // 重要: カメラの回転順序を修正
    // Y軸回転（ヨー）を先に適用し、次にX軸回転（ピッチ）を適用する
    Vector3 rotation = { cameraPitch_, cameraYaw_, 0.0f };
    camera_->SetRotate(rotation);

    // カメラの更新を適用
    camera_->Update();
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // SRVヒープを描画前に設定
    if (srvManager_) {
        srvManager_->PreDraw();
    }

    // オブジェクトの描画
    playerObject_->Draw();
    stageObject_->Draw();

    // ImGuiで操作説明と衝突状態を表示
    ImGui::Begin("衝突テスト");
    ImGui::Text("操作方法:");
    ImGui::Text("WASD - 移動");
    ImGui::Text("スペース - ジャンプ");
    ImGui::Text("Shift - 加速");
    ImGui::Text("V - カメラ視点切替");
    ImGui::Text("右クリック - カメラ回転");
    ImGui::Text("PgUp/PgDn - カメラ距離調整");
    ImGui::Text("↑↓ - カメラ追従速度調整");
    ImGui::Text("ESC - タイトルに戻る");
    ImGui::Separator();
    ImGui::Text("衝突状態: %s", isColliding_ ? "衝突中" : "衝突なし");
    ImGui::Text("プレイヤー位置: (%.1f, %.1f, %.1f)",
        playerObject_->GetPosition().x,
        playerObject_->GetPosition().y,
        playerObject_->GetPosition().z);
    ImGui::Text("ステージ情報:");
    ImGui::Text("障害物数: %zu", allObstacles_.size());
    ImGui::Text("衝突判定数: %zu", obstacleColliders_.size());
    ImGui::Text("カメラ設定:");
    ImGui::Text("視点: %s", thirdPersonCamera_ ? "三人称" : "一人称");
    ImGui::SliderFloat("カメラ距離", &cameraDistance_, 2.0f, 20.0f);
    ImGui::SliderFloat("追従速度", &cameraFollowSpeed_, 0.01f, 1.0f);
    ImGui::SliderFloat("マウス感度", &mouseSensitivity_, 0.0005f, 0.005f);
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // マウスカーソルを表示状態に戻す
    ShowCursor(TRUE);

    // OBJファイルストリームが開いていれば閉じる
    if (objFileStream_.is_open()) {
        objFileStream_.close();
    }

    // 特に何もする必要がない
    // コライダーはObject3dデストラクタで自動的に削除される
    OutputDebugStringA("GamePlayScene: Finalized collision test\n");
}

void GamePlayScene::OnPlayerCollision(const CollisionInfo& info) {
    // 衝突情報の処理
    isColliding_ = true;

    // 衝突時のめり込み解消（簡易版）
    Vector3 playerPos = playerObject_->GetPosition();
    playerPos.x += info.normal.x * info.penetration * 0.5f;
    playerPos.y += info.normal.y * info.penetration * 0.5f;
    playerPos.z += info.normal.z * info.penetration * 0.5f;
    playerObject_->SetPosition(playerPos);

    // 地面との衝突で上向きの力が働いている場合はジャンプ状態をリセット
    if (info.normal.y > 0.5f) {
        isJumping_ = false;
    }

    // 衝突情報のデバッグ出力
    char debugInfo[256];
    sprintf_s(debugInfo, "Collision: Pos=(%.2f, %.2f, %.2f), Normal=(%.2f, %.2f, %.2f), Penetration=%.2f\n",
        info.collisionPoint.x, info.collisionPoint.y, info.collisionPoint.z,
        info.normal.x, info.normal.y, info.normal.z,
        info.penetration);
    OutputDebugStringA(debugInfo);
}