// src/Engine/Audio/AudioEvent.h
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <random>
#include "Vector3.h"

// 前方宣言
class AudioSource;
class AudioSource3D;
class AudioManager;

// オーディオイベントの種類
enum class AudioEventType {
    Simple,     // 単一の音声を再生
    Random,     // ランダムに選択された音声を再生
    Sequential, // 順番に音声を再生
    Layered     // 複数の音声を同時に再生（レイヤー）
};

// オーディオイベントのパラメータ
struct AudioEventParams {
    // 基本パラメータ
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;
    bool loop = false;
    float fadeInTime = 0.0f;
    float fadeOutTime = 0.0f;
    std::string group = ""; // サウンドグループ

    // 3Dオーディオパラメータ
    bool is3D = false;
    Vector3 position;
    Vector3 velocity;
    float minDistance = 1.0f;
    float maxDistance = 10000.0f;

    // ランダム再生パラメータ
    float volumeVariation = 0.0f;
    float pitchVariation = 0.0f;

    // その他
    float delay = 0.0f;         // 再生開始までの遅延時間（秒）
    float stopTime = -1.0f;     // 再生停止時間（-1.0fは自動停止なし）
    bool ignorePause = false;   // ポーズ時も再生を続けるか
};

// オーディオイベントクラス
class AudioEvent {
protected:
    // イベント名
    std::string name_;

    // イベントタイプ
    AudioEventType type_;

    // イベントパラメータ
    AudioEventParams params_;

    // オーディオマネージャーへの参照
    AudioManager* audioManager_;

    // 再生状態
    bool isPlaying_;
    bool isPaused_;

    // 経過時間
    float elapsedTime_;

    // 遅延再生用タイマー
    float delayTimer_;

    // 停止タイマー
    float stopTimer_;

    // 完了コールバック
    std::function<void()> completeCallback_;

    // 乱数生成器
    static std::mt19937 randomEngine_;

public:
    // コンストラクタ
    AudioEvent(const std::string& name, AudioEventType type, const AudioEventParams& params, AudioManager* audioManager);

    // デストラクタ
    virtual ~AudioEvent();

    // 再生
    virtual void Play();

    // 停止
    virtual void Stop();

    // 一時停止
    virtual void Pause();

    // 再開
    virtual void Resume();

    // 更新
    virtual void Update(float deltaTime);

    // 再生中かどうか
    bool IsPlaying() const;

    // 一時停止中かどうか
    bool IsPaused() const;

    // 位置の設定（3Dサウンド用）
    virtual void SetPosition(const Vector3& position);

    // 速度の設定（3Dサウンド用）
    virtual void SetVelocity(const Vector3& velocity);

    // ボリュームの設定
    virtual void SetVolume(float volume);

    // ピッチの設定
    virtual void SetPitch(float pitch);

    // パンの設定
    virtual void SetPan(float pan);

    // 完了コールバックの設定
    void SetCompleteCallback(std::function<void()> callback);

    // 名前の取得
    const std::string& GetName() const;

    // パラメータの取得
    const AudioEventParams& GetParams() const;

protected:
    // 実際のオーディオソース作成・再生処理（サブクラスで実装）
    virtual void PlayInternal() = 0;

    // 実際のオーディオソース停止処理（サブクラスで実装）
    virtual void StopInternal() = 0;

    // クリーンアップ処理
    virtual void Cleanup();

    // パラメータにバリエーションを適用
    void ApplyVariation(AudioEventParams& params);
};

// 単一の音声を再生するオーディオイベント
class SimpleAudioEvent : public AudioEvent {
private:
    // 音声ファイルパス
    std::string filePath_;

    // オーディオソース
    AudioSource* audioSource_;
    AudioSource3D* audioSource3D_;

public:
    // コンストラクタ
    SimpleAudioEvent(const std::string& name, const std::string& filePath,
        const AudioEventParams& params, AudioManager* audioManager);

    // デストラクタ
    ~SimpleAudioEvent() override;

    // 位置の設定（3Dサウンド用）
    void SetPosition(const Vector3& position) override;

    // 速度の設定（3Dサウンド用）
    void SetVelocity(const Vector3& velocity) override;

