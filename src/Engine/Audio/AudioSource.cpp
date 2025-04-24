// src/Engine/Audio/AudioSource.cpp (拡張版)
#include "AudioSource.h"
#include "WaveFile.h"
#include "Mp3File.h"
#include <cassert>
#include <algorithm>

AudioSource::AudioSource()
    : sourceVoice(nullptr),
    isPlaying(false),
    isPaused(false),
    isLooping(false),
    volume(1.0f),
    pitch(1.0f),
    pan(0.0f),
    isStreaming(false),
    currentStreamingBuffer(0),
    isFading(false),
    fadeStartVolume(0.0f),
    fadeEndVolume(0.0f),
    fadeTime(0.0f),
    fadeElapsedTime(0.0f),
    loopStartSample(0),
    loopEndSample(0) {

    // コールバックハンドラの設定
    voiceCallback.audioSource = this;

    // ストリーミングバッファの初期化
    for (UINT32 i = 0; i < STREAMING_BUFFER_COUNT; i++) {
        isStreamingBufferSubmitted[i] = false;
    }
}

AudioSource::~AudioSource() {
    // 再生中なら停止
    if (sourceVoice) {
        sourceVoice->Stop();
        sourceVoice->FlushSourceBuffers();
        sourceVoice->DestroyVoice();
        sourceVoice = nullptr;
    }
}

bool AudioSource::Initialize(IXAudio2* xAudio2, WaveFile* waveFile) {
    assert(xAudio2);
    assert(waveFile);

    // WAVEフォーマットを取得
    waveFormat = waveFile->GetWaveFormat();

    // オーディオデータを取得
    audioData = waveFile->GetAudioData();

    // ソースボイスの作成
    HRESULT hr = xAudio2->CreateSourceVoice(
        &sourceVoice,
        &waveFormat,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        &voiceCallback);

    // ループポイントの初期設定（全範囲）
    loopStartSample = 0;
    loopEndSample = static_cast<UINT32>(audioData.size() / waveFormat.nBlockAlign);

    return SUCCEEDED(hr);
}

bool AudioSource::Initialize(IXAudio2* xAudio2, Mp3File* mp3File) {
    assert(xAudio2);
    assert(mp3File);

    // WAVEフォーマットを取得
    waveFormat = mp3File->GetWaveFormat();

    // オーディオデータを取得
    audioData = mp3File->GetAudioData();

    // ソースボイスの作成
    HRESULT hr = xAudio2->CreateSourceVoice(
        &sourceVoice,
        &waveFormat,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        &voiceCallback);

    // ループポイントの初期設定（全範囲）
    loopStartSample = 0;
    loopEndSample = static_cast<UINT32>(audioData.size() / waveFormat.nBlockAlign);

    return SUCCEEDED(hr);
}

bool AudioSource::InitializeStreaming(IXAudio2* xAudio2, const std::string& filePath) {
    // ファイルの拡張子からフォーマットを判定
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    isStreaming = true;
    this->filePath = filePath;

    if (extension == "wav") {
        return InitializeStreamingWAV(xAudio2, filePath);
    }
    else if (extension == "mp3") {
        return InitializeStreamingMP3(xAudio2, filePath);
    }
    else {
        OutputDebugStringA(("AudioSource: Unsupported file format - " + extension + "\n").c_str());
        return false;
    }
}

bool AudioSource::InitializeStreamingWAV(IXAudio2* xAudio2, const std::string& filePath) {
    // WAVファイルのストリーミング初期化
    streamingWaveFile = std::make_unique<WaveFile>();
    if (!streamingWaveFile->Load(filePath)) {
        return false;
    }

    // WAVEフォーマットを取得
    waveFormat = streamingWaveFile->GetWaveFormat();

    // ソースボイスの作成
    HRESULT hr = xAudio2->CreateSourceVoice(
        &sourceVoice,
        &waveFormat,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        &voiceCallback);

    if (FAILED(hr)) {
        return false;
    }

    // バッファを初期化
    for (UINT32 i = 0; i < STREAMING_BUFFER_COUNT; i++) {
        isStreamingBufferSubmitted[i] = false;
    }

    return true;
}

