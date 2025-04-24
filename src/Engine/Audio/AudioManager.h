// src/Engine/Audio/AudioManager.h (拡張版)
#pragma once

#include <xaudio2.h>
#include <x3daudio.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl.h>
#include "Vector3.h"

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")

// 前方宣言
class AudioSource;
class AudioSource3D;
class AudioBus;
class WaveFile;
class Mp3File;
class AudioEvent;

// オーディオファイル形式
enum class AudioFileFormat {
    WAV,
    MP3,
    Unknown
};

// 拡張版オーディオマネージャー
class AudioManager {
private:
    // シングルトンインスタンス
    static AudioManager* instance;

    // XAudio2インターフェイス
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

    // マスターボイス
    IXAudio2MasteringVoice* masteringVoice;

    // 3Dオーディオ関連
    X3DAUDIO_HANDLE x3dAudioHandle;
    X3DAUDIO_LISTENER listener;
    bool is3DAudioInitialized;

    // オーディオソースのマップ
    std::unordered_map<std::string, std::unique_ptr<AudioSource>> audioSources;

    // 3Dオーディオソースのマップ
    std::unordered_map<std::string, std::unique_ptr<AudioSource3D>> audioSources3D;

    // オーディオバスのマップ
    std::unordered_map<std::string, std::unique_ptr<AudioBus>> audioBuses;

    // オーディオイベントのマップ
    std::unordered_map<std::string, std::unique_ptr<AudioEvent>> audioEvents;

    // 再生中のソースリスト
    std::vector<AudioSource*> playingSources;

    // コンストラクタ（シングルトン）
    AudioManager();

    // デストラクタ
    ~AudioManager();

    // コピー禁止
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Media Foundation初期化フラグ
    bool mfInitialized;

    // グローバルボリューム
    float masterVolume;

    // グローバル3D設定
    float dopplerFactor;
    float speedOfSound;

    // 一時停止フラグ
    bool isPaused;

    // マイクロ秒精度のタイマー
    LARGE_INTEGER performanceFrequency;
    LARGE_INTEGER lastUpdateTime;

    // ファイル形式の判定
    AudioFileFormat GetAudioFileFormat(const std::string& filePath);

public:
    // シングルトンインスタンスの取得
    static AudioManager* GetInstance();

    // 初期化
    void Initialize();

    // 終了処理
    void Finalize();

    // 更新処理
    void Update();

    // WAVファイルの読み込み
    bool LoadWAV(const std::string& name, const std::string& filePath);

    // MP3ファイルの読み込み
    bool LoadMP3(const std::string& name, const std::string& filePath);

    // オーディオファイルの読み込み（拡張子による自動判別）
    bool LoadAudioFile(const std::string& name, const std::string& filePath);

    // ストリーミングモードでの読み込み（新規追加）
    bool LoadStreamingAudioFile(const std::string& name, const std::string& filePath);

    // 3Dオーディオソースの作成（新規追加）
    bool Create3DAudioSource(const std::string& name, const std::string& filePath);

    // オーディオバスの作成（新規追加）
    bool CreateAudioBus(const std::string& name);

    // オーディオイベントの作成（新規追加）
    bool CreateSimpleEvent(const std::string& name, const std::string& filePath, const AudioEventParams& params = {});
    bool CreateRandomEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params = {});
    bool CreateSequentialEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params = {});
    bool CreateLayeredEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params = {});

    // 音声の再生
    void Play(const std::string& name, bool looping = false);

    // 音声の停止
    void Stop(const std::string& name);

    // 音声の一時停止
    void Pause(const std::string& name);

    // 音声の再開
    void Resume(const std::string& name);

    // ボリューム設定（0.0f ～ 1.0f）
    void SetVolume(const std::string& name, float volume);

    // ピッチ設定（0.5f ～ 2.0f）（新規追加）
    void SetPitch(const std::string& name, float pitch);

    // パン設定（-1.0f ～ 1.0f）（新規追加）
    void SetPan(const std::string& name, float pan);

    // フェードイン（新規追加）
    void FadeIn(const std::string& name, float duration, std::function<void()> completeCallback = nullptr);

    // フェードアウト（新規追加）
    void FadeOut(const std::string& name, float duration, std::function<void()> completeCallback = nullptr);

    // 3D位置の設定（新規追加）
    void SetPosition(const std::string& name, const Vector3& position);

    // 3D速度の設定（新規追加）
    void SetVelocity(const std::string& name, const Vector3& velocity);

    // リスナーの設定（新規追加）
    void SetListenerPosition(const Vector3& position);
    void SetListenerOrientation(const Vector3& forward, const Vector3& up);
    void SetListenerVelocity(const Vector3& velocity);

    // グローバル設定（新規追加）
    void SetDopplerFactor(float factor);
    void SetSpeedOfSound(float speed);

    // マスターボリューム設定（0.0f ～ 1.0f）
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;

    // 全体の一時停止/再開（新規追加）
    void PauseAll();
    void ResumeAll();

    // グループ操作（新規追加）
    void PlayGroup(const std::string& groupName, bool looping = false);
    void StopGroup(const std::string& groupName);
    void PauseGroup(const std::string& groupName);
    void ResumeGroup(const std::string& groupName);
    void SetGroupVolume(const std::string& groupName, float volume);

    // バス操作（新規追加）
    void PlayBus(const std::string& busName, bool looping = false);
    void StopBus(const std::string& busName);
    void PauseBus(const std::string& busName);
    void ResumeBus(const std::string& busName);
    void SetBusVolume(const std::string& busName, float volume);

    // イベント操作（新規追加）
    void PlayEvent(const std::string& eventName);
    void StopEvent(const std::string& eventName);
    void PauseEvent(const std::string& eventName);
    void ResumeEvent(const std::string& eventName);
    void SetEventVolume(const std::string& eventName, float volume);

    // 再生中かどうか
    bool IsPlaying(const std::string& name);

    // オーディオソースの取得
    AudioSource* GetAudioSource(const std::string& name);

    // 3Dオーディオソースの取得（新規追加）
    AudioSource3D* GetAudioSource3D(const std::string& name);

    // オーディオバスの取得（新規追加）
    AudioBus* GetAudioBus(const std::string& name);

    // オーディオイベントの取得（新規追加）
    AudioEvent* GetAudioEvent(const std::string& name);

    // XAudio2インターフェイスの取得
    IXAudio2* GetXAudio2() { return xAudio2.Get(); }

    // X3DAudioハンドルの取得（新規追加）
    const X3DAUDIO_HANDLE& GetX3DAudioHandle() const { return x3dAudioHandle; }

    // リスナーの取得（新規追加）
    const X3DAUDIO_LISTENER& GetListener() const { return listener; }

private:
    // 3Dオーディオの初期化（新規追加）
    bool Initialize3DAudio();

    // オーディオファイルの読み込み実装
    bool LoadAudioFileImpl(const std::string& name, const std::string& filePath, AudioFileFormat format, bool streaming = false);

    // デルタ時間の計算
    float CalculateDeltaTime();
};

// オーディオイベントのパラメータ構造体（前方宣言用）
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
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    Vector3 velocity = { 0.0f, 0.0f, 0.0f };
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