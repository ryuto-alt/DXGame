// src/Engine/Audio/AudioEvent.cpp
#include "AudioEvent.h"
#include "AudioManager.h"
#include "AudioSource.h"
#include "AudioSource3D.h"
#include "WaveFile.h"
#include "Mp3File.h"
#include <random>
#include <ctime>
#include <cassert>

// オーディオイベントの基底クラス実装
AudioEvent::AudioEvent(const std::string& name, AudioEventType type, const AudioEventParams& params, AudioManager* audioManager)
    : name_(name),
    type_(type),
    params_(params),
    audioManager_(audioManager),
    isPlaying_(false),
    isPaused_(false),
    elapsedTime_(0.0f),
    delayTimer_(0.0f),
    stopTimer_(-1.0f) {
    // パラメータのチェック
    assert(audioManager != nullptr);

    // 乱数エンジンの初期化（静的メンバ、一度だけ）
    static bool isRandomInitialized = false;
    if (!isRandomInitialized) {
        randomEngine_.seed(static_cast<unsigned int>(time(nullptr)));
        isRandomInitialized = true;
    }
}

AudioEvent::~AudioEvent() {
    // 基底クラスでは特に何もしない
}

void AudioEvent::Play() {
    // 既に再生中の場合は何もしない
    if (isPlaying_ && !isPaused_) {
        return;
    }

    // 遅延タイマーとストップタイマーのリセット
    delayTimer_ = params_.delay;
    stopTimer_ = params_.stopTime;

    // 遅延がない場合は即座に再生
    if (delayTimer_ <= 0.0f) {
        PlayInternal();
    }

    // 状態の更新
    isPlaying_ = true;
    isPaused_ = false;
}

void AudioEvent::Stop() {
    // 再生中でない場合は何もしない
    if (!isPlaying_) {
        return;
    }

    // 内部的な停止処理
    StopInternal();

    // 状態の更新
    isPlaying_ = false;
    isPaused_ = false;

    // タイマーのリセット
    elapsedTime_ = 0.0f;
    delayTimer_ = 0.0f;
    stopTimer_ = -1.0f;
}

void AudioEvent::Pause() {
    // 既に一時停止中または再生中でない場合は何もしない
    if (isPaused_ || !isPlaying_) {
        return;
    }

    // 実装は派生クラスに任せる
    isPaused_ = true;
}

void AudioEvent::Resume() {
    // 一時停止中でない場合は何もしない
    if (!isPaused_ || !isPlaying_) {
        return;
    }

    // 実装は派生クラスに任せる
    isPaused_ = false;
}

void AudioEvent::Update(float deltaTime) {
    // 再生中でない場合は何もしない
    if (!isPlaying_) {
        return;
    }

    // 一時停止中の場合は時間を進めない
    if (isPaused_) {
        return;
    }

    // 経過時間の更新
    elapsedTime_ += deltaTime;

    // 遅延タイマーの処理
    if (delayTimer_ > 0.0f) {
        delayTimer_ -= deltaTime;
        if (delayTimer_ <= 0.0f) {
            // 遅延時間が経過したら実際に再生開始
            PlayInternal();
        }
    }

    // ストップタイマーの処理
    if (stopTimer_ > 0.0f) {
        stopTimer_ -= deltaTime;
        if (stopTimer_ <= 0.0f) {
            // 停止時間が経過したら停止
            Stop();

            // コールバックの呼び出し
            if (completeCallback_) {
                completeCallback_();
            }
        }
    }
}

bool AudioEvent::IsPlaying() const {
    return isPlaying_;
}

bool AudioEvent::IsPaused() const {
    return isPaused_;
}

void AudioEvent::SetPosition(const Vector3& position) {
    // デフォルトでは何もしない（3Dイベントのみ対応）
}

void AudioEvent::SetVelocity(const Vector3& velocity) {
    // デフォルトでは何もしない（3Dイベントのみ対応）
}

void AudioEvent::SetVolume(float volume) {
    // デフォルトでは何もしない（派生クラスで実装）
}

void AudioEvent::SetPitch(float pitch) {
    // デフォルトでは何もしない（派生クラスで実装）
}

void AudioEvent::SetPan(float pan) {
    // デフォルトでは何もしない（派生クラスで実装）
}

void AudioEvent::SetCompleteCallback(std::function<void()> callback) {
    completeCallback_ = callback;
}