bool AudioSource::InitializeStreamingMP3(IXAudio2* xAudio2, const std::string& filePath) {
    // MP3ファイルのストリーミング初期化
    streamingMp3File = std::make_unique<Mp3File>();
    if (!streamingMp3File->Load(filePath)) {
        return false;
    }

    // WAVEフォーマットを取得
    waveFormat = streamingMp3File->GetWaveFormat();

    // ソースボイスの作成
    HRESULT hr = xAudio2->CreateSourceVoice(
        &sourceVoice,
        &waveFormat,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        &voiceCallback);

    if (FAILED(hr)) {
        return false;
    }

    // バッファを初期化
    for (UINT32 i = 0; i < STREAMING_BUFFER_COUNT; i++) {
        isStreamingBufferSubmitted[i] = false;
    }

    return true;
}

void AudioSource::Play(bool looping) {
    if (!sourceVoice) {
        return;
    }

    // 既に再生中なら一度停止
    if (isPlaying) {
        sourceVoice->Stop();
        sourceVoice->FlushSourceBuffers();
    }

    // ループ設定
    isLooping = looping;

    if (isStreaming) {
        // ストリーミングモードの場合
        // ストリーミングバッファをすべて埋める
        for (UINT32 i = 0; i < STREAMING_BUFFER_COUNT; i++) {
            if (FillBuffer(i)) {
                SubmitBuffer(i);
            }
        }

        // 再生開始
        sourceVoice->Start();
    }
    else {
        // 通常モードの場合
        // バッファの設定
        XAUDIO2_BUFFER buffer = {};
        buffer.AudioBytes = static_cast<UINT32>(audioData.size());
        buffer.pAudioData = audioData.data();
        buffer.Flags = XAUDIO2_END_OF_STREAM;

        // ループ設定
        if (looping) {
            buffer.LoopBegin = loopStartSample;
            buffer.LoopLength = loopEndSample - loopStartSample;
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        }

        // バッファの登録
        HRESULT hr = sourceVoice->SubmitSourceBuffer(&buffer);
        if (FAILED(hr)) {
            return;
        }

        // ボリュームの設定
        sourceVoice->SetVolume(volume);

        // 再生開始
        sourceVoice->Start();
    }

    // 状態の更新
    isPlaying = true;
    isPaused = false;
}

void AudioSource::Stop() {
    if (!sourceVoice || !isPlaying) {
        return;
    }

    // 停止
    sourceVoice->Stop();
    sourceVoice->FlushSourceBuffers();

    // ストリーミングモードの場合、バッファをリセット
    if (isStreaming) {
        for (UINT32 i = 0; i < STREAMING_BUFFER_COUNT; i++) {
            isStreamingBufferSubmitted[i] = false;
        }

        currentStreamingBuffer = 0;

        // ストリーミングファイルの位置をリセット
        if (streamingWaveFile) {
            // WAVファイルはシーク機能がないため、再読み込みが必要
        }
        else if (streamingMp3File) {
            // MP3も同様
        }
    }

    // 状態の更新
    isPlaying = false;
    isPaused = false;

    // フェード中ならキャンセル
    isFading = false;
}

void AudioSource::Pause() {
    if (!sourceVoice || !isPlaying || isPaused) {
        return;
    }

    // 一時停止
    sourceVoice->Stop();

    // 状態の更新
    isPaused = true;
}

void AudioSource::Resume() {
    if (!sourceVoice || !isPlaying || !isPaused) {
        return;
    }

    // 再開
    sourceVoice->Start();

    // 状態の更新
    isPaused = false;
}

void AudioSource::SetVolume(float volume) {
    // 0.0f～1.0fの範囲に制限
    this->volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

    if (sourceVoice) {
        sourceVoice->SetVolume(this->volume);
    }
}

float AudioSource::GetVolume() const {
    return volume;
}

void AudioSource::SetPitch(float pitch) {
    // 0.5f～2.0fの範囲に制限
    this->pitch = (pitch < 0.5f) ? 0.5f : (pitch > 2.0f) ? 2.0f : pitch;

    if (sourceVoice) {
        sourceVoice->SetFrequencyRatio(this->pitch);
    }
}

