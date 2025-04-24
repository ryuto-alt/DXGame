// src/Engine/Audio/AudioSource.h (拡張版)
#pragma once

#include <xaudio2.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// 前方宣言
class WaveFile;
class Mp3File;

// オーディオソースクラス（拡張版）
class AudioSource {
private:
    // ソースボイス
    IXAudio2SourceVoice* sourceVoice;

    // オーディオバッファ
    std::vector<BYTE> audioData;

    // オーディオフォーマット
    WAVEFORMATEX waveFormat;

    // 再生状態
    bool isPlaying;
    bool isPaused;
    bool isLooping;

    // ボリューム・ピッチ・パン
    float volume;
    float pitch;
    float pan;

    // ストリーミング再生関連
    bool isStreaming;
    std::string filePath;
    std::unique_ptr<WaveFile> streamingWaveFile;
    std::unique_ptr<Mp3File> streamingMp3File;
    static const UINT32 STREAMING_BUFFER_SIZE = 65536; // 64KB
    static const UINT32 STREAMING_BUFFER_COUNT = 3;
    BYTE streamingBuffers[STREAMING_BUFFER_COUNT][STREAMING_BUFFER_SIZE];
    UINT32 currentStreamingBuffer;
    bool isStreamingBufferSubmitted[STREAMING_BUFFER_COUNT];

    // フェード関連
    bool isFading;
    float fadeStartVolume;
    float fadeEndVolume;
    float fadeTime;
    float fadeElapsedTime;
    std::function<void()> fadeCompleteCallback;

    // ループポイント
    UINT32 loopStartSample;
    UINT32 loopEndSample;

    // サウンドグループ
    std::string groupName;

public:
    // コンストラクタ
    AudioSource();

    // デストラクタ
    virtual ~AudioSource();

    // WAVファイルからの初期化
    virtual bool Initialize(IXAudio2* xAudio2, WaveFile* waveFile);

    // MP3ファイルからの初期化
    virtual bool Initialize(IXAudio2* xAudio2, Mp3File* mp3File);

    // ストリーミングモードでの初期化（新規追加）
    virtual bool InitializeStreaming(IXAudio2* xAudio2, const std::string& filePath);

    // 再生
    virtual void Play(bool looping = false);

    // 停止
    virtual void Stop();

    // 一時停止
    virtual void Pause();

    // 再開
    virtual void Resume();

    // ボリューム設定（0.0f ～ 1.0f）
    virtual void SetVolume(float volume);
    virtual float GetVolume() const;

    // ピッチ設定（0.5f ～ 2.0f）（新規追加）
    virtual void SetPitch(float pitch);
    virtual float GetPitch() const;

    // パン設定（-1.0f ～ 1.0f）（新規追加）
    virtual void SetPan(float pan);
    virtual float GetPan() const;

    // フェード関連（新規追加）
    virtual void FadeIn(float duration, std::function<void()> completeCallback = nullptr);
    virtual void FadeOut(float duration, std::function<void()> completeCallback = nullptr);
    virtual void FadeTo(float targetVolume, float duration, std::function<void()> completeCallback = nullptr);
    virtual void UpdateFade(float deltaTime);
    virtual bool IsFading() const;

    // ループポイント設定（新規追加）
    virtual void SetLoopPoints(UINT32 startSample, UINT32 endSample);
    virtual UINT32 GetLoopStartSample() const;
    virtual UINT32 GetLoopEndSample() const;

    // サウンドグループ設定（新規追加）
    virtual void SetGroupName(const std::string& name);
    virtual const std::string& GetGroupName() const;

    // 再生位置関連（新規追加）
    virtual UINT32 GetCurrentPosition() const;
    virtual UINT32 GetTotalSamples() const;
    virtual float GetCurrentTime() const;
    virtual float GetTotalTime() const;
    virtual void SetPosition(UINT32 samplePosition);
    virtual void SetTime(float seconds);

    // 再生状態の取得
    virtual bool IsPlaying() const;
    virtual bool IsPaused() const;
    virtual bool IsLooping() const;
    virtual bool IsStreaming() const;

    // ソースボイスの取得
    IXAudio2SourceVoice* GetSourceVoice() const { return sourceVoice; }

    // バッファ管理（ストリーミング用）（新規追加）
    virtual bool SubmitBuffer(UINT32 bufferIndex);
    virtual void OnBufferEnd(UINT32 bufferIndex);
    virtual bool FillBuffer(UINT32 bufferIndex);

    // コールバックハンドラクラス
    class VoiceCallback : public IXAudio2VoiceCallback {
    public:
        // 参照するAudioSource
        AudioSource* audioSource;

        // コンストラクタ
        VoiceCallback() : audioSource(nullptr) {}

        // 再生終了時のコールバック
        void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override;

        // ストリーミングバッファのコールバック
        void STDMETHODCALLTYPE OnStreamEnd() override;
        void STDMETHODCALLTYPE OnBufferStart(void* pBufferContext) override;
        void STDMETHODCALLTYPE OnLoopEnd(void* pBufferContext) override;

        // 未使用のコールバックは空実装
        void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
        void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
        void STDMETHODCALLTYPE OnVoiceError(void* pBufferContext, HRESULT Error) override {}
    };

    // コールバックハンドラ
    VoiceCallback voiceCallback;

protected:
    // パンマトリックスの計算（新規追加）
    void CalculatePanMatrix(float pan, float* matrix, UINT32 channelCount);

    // ストリーミングの初期化（新規追加）
    bool InitializeStreamingWAV(IXAudio2* xAudio2, const std::string& filePath);
    bool InitializeStreamingMP3(IXAudio2* xAudio2, const std::string& filePath);

    // ストリーミングの更新（新規追加）
    void UpdateStreaming();
};