const std::string& AudioEvent::GetName() const {
    return name_;
}

const AudioEventParams& AudioEvent::GetParams() const {
    return params_;
}

void AudioEvent::Cleanup() {
    // 基底クラスでは特に何もしない
}

void AudioEvent::ApplyVariation(AudioEventParams& params) {
    // ボリュームバリエーション
    if (params_.volumeVariation > 0.0f) {
        std::uniform_real_distribution<float> volumeDist(-params_.volumeVariation, params_.volumeVariation);
        params.volume = params_.volume + volumeDist(randomEngine_);
        params.volume = (params.volume < 0.0f) ? 0.0f : (params.volume > 1.0f) ? 1.0f : params.volume;
    }

    // ピッチバリエーション
    if (params_.pitchVariation > 0.0f) {
        std::uniform_real_distribution<float> pitchDist(-params_.pitchVariation, params_.pitchVariation);
        params.pitch = params_.pitch + pitchDist(randomEngine_);
        params.pitch = (params.pitch < 0.5f) ? 0.5f : (params.pitch > 2.0f) ? 2.0f : params.pitch;
    }
}


// SimpleAudioEvent実装

SimpleAudioEvent::SimpleAudioEvent(const std::string& name, const std::string& filePath, const AudioEventParams& params, AudioManager* audioManager)
    : AudioEvent(name, AudioEventType::Simple, params, audioManager),
    filePath_(filePath),
    audioSource_(nullptr),
    audioSource3D_(nullptr) {
    // 3Dオーディオかどうかで初期化方法を分ける
    if (params.is3D) {
        // 3Dオーディオソースの作成
        audioManager_->Create3DAudioSource(name + "_source", filePath_);
        audioSource3D_ = audioManager_->GetAudioSource3D(name + "_source");

        if (audioSource3D_) {
            // 3D位置の設定
            audioSource3D_->SetPosition(params.position);
            audioSource3D_->SetVelocity(params.velocity);
            audioSource3D_->SetDistance(params.minDistance, params.maxDistance);
        }
    }
    else {
        // 通常のオーディオソースの作成
        audioManager_->LoadAudioFile(name + "_source", filePath_);
        audioSource_ = audioManager_->GetAudioSource(name + "_source");

        if (audioSource_) {
            // 各種パラメータの設定
            audioSource_->SetVolume(params.volume);
            audioSource_->SetPitch(params.pitch);
            audioSource_->SetPan(params.pan);
        }
    }
}

SimpleAudioEvent::~SimpleAudioEvent() {
    // オーディオソースの所有権はAudioManagerにあるため、ここでは何もしない
}

void SimpleAudioEvent::SetPosition(const Vector3& position) {
    if (audioSource3D_) {
        audioSource3D_->SetPosition(position);
    }
}

void SimpleAudioEvent::SetVelocity(const Vector3& velocity) {
    if (audioSource3D_) {
        audioSource3D_->SetVelocity(velocity);
    }
}

void SimpleAudioEvent::SetVolume(float volume) {
    if (audioSource_) {
        audioSource_->SetVolume(volume);
    }
    else if (audioSource3D_) {
        audioSource3D_->SetVolume(volume);
    }
}

void SimpleAudioEvent::SetPitch(float pitch) {
    if (audioSource_) {
        audioSource_->SetPitch(pitch);
    }
    else if (audioSource3D_) {
        audioSource3D_->SetPitch(pitch);
    }
}

void SimpleAudioEvent::SetPan(float pan) {
    if (audioSource_) {
        audioSource_->SetPan(pan);
    }
    // 3Dオーディオソースの場合、パンは位置で制御されるため設定しない
}