float AudioSource::GetPitch() const {
    return pitch;
}

void AudioSource::SetPan(float pan) {
    // -1.0f～1.0fの範囲に制限
    this->pan = (pan < -1.0f) ? -1.0f : (pan > 1.0f) ? 1.0f : pan;

    if (sourceVoice) {
        // パンマトリックスの計算
        float matrix[8] = { 0 }; // 最大4チャンネル出力×2チャンネル入力
        CalculatePanMatrix(this->pan, matrix, waveFormat.nChannels);

        // パンマトリックスの設定
        sourceVoice->SetOutputMatrix(nullptr, waveFormat.nChannels, 2, matrix);
    }
}

float AudioSource::GetPan() const {
    return pan;
}

void AudioSource::CalculatePanMatrix(float pan, float* matrix, UINT32 channelCount) {
    // チャンネル数に応じたパンマトリックスの計算
    if (channelCount == 1) {
        // モノラル入力 -> ステレオ出力
        float leftGain = (pan <= 0.0f) ? 1.0f : 1.0f - pan;
        float rightGain = (pan >= 0.0f) ? 1.0f : 1.0f + pan;

        matrix[0] = leftGain;  // 左チャンネル出力 (入力チャンネル0から)
        matrix[1] = rightGain; // 右チャンネル出力 (入力チャンネル0から)
    }
    else if (channelCount == 2) {
        // ステレオ入力 -> ステレオ出力
        float leftToLeft = (pan <= 0.0f) ? 1.0f : 1.0f - pan;
        float leftToRight = (pan >= 0.0f) ? pan : 0.0f;
        float rightToLeft = (pan <= 0.0f) ? -pan : 0.0f;
        float rightToRight = (pan >= 0.0f) ? 1.0f : 1.0f + pan;

        matrix[0] = leftToLeft;    // 左出力 (左入力から)
        matrix[1] = leftToRight;   // 右出力 (左入力から)
        matrix[2] = rightToLeft;   // 左出力 (右入力から)
        matrix[3] = rightToRight;  // 右出力 (右入力から)
    }
    // その他のチャンネル構成は省略
}

void AudioSource::FadeIn(float duration, std::function<void()> completeCallback) {
    if (!sourceVoice) {
        return;
    }

    // フェード設定
    isFading = true;
    fadeStartVolume = 0.0f;
    fadeEndVolume = volume;
    fadeTime = duration;
    fadeElapsedTime = 0.0f;
    fadeCompleteCallback = completeCallback;

    // 初期ボリュームを0に設定
    sourceVoice->SetVolume(0.0f);

    // 再生が止まっている場合は再生開始
    if (!isPlaying) {
        Play(isLooping);
    }
    else if (isPaused) {
        Resume();
    }
}

void AudioSource::FadeOut(float duration, std::function<void()> completeCallback) {
    if (!sourceVoice || !isPlaying) {
        return;
    }

    // フェード設定
    isFading = true;
    fadeStartVolume = volume;
    fadeEndVolume = 0.0f;
    fadeTime = duration;
    fadeElapsedTime = 0.0f;
    fadeCompleteCallback = completeCallback;
}

void AudioSource::FadeTo(float targetVolume, float duration, std::function<void()> completeCallback) {
    if (!sourceVoice || !isPlaying) {
        return;
    }

    // 0.0f～1.0fの範囲に制限
    targetVolume = (targetVolume < 0.0f) ? 0.0f : (targetVolume > 1.0f) ? 1.0f : targetVolume;

    // フェード設定
    isFading = true;
    fadeStartVolume = volume;
    fadeEndVolume = targetVolume;
    fadeTime = duration;
    fadeElapsedTime = 0.0f;
    fadeCompleteCallback = completeCallback;
}

