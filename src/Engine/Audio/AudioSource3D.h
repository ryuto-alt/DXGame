// src/Engine/Audio/AudioSource3D.h
#pragma once

#include "AudioSource.h"
#include "Vector3.h"
#include <xaudio2.h>
#include <x3daudio.h>
#include <memory>

// 3Dオーディオをサポートする拡張クラス
class AudioSource3D : public AudioSource {
private:
    // 3D位置情報
    Vector3 position_;
    Vector3 velocity_;

    // 3Dオーディオの設定
    float minDistance_;  // 減衰が始まる最小距離
    float maxDistance_;  // 最大聞こえる距離
    float innerAngle_;   // コーン内側の角度（ラジアン）
    float outerAngle_;   // コーン外側の角度（ラジアン）
    float outerGain_;    // コーン外側の減衰係数

    // 3Dオーディオ処理用
    X3DAUDIO_EMITTER emitter_;
    X3DAUDIO_DSP_SETTINGS dspSettings_;
    std::unique_ptr<float[]> matrixCoefficients_;
    std::unique_ptr<float[]> delayTimes_;
    bool isInitialized3D_;

public:
    // コンストラクタ
    AudioSource3D();

    // デストラクタ
    ~AudioSource3D() override;

    // 初期化（WAVファイル）
    bool Initialize(IXAudio2* xAudio2, WaveFile* waveFile);

    // 初期化（MP3ファイル）
    bool Initialize(IXAudio2* xAudio2, Mp3File* mp3File);

    // 3Dオーディオのセットアップ
    void Setup3DAudio(const X3DAUDIO_HANDLE& x3DAudioHandle, UINT32 channelCount);

    // 3Dオーディオの更新
    void Update3D(const X3DAUDIO_HANDLE& x3DAudioHandle, const X3DAUDIO_LISTENER& listener);

    // 位置の設定
    void SetPosition(const Vector3& position);
    const Vector3& GetPosition() const;

    // 速度の設定
    void SetVelocity(const Vector3& velocity);
    const Vector3& GetVelocity() const;

    // 減衰距離の設定
    void SetDistance(float minDistance, float maxDistance);
    float GetMinDistance() const;
    float GetMaxDistance() const;

    // 指向性の設定
    void SetConeAngles(float innerAngle, float outerAngle, float outerGain);
    float GetInnerAngle() const;
    float GetOuterAngle() const;
    float GetOuterGain() const;
};