void SimpleAudioEvent::PlayInternal() {
    // オーディオソースが有効かチェック
    if (!audioSource_ && !audioSource3D_) {
        return;
    }

    // バリエーションを適用したパラメータを作成
    AudioEventParams variedParams = params_;
    ApplyVariation(variedParams);

    // 通常のオーディオソースの場合
    if (audioSource_) {
        // ボリューム、ピッチ、パンを設定
        audioSource_->SetVolume(variedParams.volume);
        audioSource_->SetPitch(variedParams.pitch);
        audioSource_->SetPan(variedParams.pan);

        // フェードインの設定
        if (variedParams.fadeInTime > 0.0f) {
            audioSource_->FadeIn(variedParams.fadeInTime);
        }
        else {
            // 通常再生
            audioSource_->Play(variedParams.loop);
        }
    }
    // 3Dオーディオソースの場合
    else if (audioSource3D_) {
        // ボリューム、ピッチを設定
        audioSource3D_->SetVolume(variedParams.volume);
        audioSource3D_->SetPitch(variedParams.pitch);

        // 位置・速度を設定
        audioSource3D_->SetPosition(variedParams.position);
        audioSource3D_->SetVelocity(variedParams.velocity);

        // 減衰距離を設定
        audioSource3D_->SetDistance(variedParams.minDistance, variedParams.maxDistance);

        // フェードインの設定
        if (variedParams.fadeInTime > 0.0f) {
            audioSource3D_->FadeIn(variedParams.fadeInTime);
        }
        else {
            // 通常再生
            audioSource3D_->Play(variedParams.loop);
        }
    }
}

void SimpleAudioEvent::StopInternal() {
    // フェードアウトの設定
    if (params_.fadeOutTime > 0.0f) {
        if (audioSource_) {
            audioSource_->FadeOut(params_.fadeOutTime);
        }
        else if (audioSource3D_) {
            audioSource3D_->FadeOut(params_.fadeOutTime);
        }
    }
    else {
        // 即時停止
        if (audioSource_) {
            audioSource_->Stop();
        }
        else if (audioSource3D_) {
            audioSource3D_->Stop();
        }
    }
}

void SimpleAudioEvent::Cleanup() {
    // 必要に応じてクリーンアップ処理を実装
}


// RandomAudioEvent実装

RandomAudioEvent::RandomAudioEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params, AudioManager* audioManager)
    : AudioEvent(name, AudioEventType::Random, params, audioManager),
    filePaths_(filePaths),
    currentAudioSource_(nullptr),
    currentAudioSource3D_(nullptr),
    currentIndex_(-1) {
    // ファイルパスが空かチェック
    assert(!filePaths.empty());
}

RandomAudioEvent::~RandomAudioEvent() {
    // オーディオソースの所有権はAudioManagerにあるため、ここでは何もしない
}

void RandomAudioEvent::SetPosition(const Vector3& position) {
    if (currentAudioSource3D_) {
        currentAudioSource3D_->SetPosition(position);
    }
}

void RandomAudioEvent::SetVelocity(const Vector3& velocity) {
    if (currentAudioSource3D_) {
        currentAudioSource3D_->SetVelocity(velocity);
    }
}

void RandomAudioEvent::SetVolume(float volume) {
    if (currentAudioSource_) {
        currentAudioSource_->SetVolume(volume);
    }
    else if (currentAudioSource3D_) {
        currentAudioSource3D_->SetVolume(volume);
    }
}

void RandomAudioEvent::SetPitch(float pitch) {
    if (currentAudioSource_) {
        currentAudioSource_->SetPitch(pitch);
    }
    else if (currentAudioSource3D_) {
        currentAudioSource3D_->SetPitch(pitch);
    }
}

void RandomAudioEvent::SetPan(float pan) {
    if (currentAudioSource_) {
        currentAudioSource_->SetPan(pan);
    }
    // 3Dオーディオソースの場合、パンは位置で制御されるため設定しない
}