void AudioSource::UpdateFade(float deltaTime) {
    if (!isFading || !sourceVoice || !isPlaying) {
        return;
    }

    // 経過時間を更新
    fadeElapsedTime += deltaTime;

    // フェード完了チェック
    if (fadeElapsedTime >= fadeTime) {
        // フェード完了
        volume = fadeEndVolume;
        sourceVoice->SetVolume(volume);
        isFading = false;

        // ボリュームが0になったらストップ
        if (fadeEndVolume <= 0.0f) {
            Stop();
        }

        // コールバックがあれば実行
        if (fadeCompleteCallback) {
            fadeCompleteCallback();
        }
    }
    else {
        // フェード中
        float t = fadeElapsedTime / fadeTime;
        float currentVolume = fadeStartVolume + (fadeEndVolume - fadeStartVolume) * t;
        sourceVoice->SetVolume(currentVolume);
    }
}

bool AudioSource::IsFading() const {
    return isFading;
}

void AudioSource::SetLoopPoints(UINT32 startSample, UINT32 endSample) {
    // 範囲チェック
    UINT32 totalSamples = GetTotalSamples();
    if (startSample >= totalSamples) {
        startSample = 0;
    }
    if (endSample > totalSamples || endSample <= startSample) {
        endSample = totalSamples;
    }

    loopStartSample = startSample;
    loopEndSample = endSample;

    // 再生中でループ再生ならループポイントを更新
    if (isPlaying && isLooping && !isStreaming) {
        Stop();
        Play(true);
    }
}

UINT32 AudioSource::GetLoopStartSample() const {
    return loopStartSample;
}

UINT32 AudioSource::GetLoopEndSample() const {
    return loopEndSample;
}

void AudioSource::SetGroupName(const std::string& name) {
    groupName = name;
}

const std::string& AudioSource::GetGroupName() const {
    return groupName;
}

UINT32 AudioSource::GetCurrentPosition() const {
    if (!sourceVoice) {
        return 0;
    }

    XAUDIO2_VOICE_STATE state;
    sourceVoice->GetState(&state);
    return state.SamplesPlayed % GetTotalSamples();
}

UINT32 AudioSource::GetTotalSamples() const {
    if (isStreaming) {
        // ストリーミングモードの場合は推定値
        return static_cast<UINT32>(GetTotalTime() * waveFormat.nSamplesPerSec);
    }
    else {
        // 通常モードの場合は実際のサンプル数
        return static_cast<UINT32>(audioData.size() / waveFormat.nBlockAlign);
    }
}

float AudioSource::GetCurrentTime() const {
    return static_cast<float>(GetCurrentPosition()) / waveFormat.nSamplesPerSec;
}

float AudioSource::GetTotalTime() const {
    if (isStreaming) {
        // ストリーミングモードの場合
        if (streamingWaveFile) {
            // TODO: WAVの総再生時間の取得
            return 0.0f;
        }
        else if (streamingMp3File) {
            // TODO: MP3の総再生時間の取得
            return 0.0f;
        }
        return 0.0f;
    }
    else {
        // 通常モードの場合
        return static_cast<float>(GetTotalSamples()) / waveFormat.nSamplesPerSec;
    }
}

void AudioSource::SetPosition(UINT32 samplePosition) {
    if (!sourceVoice) {
        return;
    }

    // 範囲チェック
    UINT32 totalSamples = GetTotalSamples();
    if (samplePosition >= totalSamples) {
        samplePosition = 0;
    }

    if (isStreaming) {
        // ストリーミングモードの場合はシーク操作が複雑
        // 現在はシーク操作が実装されていないため省略
    }
    else {
        // 通常モードの場合は再生し直し
        // 現在の再生状態を保存
        bool wasPlaying = isPlaying && !isPaused;
        bool wasLooping = isLooping;

        // 一旦停止
        Stop();

        // バッファを再設定
        XAUDIO2_BUFFER buffer = {};
        buffer.AudioBytes = static_cast<UINT32>(audioData.size());
        buffer.pAudioData = audioData.data();
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.PlayBegin = samplePosition;

        // ループ設定
        if (wasLooping) {
            buffer.LoopBegin = loopStartSample;
            buffer.LoopLength = loopEndSample - loopStartSample;
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        }

        // バッファの登録
        HRESULT hr = sourceVoice->SubmitSourceBuffer(&buffer);
        if (FAILED(hr)) {
            return;
        }

        // 再生状態を復元
        if (wasPlaying) {
            sourceVoice->Start();
            isPlaying = true;
            isPaused = false;
        }
    }
}

