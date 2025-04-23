// src/Engine/Graphics/Object3d.cpp
#include "Object3d.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Math.h"
#include "TextureManager.h"


Object3d::Object3d() : model_(nullptr), dxCommon_(nullptr), spriteCommon_(nullptr),
materialData_(nullptr), transformationMatrixData_(nullptr), directionalLightData_(nullptr),
camera_(nullptr) {
    // 初期値設定
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
}

Object3d::~Object3d() {}

void Object3d::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    assert(dxCommon);
    assert(spriteCommon);
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;

    // マテリアルリソースの作成
    materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));
    // マテリアルデータの書き込み
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = true;
    materialData_->uvTransform = MakeIdentity4x4();

    // 変換行列リソースの作成
    transformationMatrixResource_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
    // 変換行列データの書き込み
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    transformationMatrixData_->WVP = MakeIdentity4x4();
    transformationMatrixData_->World = MakeIdentity4x4();

    // ライトリソースの作成
    directionalLightResource_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
    // ライトデータの書き込み
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
    directionalLightData_->intensity = 1.0f;
}

void Object3d::SetModel(Model* model) {
    model_ = model;

    // モデルのマテリアル情報をシェーダーに設定
    if (model_ && materialData_) {
        const MaterialData& modelMaterial = model_->GetMaterial();

        // マテリアルデータをシェーダーのMaterial構造体に反映
        // シェーダーのcolor変数にdiffuse色を設定
        materialData_->color = modelMaterial.diffuse;

        // アルファ値も設定
        materialData_->color.w = modelMaterial.alpha;

        // モデルのテクスチャパスの確認
        std::string texturePath = model_->GetTextureFilePath();
        OutputDebugStringA(("Object3d::SetModel - Model texture path: " + texturePath + "\n").c_str());

        if (!texturePath.empty()) {
            // テクスチャが未ロードなら読み込む
            if (!TextureManager::GetInstance()->IsTextureExists(texturePath)) {
                OutputDebugStringA(("Object3d::SetModel - Loading texture: " + texturePath + "\n").c_str());
                TextureManager::GetInstance()->LoadTexture(texturePath);
            }
            else {
                OutputDebugStringA(("Object3d::SetModel - Texture already loaded: " + texturePath + "\n").c_str());
            }
        }
        else {
            OutputDebugStringA("Object3d::SetModel - No texture path provided by model\n");
        }

        // デバッグ情報
        OutputDebugStringA("Object3d::SetModel - Material information:\n");
        OutputDebugStringA(("  - Diffuse (RGBA): " +
            std::to_string(materialData_->color.x) + ", " +
            std::to_string(materialData_->color.y) + ", " +
            std::to_string(materialData_->color.z) + ", " +
            std::to_string(materialData_->color.w) + "\n").c_str());
        OutputDebugStringA(("  - Texture: " + (texturePath.empty() ? "None" : texturePath) + "\n").c_str());
    }
}

// 従来のUpdateメソッド（ビュー行列とプロジェクション行列を直接指定）
void Object3d::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
    assert(transformationMatrixData_);

    // ワールド行列の計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // WVP行列の計算
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

    // 行列の更新
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;
}

// カメラセッター
void Object3d::SetCamera(Camera* camera) {
    camera_ = camera;
}

// カメラゲッター
Camera* Object3d::GetCamera() const {
    return camera_;
}

// 新しいUpdateメソッド（カメラを使用）
void Object3d::Update() {
    assert(transformationMatrixData_);

    // カメラが設定されていない場合はデフォルトカメラを使用
    Camera* useCamera = camera_;
    if (!useCamera) {
        useCamera = Object3dCommon::GetDefaultCamera();
    }

    // カメラが有効かチェック
    assert(useCamera);

    // ワールド行列の計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // WVP行列の計算（カメラからビュープロジェクション行列を取得）
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, useCamera->GetViewProjectionMatrix());

    // 行列の更新
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = worldMatrix;
}

void Object3d::Draw() {
    assert(dxCommon_);
    assert(model_);

    // 共通描画設定
    spriteCommon_->CommonDraw();

    // モデルの頂点バッファをセット
    dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &model_->GetVBView());

    // マテリアルCBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // 変換行列CBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

    // テクスチャの場所を設定
    std::string texturePath = model_->GetTextureFilePath();
    OutputDebugStringA(("Object3d::Draw - Using texture path: " + texturePath + "\n").c_str());

    // テクスチャが空または存在しない場合の詳細なチェック
    if (texturePath.empty()) {
        OutputDebugStringA("Object3d::Draw - Texture path is empty, using default texture\n");
        TextureManager::GetInstance()->LoadDefaultTexture();
        texturePath = TextureManager::GetInstance()->GetDefaultTexturePath();
    }
    else if (!TextureManager::GetInstance()->IsTextureExists(texturePath)) {
        OutputDebugStringA(("Object3d::Draw - Texture does not exist in TextureManager: " + texturePath + "\n").c_str());

        // 試しにテクスチャを再ロードする
        bool loadSuccess = false;

        // ファイルの存在チェック
        DWORD fileAttributes = GetFileAttributesA(texturePath.c_str());
        if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
            // ファイルが存在する場合はロードを試みる
            OutputDebugStringA(("Object3d::Draw - File exists, trying to load texture: " + texturePath + "\n").c_str());
            TextureManager::GetInstance()->LoadTexture(texturePath);

            // 読み込みに成功したか確認
            if (TextureManager::GetInstance()->IsTextureExists(texturePath)) {
                OutputDebugStringA(("Object3d::Draw - Successfully loaded texture: " + texturePath + "\n").c_str());
                loadSuccess = true;
            }
        }

        // それでも失敗した場合はデフォルトテクスチャを使用
        if (!loadSuccess) {
            OutputDebugStringA("Object3d::Draw - Using default texture\n");
            TextureManager::GetInstance()->LoadDefaultTexture();
            texturePath = TextureManager::GetInstance()->GetDefaultTexturePath();
        }
    }
    else {
        OutputDebugStringA(("Object3d::Draw - Using valid texture: " + texturePath + "\n").c_str());
    }

    // テクスチャをセット（必ずテクスチャがセットされることを保証）
    dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,
        TextureManager::GetInstance()->GetSrvHandleGPU(texturePath));

    // ライトCBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

    // 描画
    dxCommon_->GetCommandList()->DrawInstanced(model_->GetVertexCount(), 1, 0, 0);
}