void RandomAudioEvent::PlayInternal() {
    // ランダムに音声を選択
    currentIndex_ = SelectRandomIndex();

    // 無効なインデックスの場合は何もしない
    if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(filePaths_.size())) {
        return;
    }

    // オーディオソースの取得・作成
    std::string sourceName = name_ + "_source" + std::to_string(currentIndex_);

    // バリエーションを適用したパラメータを作成
    AudioEventParams variedParams = params_;
    ApplyVariation(variedParams);

    // 3Dオーディオかどうかで処理を分ける
    if (variedParams.is3D) {
        // 3Dオーディオソースが存在しない場合は作成
        if (!audioManager_->GetAudioSource3D(sourceName)) {
            audioManager_->Create3DAudioSource(sourceName, filePaths_[currentIndex_]);
        }

        currentAudioSource3D_ = audioManager_->GetAudioSource3D(sourceName);
        currentAudioSource_ = nullptr;

        if (currentAudioSource3D_) {
            // 各種パラメータの設定
            currentAudioSource3D_->SetVolume(variedParams.volume);
            currentAudioSource3D_->SetPitch(variedParams.pitch);
            currentAudioSource3D_->SetPosition(variedParams.position);
            currentAudioSource3D_->SetVelocity(variedParams.velocity);
            currentAudioSource3D_->SetDistance(variedParams.minDistance, variedParams.maxDistance);

            // フェードインの設定
            if (variedParams.fadeInTime > 0.0f) {
                currentAudioSource3D_->FadeIn(variedParams.fadeInTime);
            }
            else {
                // 通常再生
                currentAudioSource3D_->Play(variedParams.loop);
            }
        }
    }
    else {
        // 通常のオーディオソースが存在しない場合は作成
        if (!audioManager_->GetAudioSource(sourceName)) {
            audioManager_->LoadAudioFile(sourceName, filePaths_[currentIndex_]);
        }

        currentAudioSource_ = audioManager_->GetAudioSource(sourceName);
        currentAudioSource3D_ = nullptr;

        if (currentAudioSource_) {
            // 各種パラメータの設定
            currentAudioSource_->SetVolume(variedParams.volume);
            currentAudioSource_->SetPitch(variedParams.pitch);
            currentAudioSource_->SetPan(variedParams.pan);

            // フェードインの設定
            if (variedParams.fadeInTime > 0.0f) {
                currentAudioSource_->FadeIn(variedParams.fadeInTime);
            }
            else {
                // 通常再生
                currentAudioSource_->Play(variedParams.loop);
            }
        }
    }
}

void RandomAudioEvent::StopInternal() {
    // フェードアウトの設定
    if (params_.fadeOutTime > 0.0f) {
        if (currentAudioSource_) {
            currentAudioSource_->FadeOut(params_.fadeOutTime);
        }
        else if (currentAudioSource3D_) {
            currentAudioSource3D_->FadeOut(params_.fadeOutTime);
        }
    }
    else {
        // 即時停止
        if (currentAudioSource_) {
            currentAudioSource_->Stop();
        }
        else if (currentAudioSource3D_) {
            currentAudioSource3D_->Stop();
        }
    }

    // リソースのクリア
    currentAudioSource_ = nullptr;
    currentAudioSource3D_ = nullptr;
    currentIndex_ = -1;
}

void RandomAudioEvent::Cleanup() {
    // 必要に応じてクリーンアップ処理を実装
}

int RandomAudioEvent::SelectRandomIndex() {
    // ファイルパスが空の場合は-1を返す
    if (filePaths_.empty()) {
        return -1;
    }

    // 1つしかない場合は0を返す
    if (filePaths_.size() == 1) {
        return 0;
    }

    // 乱数分布の作成
    std::uniform_int_distribution<int> dist(0, static_cast<int>(filePaths_.size()) - 1);

    // 前回と異なるインデックスを選択
    int index = dist(randomEngine_);
    if (index == currentIndex_ && filePaths_.size() > 1) {
        index = (index + 1) % filePaths_.size();
    }

    return index;
}


// SequentialAudioEvent実装

SequentialAudioEvent::SequentialAudioEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params, AudioManager* audioManager)
    : AudioEvent(name, AudioEventType::Sequential, params, audioManager),
    filePaths_(filePaths),
    currentAudioSource_(nullptr),
    currentAudioSource3D_(nullptr),
    currentIndex_(-1) {
    // ファイルパスが空かチェック
    assert(!filePaths.empty());
}

SequentialAudioEvent::~SequentialAudioEvent() {
    // オーディオソースの所有権はAudioManagerにあるため、ここでは何もしない
}

void SequentialAudioEvent::SetPosition(const Vector3& position) {
    if (currentAudioSource3D_) {
        currentAudioSource3D_->SetPosition(position);
    }
}

void SequentialAudioEvent::SetVelocity(const Vector3& velocity) {
    if (currentAudioSource3D_) {
        currentAudioSource3D_->SetVelocity(velocity);
    }
}

void SequentialAudioEvent::SetVolume(float volume) {
    if (currentAudioSource_) {
        currentAudioSource_->SetVolume(volume);
    }
    else if (currentAudioSource3D_) {
        currentAudioSource3D_->SetVolume(volume);
    }
}

void SequentialAudioEvent::SetPitch(float pitch) {
    if (currentAudioSource_) {
        currentAudioSource_->SetPitch(pitch);
    }
    else if (currentAudioSource3D_) {
        currentAudioSource3D_->SetPitch(pitch);
    }
}