    // ボリュームの設定
    void SetVolume(float volume) override;

    // ピッチの設定
    void SetPitch(float pitch) override;

    // パンの設定
    void SetPan(float pan) override;

protected:
    // 実際のオーディオソース作成・再生処理
    void PlayInternal() override;

    // 実際のオーディオソース停止処理
    void StopInternal() override;

    // クリーンアップ処理
    void Cleanup() override;
};

// ランダムに選択された音声を再生するオーディオイベント
class RandomAudioEvent : public AudioEvent {
private:
    // 音声ファイルパスのリスト
    std::vector<std::string> filePaths_;

    // 現在のオーディオソース
    AudioSource* currentAudioSource_;
    AudioSource3D* currentAudioSource3D_;

    // 現在の選択インデックス
    int currentIndex_;

public:
    // コンストラクタ
    RandomAudioEvent(const std::string& name, const std::vector<std::string>& filePaths,
        const AudioEventParams& params, AudioManager* audioManager);

    // デストラクタ
    ~RandomAudioEvent() override;

    // 位置の設定（3Dサウンド用）
    void SetPosition(const Vector3& position) override;

    // 速度の設定（3Dサウンド用）
    void SetVelocity(const Vector3& velocity) override;

    // ボリュームの設定
    void SetVolume(float volume) override;

    // ピッチの設定
    void SetPitch(float pitch) override;

    // パンの設定
    void SetPan(float pan) override;

protected:
    // 実際のオーディオソース作成・再生処理
    void PlayInternal() override;

    // 実際のオーディオソース停止処理
    void StopInternal() override;

    // クリーンアップ処理
    void Cleanup() override;

private:
    // ランダムに音声を選択
    int SelectRandomIndex();
};

// 順番に音声を再生するオーディオイベント
class SequentialAudioEvent : public AudioEvent {
private:
    // 音声ファイルパスのリスト
    std::vector<std::string> filePaths_;

    // 現在のオーディオソース
    AudioSource* currentAudioSource_;
    AudioSource3D* currentAudioSource3D_;

    // 現在の選択インデックス
    int currentIndex_;

public:
    // コンストラクタ
    SequentialAudioEvent(const std::string& name, const std::vector<std::string>& filePaths,
        const AudioEventParams& params, AudioManager* audioManager);

    // デストラクタ
    ~SequentialAudioEvent() override;

    // 位置の設定（3Dサウンド用）
    void SetPosition(const Vector3& position) override;

    // 速度の設定（3Dサウンド用）
    void SetVelocity(const Vector3& velocity) override;

    // ボリュームの設定
    void SetVolume(float volume) override;

    // ピッチの設定
    void SetPitch(float pitch) override;

    // パンの設定
    void SetPan(float pan) override;

protected:
    // 実際のオーディオソース作成・再生処理
    void PlayInternal() override;

    // 実際のオーディオソース停止処理
    void StopInternal() override;

    // クリーンアップ処理
    void Cleanup() override;

    // 次の音声に進む
    void AdvanceToNextSound();
};

// 複数の音声を同時に再生するオーディオイベント
class LayeredAudioEvent : public AudioEvent {
private:
    // 音声ファイルパスのリスト
    std::vector<std::string> filePaths_;

    // オーディオソースのリスト
    std::vector<AudioSource*> audioSources_;
    std::vector<AudioSource3D*> audioSources3D_;

public:
    // コンストラクタ
    LayeredAudioEvent(const std::string& name, const std::vector<std::string>& filePaths,
        const AudioEventParams& params, AudioManager* audioManager);

    // デストラクタ
    ~LayeredAudioEvent() override;

    // 位置の設定（3Dサウンド用）
    void SetPosition(const Vector3& position) override;

    // 速度の設定（3Dサウンド用）
    void SetVelocity(const Vector3& velocity) override;

    // ボリュームの設定
    void SetVolume(float volume) override;

    // ピッチの設定
    void SetPitch(float pitch) override;

    // パンの設定
    void SetPan(float pan) override;

protected:
    // 実際のオーディオソース作成・再生処理
    void PlayInternal() override;

    // 実際のオーディオソース停止処理
    void StopInternal() override;

    // クリーンアップ処理
    void Cleanup() override;
};