#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Mymath.h"

// カメラクラス - 3Dオブジェクトからカメラ機能を分離
class Camera {
public:
    // コンストラクタ・デストラクタ
    Camera();
    ~Camera();

    // 更新処理 - 毎フレーム行列を更新する
    void Update();

    // セッター
    void SetRotate(const Vector3& rotate);
    void SetTranslate(const Vector3& translate);
    void SetFovY(float fovY);
    void SetAspectRatio(float aspectRatio);
    void SetNearClip(float nearClip);
    void SetFarClip(float farClip);

    // ゲッター
    const Matrix4x4& GetWorldMatrix() const;
    const Matrix4x4& GetViewMatrix() const;
    const Matrix4x4& GetProjectionMatrix() const;
    const Matrix4x4& GetViewProjectionMatrix() const;
    const Vector3& GetRotate() const;
    const Vector3& GetTranslate() const;
    float GetFovY() const;
    float GetAspectRatio() const;
    float GetNearClip() const;
    float GetFarClip() const;

private:
    // ビュー行列関連データ
    Transform transform_;       // カメラのトランスフォーム
    Matrix4x4 worldMatrix_;     // カメラのワールド行列
    Matrix4x4 viewMatrix_;      // ビュー行列

    // プロジェクション行列関連データ
    Matrix4x4 projectionMatrix_; // プロジェクション行列
    float fovY_;                // 視野角
    float aspectRatio_;         // アスペクト比
    float nearClip_;            // ニアクリップ距離
    float farClip_;             // ファークリップ距離

    // 合成行列 - ビュー行列とプロジェクション行列の積
    Matrix4x4 viewProjectionMatrix_;
};

// 静的なデフォルトカメラの定義
class Object3dCommon {
public:
    // デフォルトカメラの設定・取得
    static void SetDefaultCamera(Camera* camera);
    static Camera* GetDefaultCamera();

private:
    // デフォルトカメラ
    static Camera* defaultCamera_;
};