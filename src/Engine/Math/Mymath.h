#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <string>

//float Cot(float theta);

Matrix4x4 MakeIdentity4x4();
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 MakeRotateMatrix(const Vector3& rotate);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
Matrix4x4 Inverse(const Matrix4x4& m);
//Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
//Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearclip, float farclip);
//Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian);
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

struct Material {
    Vector4 color;
    int32_t enableLighting;
    float padding[3];
    Matrix4x4 uvTransform;
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};

// マテリアルデータ構造体の定義
struct MaterialData {
    std::string textureFilePath;  // テクスチャファイルパス
    Vector4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f };  // 環境光(Ka)
    Vector4 diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };  // 拡散反射光(Kd)
    Vector4 specular = { 0.0f, 0.0f, 0.0f, 1.0f }; // 鏡面反射光(Ks)
    float shininess = 0.0f;                      // 光沢度(Ns)
    float alpha = 1.0f;                          // 透明度(d)
};

struct ModelData {
    std::vector<VertexData>vertices;
    MaterialData material;
};