void SequentialAudioEvent::SetPan(float pan) {
    if (currentAudioSource_) {
        currentAudioSource_->SetPan(pan);
    }
    // 3Dオーディオソースの場合、パンは位置で制御されるため設定しない
}

void SequentialAudioEvent::PlayInternal() {
    // 初回再生時または最後まで再生した後は先頭から
    if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(filePaths_.size()) - 1) {
        currentIndex_ = 0;
    }
    else {
        // 次の音声へ
        currentIndex_++;
    }

    // 無効なインデックスの場合は何もしない
    if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(filePaths_.size())) {
        return;
    }

    // オーディオソースの取得・作成
    std::string sourceName = name_ + "_source" + std::to_string(currentIndex_);

    // バリエーションを適用したパラメータを作成
    AudioEventParams variedParams = params_;
    ApplyVariation(variedParams);

    // 3Dオーディオかどうかで処理を分ける
    if (variedParams.is3D) {
        // 3Dオーディオソースが存在しない場合は作成
        if (!audioManager_->GetAudioSource3D(sourceName)) {
            audioManager_->Create3DAudioSource(sourceName, filePaths_[currentIndex_]);
        }

        currentAudioSource3D_ = audioManager_->GetAudioSource3D(sourceName);
        currentAudioSource_ = nullptr;

        if (currentAudioSource3D_) {
            // 各種パラメータの設定
            currentAudioSource3D_->SetVolume(variedParams.volume);
            currentAudioSource3D_->SetPitch(variedParams.pitch);
            currentAudioSource3D_->SetPosition(variedParams.position);
            currentAudioSource3D_->SetVelocity(variedParams.velocity);
            currentAudioSource3D_->SetDistance(variedParams.minDistance, variedParams.maxDistance);

            // フェードインの設定
            if (variedParams.fadeInTime > 0.0f) {
                currentAudioSource3D_->FadeIn(variedParams.fadeInTime);
            }
            else {
                // 通常再生
                currentAudioSource3D_->Play(false); // シーケンシャルなので単体はループしない
            }
        }
    }
    else {
        // 通常のオーディオソースが存在しない場合は作成
        if (!audioManager_->GetAudioSource(sourceName)) {
            audioManager_->LoadAudioFile(sourceName, filePaths_[currentIndex_]);
        }

        currentAudioSource_ = audioManager_->GetAudioSource(sourceName);
        currentAudioSource3D_ = nullptr;

        if (currentAudioSource_) {
            // 各種パラメータの設定
            currentAudioSource_->SetVolume(variedParams.volume);
            currentAudioSource_->SetPitch(variedParams.pitch);
            currentAudioSource_->SetPan(variedParams.pan);

            // フェードインの設定
            if (variedParams.fadeInTime > 0.0f) {
                currentAudioSource_->FadeIn(variedParams.fadeInTime);
            }
            else {
                // 通常再生
                currentAudioSource_->Play(false); // シーケンシャルなので単体はループしない
            }
        }
    }
}

void SequentialAudioEvent::StopInternal() {
    // フェードアウトの設定
    if (params_.fadeOutTime > 0.0f) {
        if (currentAudioSource_) {
            currentAudioSource_->FadeOut(params_.fadeOutTime);
        }
        else if (currentAudioSource3D_) {
            currentAudioSource3D_->FadeOut(params_.fadeOutTime);
        }
    }
    else {
        // 即時停止
        if (currentAudioSource_) {
            currentAudioSource_->Stop();
        }
        else if (currentAudioSource3D_) {
            currentAudioSource3D_->Stop();
        }
    }

    // リソースのクリア
    currentAudioSource_ = nullptr;
    currentAudioSource3D_ = nullptr;
    currentIndex_ = -1;
}

void SequentialAudioEvent::Cleanup() {
    // 必要に応じてクリーンアップ処理を実装
}

void SequentialAudioEvent::AdvanceToNextSound() {
    // 現在の音声を停止
    if (currentAudioSource_) {
        currentAudioSource_->Stop();
    }
    else if (currentAudioSource3D_) {
        currentAudioSource3D_->Stop();
    }

    // 次の音声を再生
    PlayInternal();
}


// LayeredAudioEvent実装

LayeredAudioEvent::LayeredAudioEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params, AudioManager* audioManager)
    : AudioEvent(name, AudioEventType::Layered, params, audioManager),
    filePaths_(filePaths) {
    // ファイルパスが空かチェック
    assert(!filePaths.empty());
}

