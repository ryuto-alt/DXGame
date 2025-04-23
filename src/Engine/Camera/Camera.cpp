#include "Camera.h"
#include "RenderingPipeline.h"

// 静的メンバ変数の実体化
Camera* Object3dCommon::defaultCamera_ = nullptr;

Camera::Camera() :
    fovY_(0.45f),
    aspectRatio_(16.0f / 9.0f),
    nearClip_(0.1f),
    farClip_(100.0f)
{
    // トランスフォームの初期設定
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, -5.0f };

    // 初期更新
    Update();
}

Camera::~Camera() {
    // デフォルトカメラが自分自身だった場合はnullptrにする
    if (Object3dCommon::GetDefaultCamera() == this) {
        Object3dCommon::SetDefaultCamera(nullptr);
    }
}

void Camera::Update() {
    // ワールド行列の計算
    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // ビュー行列の計算（ワールド行列の逆行列）
    viewMatrix_ = Inverse(worldMatrix_);

    // プロジェクション行列の計算
    projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);

    // ビュープロジェクション行列の計算
    viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

// セッター
void Camera::SetRotate(const Vector3& rotate) {
    transform_.rotate = rotate;
}

void Camera::SetTranslate(const Vector3& translate) {
    transform_.translate = translate;
}

void Camera::SetFovY(float fovY) {
    fovY_ = fovY;
}

void Camera::SetAspectRatio(float aspectRatio) {
    aspectRatio_ = aspectRatio;
}

void Camera::SetNearClip(float nearClip) {
    nearClip_ = nearClip;
}

void Camera::SetFarClip(float farClip) {
    farClip_ = farClip;
}

// ゲッター
const Matrix4x4& Camera::GetWorldMatrix() const {
    return worldMatrix_;
}

const Matrix4x4& Camera::GetViewMatrix() const {
    return viewMatrix_;
}

const Matrix4x4& Camera::GetProjectionMatrix() const {
    return projectionMatrix_;
}

const Matrix4x4& Camera::GetViewProjectionMatrix() const {
    return viewProjectionMatrix_;
}

const Vector3& Camera::GetRotate() const {
    return transform_.rotate;
}

const Vector3& Camera::GetTranslate() const {
    return transform_.translate;
}

float Camera::GetFovY() const {
    return fovY_;
}

float Camera::GetAspectRatio() const {
    return aspectRatio_;
}

float Camera::GetNearClip() const {
    return nearClip_;
}

float Camera::GetFarClip() const {
    return farClip_;
}

// デフォルトカメラの設定・取得
void Object3dCommon::SetDefaultCamera(Camera* camera) {
    defaultCamera_ = camera;
}

Camera* Object3dCommon::GetDefaultCamera() {
    return defaultCamera_;
}