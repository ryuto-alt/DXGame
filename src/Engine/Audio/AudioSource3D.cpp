// src/Engine/Audio/AudioSource3D.cpp
#include "AudioSource3D.h"
#include "WaveFile.h"
#include "Mp3File.h"
#include <cassert>

AudioSource3D::AudioSource3D()
    : position_({ 0.0f, 0.0f, 0.0f }),
    velocity_({ 0.0f, 0.0f, 0.0f }),
    minDistance_(1.0f),
    maxDistance_(10000.0f),
    innerAngle_(X3DAUDIO_PI / 4.0f),
    outerAngle_(X3DAUDIO_PI / 2.0f),
    outerGain_(0.0f),
    isInitialized3D_(false) {

    // エミッタの初期化
    ZeroMemory(&emitter_, sizeof(X3DAUDIO_EMITTER));
    // DSP設定の初期化
    ZeroMemory(&dspSettings_, sizeof(X3DAUDIO_DSP_SETTINGS));
}

AudioSource3D::~AudioSource3D() {
    // AudioSource基底クラスのデストラクタで音声関連のリソースを解放
}

bool AudioSource3D::Initialize(IXAudio2* xAudio2, WaveFile* waveFile) {
    // AudioSource基底クラスの初期化
    if (!AudioSource::Initialize(xAudio2, waveFile)) {
        return false;
    }

    return true; // 基本初期化成功
}

bool AudioSource3D::Initialize(IXAudio2* xAudio2, Mp3File* mp3File) {
    // AudioSource基底クラスの初期化
    if (!AudioSource::Initialize(xAudio2, mp3File)) {
        return false;
    }

    return true; // 基本初期化成功
}

void AudioSource3D::Setup3DAudio(const X3DAUDIO_HANDLE& x3DAudioHandle, UINT32 channelCount) {
    // エミッターの設定
    emitter_.ChannelCount = 1; // モノラルソースを想定
    emitter_.CurveDistanceScaler = 1.0f;
    emitter_.OrientFront = { 0.0f, 0.0f, 1.0f }; // デフォルトは前向き（Z+方向）
    emitter_.OrientTop = { 0.0f, 1.0f, 0.0f };   // デフォルトは上向き（Y+方向）
    emitter_.Position = { position_.x, position_.y, position_.z };
    emitter_.Velocity = { velocity_.x, velocity_.y, velocity_.z };

    // 減衰設定
    emitter_.InnerRadius = minDistance_;
    emitter_.OuterRadius = maxDistance_;

    // 指向性の設定
    emitter_.pCone = new X3DAUDIO_CONE();
    emitter_.pCone->InnerAngle = innerAngle_;
    emitter_.pCone->OuterAngle = outerAngle_;
    emitter_.pCone->InnerVolume = 1.0f;
    emitter_.pCone->OuterVolume = outerGain_;
    emitter_.pCone->InnerLPF = 0.0f;
    emitter_.pCone->OuterLPF = 0.0f;
    emitter_.pCone->InnerReverb = 0.0f;
    emitter_.pCone->OuterReverb = 1.0f;

    // DSP設定のメモリ確保
    matrixCoefficients_ = std::make_unique<float[]>(channelCount);
    delayTimes_ = std::make_unique<float[]>(channelCount);

    dspSettings_.SrcChannelCount = 1; // モノラルソース
    dspSettings_.DstChannelCount = channelCount;
    dspSettings_.pMatrixCoefficients = matrixCoefficients_.get();
    dspSettings_.pDelayTimes = delayTimes_.get();

    isInitialized3D_ = true;
}

void AudioSource3D::Update3D(const X3DAUDIO_HANDLE& x3DAudioHandle, const X3DAUDIO_LISTENER& listener) {
    if (!isInitialized3D_ || !IsPlaying()) {
        return;
    }

    // エミッタ情報を更新
    emitter_.Position = { position_.x, position_.y, position_.z };
    emitter_.Velocity = { velocity_.x, velocity_.y, velocity_.z };

    // 3Dオーディオ計算フラグ
    UINT32 flags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DELAY;

    // 3Dオーディオ計算の実行
    X3DAudioCalculate(
        x3DAudioHandle,
        &listener,
        &emitter_,
        flags,
        &dspSettings_
    );

    // 音量と遅延の適用
    if (GetSourceVoice()) {
        GetSourceVoice()->SetOutputMatrix(nullptr, 1, dspSettings_.DstChannelCount, dspSettings_.pMatrixCoefficients);
        GetSourceVoice()->SetOutputMatrix(nullptr, 1, dspSettings_.DstChannelCount, dspSettings_.pMatrixCoefficients);
    }
}

void AudioSource3D::SetPosition(const Vector3& position) {
    position_ = position;
}

const Vector3& AudioSource3D::GetPosition() const {
    return position_;
}

void AudioSource3D::SetVelocity(const Vector3& velocity) {
    velocity_ = velocity;
}

const Vector3& AudioSource3D::GetVelocity() const {
    return velocity_;
}

void AudioSource3D::SetDistance(float minDistance, float maxDistance) {
    minDistance_ = minDistance;
    maxDistance_ = maxDistance;

    if (isInitialized3D_) {
        emitter_.InnerRadius = minDistance_;
        emitter_.OuterRadius = maxDistance_;
    }
}

float AudioSource3D::GetMinDistance() const {
    return minDistance_;
}

float AudioSource3D::GetMaxDistance() const {
    return maxDistance_;
}

void AudioSource3D::SetConeAngles(float innerAngle, float outerAngle, float outerGain) {
    innerAngle_ = innerAngle;
    outerAngle_ = outerAngle;
    outerGain_ = outerGain;

    if (isInitialized3D_ && emitter_.pCone) {
        emitter_.pCone->InnerAngle = innerAngle_;
        emitter_.pCone->OuterAngle = outerAngle_;
        emitter_.pCone->OuterVolume = outerGain_;
    }
}

float AudioSource3D::GetInnerAngle() const {
    return innerAngle_;
}

float AudioSource3D::GetOuterAngle() const {
    return outerAngle_;
}

float AudioSource3D::GetOuterGain() const {
    return outerGain_;
}