LayeredAudioEvent::~LayeredAudioEvent() {
    // オーディオソースの所有権はAudioManagerにあるため、ここでは何もしない
}

void LayeredAudioEvent::SetPosition(const Vector3& position) {
    for (auto source : audioSources3D_) {
        if (source) {
            source->SetPosition(position);
        }
    }
}

void LayeredAudioEvent::SetVelocity(const Vector3& velocity) {
    for (auto source : audioSources3D_) {
        if (source) {
            source->SetVelocity(velocity);
        }
    }
}

void LayeredAudioEvent::SetVolume(float volume) {
    for (auto source : audioSources_) {
        if (source) {
            source->SetVolume(volume);
        }
    }

    for (auto source : audioSources3D_) {
        if (source) {
            source->SetVolume(volume);
        }
    }
}

void LayeredAudioEvent::SetPitch(float pitch) {
    for (auto source : audioSources_) {
        if (source) {
            source->SetPitch(pitch);
        }
    }

    for (auto source : audioSources3D_) {
        if (source) {
            source->SetPitch(pitch);
        }
    }
}

void LayeredAudioEvent::SetPan(float pan) {
    for (auto source : audioSources_) {
        if (source) {
            source->SetPan(pan);
        }
    }
    // 3Dオーディオソースの場合、パンは位置で制御されるため設定しない
}

void LayeredAudioEvent::PlayInternal() {
    // オーディオソースの配列をクリア
    audioSources_.clear();
    audioSources3D_.clear();

    // バリエーションを適用したパラメータを作成
    AudioEventParams variedParams = params_;
    ApplyVariation(variedParams);

    // 各ファイルに対応するオーディオソースを作成・再生
    for (size_t i = 0; i < filePaths_.size(); ++i) {
        std::string sourceName = name_ + "_layer" + std::to_string(i);

        // 3Dオーディオかどうかで処理を分ける
        if (variedParams.is3D) {
            // 3Dオーディオソースが存在しない場合は作成
            if (!audioManager_->GetAudioSource3D(sourceName)) {
                audioManager_->Create3DAudioSource(sourceName, filePaths_[i]);
            }

            AudioSource3D* source = audioManager_->GetAudioSource3D(sourceName);
            if (source) {
                // 各種パラメータの設定
                source->SetVolume(variedParams.volume);
                source->SetPitch(variedParams.pitch);
                source->SetPosition(variedParams.position);
                source->SetVelocity(variedParams.velocity);
                source->SetDistance(variedParams.minDistance, variedParams.maxDistance);

                // フェードインの設定
                if (variedParams.fadeInTime > 0.0f) {
                    source->FadeIn(variedParams.fadeInTime);
                }
                else {
                    // 通常再生
                    source->Play(variedParams.loop);
                }

                // リストに追加
                audioSources3D_.push_back(source);
            }
        }
        else {
            // 通常のオーディオソースが存在しない場合は作成
            if (!audioManager_->GetAudioSource(sourceName)) {
                audioManager_->LoadAudioFile(sourceName, filePaths_[i]);
            }

            AudioSource* source = audioManager_->GetAudioSource(sourceName);
            if (source) {
                // 各種パラメータの設定
                source->SetVolume(variedParams.volume);
                source->SetPitch(variedParams.pitch);
                source->SetPan(variedParams.pan);

                // フェードインの設定
                if (variedParams.fadeInTime > 0.0f) {
                    source->FadeIn(variedParams.fadeInTime);
                }
                else {
                    // 通常再生
                    source->Play(variedParams.loop);
                }

                // リストに追加
                audioSources_.push_back(source);
            }
        }
    }
}

void LayeredAudioEvent::StopInternal() {
    // フェードアウトの設定
    if (params_.fadeOutTime > 0.0f) {
        for (auto source : audioSources_) {
            if (source) {
                source->FadeOut(params_.fadeOutTime);
            }
        }

        for (auto source : audioSources3D_) {
            if (source) {
                source->FadeOut(params_.fadeOutTime);
            }
        }
    }
    else {
        // 即時停止
        for (auto source : audioSources_) {
            if (source) {
                source->Stop();
            }
        }

        for (auto source : audioSources3D_) {
            if (source) {
                source->Stop();
            }
        }
    }

    // リソースのクリア
    audioSources_.clear();
    audioSources3D_.clear();
}

void LayeredAudioEvent::Cleanup() {
    // 必要に応じてクリーンアップ処理を実装
}