// src/Engine/Audio/AudioSource.cpp
#include "AudioSource.h"
#include "WaveFile.h"
#include "Mp3File.h"
#include <cassert>

AudioSource::AudioSource()
    : sourceVoice(nullptr), isPlaying(false), isPaused(false), isLooping(false), volume(1.0f) {
    // コールバックハンドラの設定
    voiceCallback.audioSource = this;
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

    return SUCCEEDED(hr);
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

    // バッファの設定
    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = static_cast<UINT32>(audioData.size());
    buffer.pAudioData = audioData.data();
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    // ループ設定
    if (looping) {
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

    // 状態の更新
    isPlaying = true;
    isPaused = false;
    isLooping = looping;
}

void AudioSource::Stop() {
    if (!sourceVoice || !isPlaying) {
        return;
    }

    // 停止
    sourceVoice->Stop();
    sourceVoice->FlushSourceBuffers();

    // 状態の更新
    isPlaying = false;
    isPaused = false;
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

bool AudioSource::IsPlaying() const {
    return isPlaying;
}

bool AudioSource::IsPaused() const {
    return isPaused;
}

bool AudioSource::IsLooping() const {
    return isLooping;
}

void AudioSource::VoiceCallback::OnBufferEnd(void* pBufferContext) {
    // ループ再生でない場合は再生終了
    if (audioSource && !audioSource->isLooping) {
        audioSource->isPlaying = false;
    }
}