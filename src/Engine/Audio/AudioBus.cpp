// src/Engine/Audio/AudioBus.cpp
#include "AudioBus.h"
#include "AudioSource.h"
#include <algorithm>
#include <cassert>

AudioBus::AudioBus(const std::string& name, IXAudio2* xAudio2,
    UINT32 inputChannels, UINT32 outputChannels)
    : name_(name),
    submixVoice_(nullptr),
    volume_(1.0f) {

    assert(xAudio2);

    // サブミックスボイスの作成
    HRESULT hr = xAudio2->CreateSubmixVoice(
        &submixVoice_,
        outputChannels,
        44100, // サンプリングレート（一般的な値）
        0,
        0,
        nullptr,
        nullptr
    );
    assert(SUCCEEDED(hr) && submixVoice_);

    // エフェクトチェーンの初期化
    ZeroMemory(&effectChain_, sizeof(XAUDIO2_EFFECT_CHAIN));
}

AudioBus::~AudioBus() {
    // オーディオソースリストをクリア
    audioSources_.clear();

    // エフェクトチェーンのクリア
    for (auto& effect : effectInterfaces_) {
        if (effect) {
            effect->Release();
        }
    }
    effectInterfaces_.clear();
    effectDescriptors_.clear();
    effectParameters_.clear();

    // サブミックスボイスの破棄
    if (submixVoice_) {
        submixVoice_->DestroyVoice();
        submixVoice_ = nullptr;
    }
}

void AudioBus::AddAudioSource(AudioSource* audioSource) {
    // nullptrチェック
    if (!audioSource) {
        return;
    }

    // すでに追加済みかチェック
    if (std::find(audioSources_.begin(), audioSources_.end(), audioSource) != audioSources_.end()) {
        return;
    }

    // オーディオソースを追加
    audioSources_.push_back(audioSource);

    // サブミックスボイスに接続（オーディオグラフの変更）
    if (audioSource->GetSourceVoice() && submixVoice_) {
        // ソースボイスの出力先にこのバスのサブミックスボイスを追加
        XAUDIO2_SEND_DESCRIPTOR sendDesc = { 0, submixVoice_ };
        XAUDIO2_VOICE_SENDS sends = { 1, &sendDesc };

        HRESULT hr = audioSource->GetSourceVoice()->SetOutputVoices(&sends);
        assert(SUCCEEDED(hr));
    }
}

void AudioBus::RemoveAudioSource(AudioSource* audioSource) {
    // nullptrチェック
    if (!audioSource) {
        return;
    }

    // オーディオソースを検索して削除
    auto it = std::find(audioSources_.begin(), audioSources_.end(), audioSource);
    if (it != audioSources_.end()) {
        // オーディオソースをリストから削除
        audioSources_.erase(it);

        // サブミックスボイスからの接続解除
        if (audioSource->GetSourceVoice()) {
            // デフォルトの出力先に戻す（マスターボイスのみ）
            audioSource->GetSourceVoice()->SetOutputVoices(nullptr);
        }
    }
}

void AudioBus::PlayAll(bool looping) {
    // すべてのオーディオソースを再生
    for (auto& source : audioSources_) {
        if (source) {
            source->Play(looping);
        }
    }
}

void AudioBus::StopAll() {
    // すべてのオーディオソースを停止
    for (auto& source : audioSources_) {
        if (source) {
            source->Stop();
        }
    }
}

void AudioBus::PauseAll() {
    // すべてのオーディオソースを一時停止
    for (auto& source : audioSources_) {
        if (source) {
            source->Pause();
        }
    }
}

void AudioBus::ResumeAll() {
    // すべてのオーディオソースを再開
    for (auto& source : audioSources_) {
        if (source) {
            source->Resume();
        }
    }
}

void AudioBus::SetVolume(float volume) {
    // 0.0f～1.0fの範囲に制限
    volume_ = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

    // サブミックスボイスのボリュームを設定
    if (submixVoice_) {
        submixVoice_->SetVolume(volume_);
    }
}

float AudioBus::GetVolume() const {
    return volume_;
}

bool AudioBus::EnableEffect(const std::string& effectName, bool enabled) {
    // エフェクトが存在するかチェック
    auto it = effectParameters_.find(effectName);
    if (it == effectParameters_.end()) {
        return false;
    }

    // エフェクトの有効/無効を設定
    if (submixVoice_) {
        HRESULT hr = submixVoice_->EnableEffect(
            it->second.effectIndex,
            enabled ? XAUDIO2_COMMIT_NOW : 0
        );
        return SUCCEEDED(hr);
    }

    return false;
}

bool AudioBus::RemoveEffect(const std::string& effectName) {
    // エフェクトが存在するかチェック
    auto it = effectParameters_.find(effectName);
    if (it == effectParameters_.end()) {
        return false;
    }

    // エフェクトインデックスを取得
    UINT32 effectIndex = it->second.effectIndex;

    // エフェクトを無効化
    if (submixVoice_) {
        submixVoice_->DisableEffect(effectIndex);
    }

    // エフェクトインターフェースの参照を解放
    effectInterfaces_[effectIndex]->Release();
    effectInterfaces_[effectIndex] = nullptr;

    // エフェクトパラメータを削除
    effectParameters_.erase(it);

    // エフェクトチェーンを再構築
    return RebuildEffectChain();
}

bool AudioBus::RebuildEffectChain() {
    // エフェクトがない場合は空のチェーンにする
    if (effectDescriptors_.empty()) {
        if (submixVoice_) {
            // 空のエフェクトチェーンを設定
            XAUDIO2_EFFECT_CHAIN emptyChain = { 0, nullptr };
            HRESULT hr = submixVoice_->SetEffectChain(&emptyChain);
            return SUCCEEDED(hr);
        }
        return false;
    }

    // エフェクトチェーンを設定
    effectChain_.EffectCount = static_cast<UINT32>(effectDescriptors_.size());
    effectChain_.pEffectDescriptors = effectDescriptors_.data();

    // サブミックスボイスにエフェクトチェーンを適用
    if (submixVoice_) {
        HRESULT hr = submixVoice_->SetEffectChain(&effectChain_);
        return SUCCEEDED(hr);
    }

    return false;
}