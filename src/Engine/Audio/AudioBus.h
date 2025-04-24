// src/Engine/Audio/AudioBus.h
#pragma once

#include <xaudio2.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class AudioSource;

// オーディオバスクラス - 複数の音声をグループ化して一括管理
class AudioBus {
private:
    // バス名
    std::string name_;

    // サブミックスボイス
    IXAudio2SubmixVoice* submixVoice_;

    // バスに所属するオーディオソースリスト
    std::vector<AudioSource*> audioSources_;

    // ボリューム
    float volume_;

    // バスのエフェクト
    struct EffectParameter {
        UINT32 effectIndex;
        std::unique_ptr<void, void(*)(void*)> parameters;
        UINT32 parameterSize;

        EffectParameter() : effectIndex(0), parameters(nullptr, [](void*) {}), parameterSize(0) {}
        EffectParameter(UINT32 index, void* params, UINT32 size, void(*deleter)(void*))
            : effectIndex(index), parameters(params, deleter), parameterSize(size) {
        }
    };

    // エフェクトチェーン
    XAUDIO2_EFFECT_CHAIN effectChain_;
    std::vector<XAUDIO2_EFFECT_DESCRIPTOR> effectDescriptors_;
    std::vector<IUnknown*> effectInterfaces_;
    std::unordered_map<std::string, EffectParameter> effectParameters_;

public:
    // コンストラクタ
    AudioBus(const std::string& name, IXAudio2* xAudio2,
        UINT32 inputChannels = 2, UINT32 outputChannels = 2);

    // デストラクタ
    ~AudioBus();

    // オーディオソースの追加
    void AddAudioSource(AudioSource* audioSource);

    // オーディオソースの削除
    void RemoveAudioSource(AudioSource* audioSource);

    // すべてのオーディオソースの再生
    void PlayAll(bool looping = false);

    // すべてのオーディオソースの停止
    void StopAll();

    // すべてのオーディオソースの一時停止
    void PauseAll();

    // すべてのオーディオソースの再開
    void ResumeAll();

    // ボリューム設定
    void SetVolume(float volume);
    float GetVolume() const;

    // エフェクトの追加
    template<typename T>
    bool AddEffect(const std::string& effectName, IUnknown* effectInterface,
        const T& parameters, void(*deleter)(void*) = [](void* p) { delete static_cast<T*>(p); });

    // エフェクトパラメータの更新
    template<typename T>
    bool UpdateEffectParameters(const std::string& effectName, const T& parameters);

    // エフェクトの有効/無効切り替え
    bool EnableEffect(const std::string& effectName, bool enabled);

    // エフェクトの削除
    bool RemoveEffect(const std::string& effectName);

    // サブミックスボイスの取得
    IXAudio2SubmixVoice* GetSubmixVoice() const { return submixVoice_; }

    // バス名の取得
    const std::string& GetName() const { return name_; }

private:
    // エフェクトチェーンの再構築
    bool RebuildEffectChain();
};

// テンプレート関数の実装
template<typename T>
bool AudioBus::AddEffect(const std::string& effectName, IUnknown* effectInterface,
    const T& parameters, void(*deleter)(void*)) {
    // すでに存在するエフェクト名かチェック
    if (effectParameters_.find(effectName) != effectParameters_.end()) {
        return false;
    }

    // エフェクトデスクリプタを追加
    XAUDIO2_EFFECT_DESCRIPTOR descriptor;
    descriptor.InitialState = TRUE;
    descriptor.OutputChannels = 2; // ステレオ出力を想定
    descriptor.pEffect = effectInterface;
    effectDescriptors_.push_back(descriptor);

    // エフェクトインターフェースを保存
    effectInterfaces_.push_back(effectInterface);
    effectInterface->AddRef(); // 参照カウントを増やす

    // エフェクトパラメータをコピー
    T* paramsCopy = new T(parameters);
    effectParameters_[effectName] = EffectParameter(
        static_cast<UINT32>(effectDescriptors_.size() - 1),
        paramsCopy,
        sizeof(T),
        deleter
    );

    // エフェクトチェーンを再構築
    return RebuildEffectChain();
}

template<typename T>
bool AudioBus::UpdateEffectParameters(const std::string& effectName, const T& parameters) {
    // エフェクトが存在するかチェック
    auto it = effectParameters_.find(effectName);
    if (it == effectParameters_.end()) {
        return false;
    }

    // パラメータサイズが一致するかチェック
    if (it->second.parameterSize != sizeof(T)) {
        return false;
    }

    // パラメータをコピー
    T* paramsCopy = static_cast<T*>(it->second.parameters.get());
    *paramsCopy = parameters;

    // パラメータを設定
    if (submixVoice_) {
        HRESULT hr = submixVoice_->SetEffectParameters(
            it->second.effectIndex,
            paramsCopy,
            sizeof(T),
            0
        );
        return SUCCEEDED(hr);
    }

    return false;
}