void AudioSource::SetTime(float seconds) {
    SetPosition(static_cast<UINT32>(seconds * waveFormat.nSamplesPerSec));
}

bool AudioSource::IsPlaying() const {
    return isPlaying;
}

bool AudioSource::IsPaused() const {
    return isPaused;
}

bool AudioSource::IsLooping() const {
    return isLooping;
}

bool AudioSource::IsStreaming() const {
    return isStreaming;
}

bool AudioSource::SubmitBuffer(UINT32 bufferIndex) {
    if (!sourceVoice || bufferIndex >= STREAMING_BUFFER_COUNT) {
        return false;
    }

    // バッファが既に送信済みならスキップ
    if (isStreamingBufferSubmitted[bufferIndex]) {
        return false;
    }

    // バッファを設定
    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = STREAMING_BUFFER_SIZE;
    buffer.pAudioData = streamingBuffers[bufferIndex];
    buffer.pContext = reinterpret_cast<void*>(static_cast<UINT_PTR>(bufferIndex));

    if (isLooping) {
        // ループ再生の場合はフラグなし
    }
    else {
        // 通常再生の場合は最後のバッファにEOSフラグを立てる
        // TODO: 最終バッファの判定
    }

    // バッファの登録
    HRESULT hr = sourceVoice->SubmitSourceBuffer(&buffer);
    if (SUCCEEDED(hr)) {
        isStreamingBufferSubmitted[bufferIndex] = true;
        return true;
    }

    return false;
}

void AudioSource::OnBufferEnd(UINT32 bufferIndex) {
    if (bufferIndex >= STREAMING_BUFFER_COUNT) {
        return;
    }

    // バッファは送信済みでなくなった
    isStreamingBufferSubmitted[bufferIndex] = false;

    // ストリーミング再生中なら次のバッファを用意
    if (isStreaming && isPlaying && !isPaused) {
        if (FillBuffer(bufferIndex)) {
            SubmitBuffer(bufferIndex);
        }
        else if (isLooping) {
            // ループ再生の場合は先頭に戻る
            // TODO: ファイル先頭への巻き戻し処理
            if (FillBuffer(bufferIndex)) {
                SubmitBuffer(bufferIndex);
            }
        }
        else {
            // すべてのバッファが送信済みになったら再生終了
            bool allBuffersEmpty = true;
            for (UINT32 i = 0; i < STREAMING_BUFFER_COUNT; i++) {
                if (isStreamingBufferSubmitted[i]) {
                    allBuffersEmpty = false;
                    break;
                }
            }
            if (allBuffersEmpty) {
                isPlaying = false;
            }
        }
    }
}

bool AudioSource::FillBuffer(UINT32 bufferIndex) {
    if (!isStreaming || bufferIndex >= STREAMING_BUFFER_COUNT) {
        return false;
    }

    // ファイル形式に応じたストリーミング読み込み
    // 現時点では実装が不十分なため、常に失敗を返す
    // TODO: ストリーミング読み込みの実装
    return false;
}

void AudioSource::VoiceCallback::OnBufferEnd(void* pBufferContext) {
    if (!audioSource) {
        return;
    }

    if (audioSource->isStreaming) {
        // ストリーミングモードの場合
        UINT32 bufferIndex = static_cast<UINT32>(reinterpret_cast<UINT_PTR>(pBufferContext));
        audioSource->OnBufferEnd(bufferIndex);
    }
    else {
        // 通常モードの場合
        // ループ再生でない場合は再生終了
        if (!audioSource->isLooping) {
            audioSource->isPlaying = false;
        }
    }
}

void AudioSource::VoiceCallback::OnStreamEnd() {
    // ストリームの終わりに達した（ストリーミングモードで使用）
    if (audioSource && audioSource->isStreaming) {
        audioSource->isPlaying = false;
    }
}

void AudioSource::VoiceCallback::OnBufferStart(void* pBufferContext) {
    // バッファの再生開始時に呼ばれる（必要に応じて実装）
}

void AudioSource::VoiceCallback::OnLoopEnd(void* pBufferContext) {
    // ループの終わりに達した（必要に応じて実装）
}