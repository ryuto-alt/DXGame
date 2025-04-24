// src/Engine/Audio/AudioManager.cpp
#include "AudioManager.h"
#include "WaveFile.h"
#include "Mp3File.h"
#include "AudioSource.h"
#include "AudioSource3D.h"
#include "AudioBus.h"
#include "AudioEvent.h"
#include <cassert>
#include <algorithm>
#include <filesystem>

// 静的メンバの実体化
AudioManager* AudioManager::instance = nullptr;
std::mt19937 AudioEvent::randomEngine_;

AudioManager* AudioManager::GetInstance() {
    if (!instance) {
        instance = new AudioManager();
    }
    return instance;
}

AudioManager::AudioManager()
    : masteringVoice(nullptr),
    is3DAudioInitialized(false),
    mfInitialized(false),
    masterVolume(1.0f),
    dopplerFactor(1.0f),
    speedOfSound(343.0f),
    isPaused(false) {

    // パフォーマンスカウンターの初期化
    QueryPerformanceFrequency(&performanceFrequency);
    QueryPerformanceCounter(&lastUpdateTime);
}

AudioManager::~AudioManager() {
    Finalize();
}

void AudioManager::Initialize() {
    // XAudio2の初期化
    UINT32 flags = 0;
#ifdef _DEBUG
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif

    HRESULT hr = XAudio2Create(xAudio2.GetAddressOf(), flags);
    assert(SUCCEEDED(hr));

    // マスターボイスの作成
    hr = xAudio2->CreateMasteringVoice(&masteringVoice);
    assert(SUCCEEDED(hr));

    // 3Dオーディオの初期化
    Initialize3DAudio();

    // Media Foundationの初期化
    hr = MFStartup(MF_VERSION);
    if (SUCCEEDED(hr)) {
        mfInitialized = true;
    }
    else {
        // MP3やAACなどの読み込みはできないが、WAVは読み込める
        mfInitialized = false;
    }
}

void AudioManager::Finalize() {
    // 全てのオーディオソースを停止
    for (auto& pair : audioSources) {
        pair.second->Stop();
    }

    // 全ての3Dオーディオソースを停止
    for (auto& pair : audioSources3D) {
        pair.second->Stop();
    }

    // 全てのオーディオイベントを停止
    for (auto& pair : audioEvents) {
        pair.second->Stop();
    }

    // オーディオバスのクリーンアップ
    audioBuses.clear();

    // オーディオイベントのクリーンアップ
    audioEvents.clear();

    // 3Dオーディオソースのクリーンアップ
    audioSources3D.clear();

    // オーディオソースのクリーンアップ
    audioSources.clear();
    playingSources.clear();

    // マスターボイスの解放
    if (masteringVoice) {
        masteringVoice->DestroyVoice();
        masteringVoice = nullptr;
    }

    // Media Foundationの終了処理
    if (mfInitialized) {
        MFShutdown();
        mfInitialized = false;
    }

    // XAudio2インターフェイスの解放は自動で行われる（ComPtr）
}

void AudioManager::Update() {
    // デルタ時間の計算
    float deltaTime = CalculateDeltaTime();

    // 再生中のソースを更新
    auto it = playingSources.begin();
    while (it != playingSources.end()) {
        AudioSource* source = *it;

        // フェード処理の更新
        if (source->IsFading()) {
            source->UpdateFade(deltaTime);
        }

        // 再生が終了したソースを検出して再生リストから削除
        if (!source->IsPlaying()) {
            it = playingSources.erase(it);
        }
        else {
            ++it;
        }
    }

    // 3Dオーディオソースの更新
    if (is3DAudioInitialized) {
        for (auto& pair : audioSources3D) {
            if (pair.second->IsPlaying()) {
                pair.second->Update3D(x3dAudioHandle, listener);
            }
        }
    }

    // オーディオイベントの更新
    for (auto& pair : audioEvents) {
        pair.second->Update(deltaTime);
    }
}

bool AudioManager::LoadWAV(const std::string& name, const std::string& filePath) {
    return LoadAudioFileImpl(name, filePath, AudioFileFormat::WAV);
}

bool AudioManager::LoadMP3(const std::string& name, const std::string& filePath) {
    // Media Foundationが初期化されているかチェック
    if (!mfInitialized) {
        return false; // MP3はサポートされていない
    }

    return LoadAudioFileImpl(name, filePath, AudioFileFormat::MP3);
}

bool AudioManager::LoadAudioFile(const std::string& name, const std::string& filePath) {
    // ファイル形式を判定
    AudioFileFormat format = GetAudioFileFormat(filePath);

    // 不明な形式の場合はエラー
    if (format == AudioFileFormat::Unknown) {
        return false;
    }

    return LoadAudioFileImpl(name, filePath, format);
}

bool AudioManager::LoadStreamingAudioFile(const std::string& name, const std::string& filePath) {
    // ファイル形式を判定
    AudioFileFormat format = GetAudioFileFormat(filePath);

    // 不明な形式の場合はエラー
    if (format == AudioFileFormat::Unknown) {
        return false;
    }

    return LoadAudioFileImpl(name, filePath, format, true);
}

bool AudioManager::Create3DAudioSource(const std::string& name, const std::string& filePath) {
    // 3Dオーディオが初期化されているかチェック
    if (!is3DAudioInitialized) {
        return false;
    }

    // ファイル形式を判定
    AudioFileFormat format = GetAudioFileFormat(filePath);

    // 不明な形式の場合はエラー
    if (format == AudioFileFormat::Unknown) {
        return false;
    }

    // 既に読み込み済みかチェック
    if (audioSources3D.find(name) != audioSources3D.end()) {
        return true; // 既に読み込み済み
    }

    // 3Dオーディオソースの作成
    std::unique_ptr<AudioSource3D> audioSource = std::make_unique<AudioSource3D>();

    // ファイル形式に応じた初期化
    bool result = false;

    if (format == AudioFileFormat::WAV) {
        // WAVファイルの読み込み
        std::unique_ptr<WaveFile> waveFile = std::make_unique<WaveFile>();
        if (waveFile->Load(filePath)) {
            // 3Dオーディオソースの初期化
            result = audioSource->Initialize(xAudio2.Get(), waveFile.get());

            if (result) {
                // 3Dオーディオのセットアップ
                XAUDIO2_VOICE_DETAILS details;
                masteringVoice->GetVoiceDetails(&details);
                audioSource->Setup3DAudio(x3dAudioHandle, details.InputChannels);
            }
        }
    }
    else if (format == AudioFileFormat::MP3 && mfInitialized) {
        // MP3ファイルの読み込み
        std::unique_ptr<Mp3File> mp3File = std::make_unique<Mp3File>();
        if (mp3File->Load(filePath)) {
            // 3Dオーディオソースの初期化
            result = audioSource->Initialize(xAudio2.Get(), mp3File.get());

            if (result) {
                // 3Dオーディオのセットアップ
                XAUDIO2_VOICE_DETAILS details;
                masteringVoice->GetVoiceDetails(&details);
                audioSource->Setup3DAudio(x3dAudioHandle, details.InputChannels);
            }
        }
    }

    // 初期化に成功したら登録
    if (result) {
        audioSources3D[name] = std::move(audioSource);
        return true;
    }

    return false;
}

bool AudioManager::CreateAudioBus(const std::string& name) {
    // 既に存在するかチェック
    if (audioBuses.find(name) != audioBuses.end()) {
        return true; // 既に存在する
    }

    // XAudio2が初期化されているかチェック
    if (!xAudio2) {
        return false;
    }

    // マスターボイスのチャンネル数を取得
    XAUDIO2_VOICE_DETAILS details;
    masteringVoice->GetVoiceDetails(&details);

    // オーディオバスの作成
    std::unique_ptr<AudioBus> audioBus = std::make_unique<AudioBus>(
        name, xAudio2.Get(), 2, details.InputChannels);

    // バスの登録
    audioBuses[name] = std::move(audioBus);

    return true;
}

bool AudioManager::CreateSimpleEvent(const std::string& name, const std::string& filePath, const AudioEventParams& params) {
    // 既に存在するかチェック
    if (audioEvents.find(name) != audioEvents.end()) {
        return true; // 既に存在する
    }

    // XAudio2が初期化されているかチェック
    if (!xAudio2) {
        return false;
    }

    // 3Dオーディオが必要かつ初期化されていないかチェック
    if (params.is3D && !is3DAudioInitialized) {
        return false;
    }

    // オーディオイベントの作成
    try {
        std::unique_ptr<SimpleAudioEvent> event = std::make_unique<SimpleAudioEvent>(
            name, filePath, params, this);

        // イベントの登録
        audioEvents[name] = std::move(event);

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool AudioManager::CreateRandomEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params) {
    // 既に存在するかチェック
    if (audioEvents.find(name) != audioEvents.end()) {
        return true; // 既に存在する
    }

    // XAudio2が初期化されているかチェック
    if (!xAudio2) {
        return false;
    }

    // 3Dオーディオが必要かつ初期化されていないかチェック
    if (params.is3D && !is3DAudioInitialized) {
        return false;
    }

    // オーディオイベントの作成
    try {
        std::unique_ptr<RandomAudioEvent> event = std::make_unique<RandomAudioEvent>(
            name, filePaths, params, this);

        // イベントの登録
        audioEvents[name] = std::move(event);

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool AudioManager::CreateSequentialEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params) {
    // 既に存在するかチェック
    if (audioEvents.find(name) != audioEvents.end()) {
        return true; // 既に存在する
    }

    // XAudio2が初期化されているかチェック
    if (!xAudio2) {
        return false;
    }

    // 3Dオーディオが必要かつ初期化されていないかチェック
    if (params.is3D && !is3DAudioInitialized) {
        return false;
    }

    // オーディオイベントの作成
    try {
        std::unique_ptr<SequentialAudioEvent> event = std::make_unique<SequentialAudioEvent>(
            name, filePaths, params, this);

        // イベントの登録
        audioEvents[name] = std::move(event);

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool AudioManager::CreateLayeredEvent(const std::string& name, const std::vector<std::string>& filePaths, const AudioEventParams& params) {
    // 既に存在するかチェック
    if (audioEvents.find(name) != audioEvents.end()) {
        return true; // 既に存在する
    }

    // XAudio2が初期化されているかチェック
    if (!xAudio2) {
        return false;
    }

    // 3Dオーディオが必要かつ初期化されていないかチェック
    if (params.is3D && !is3DAudioInitialized) {
        return false;
    }

    // オーディオイベントの作成
    try {
        std::unique_ptr<LayeredAudioEvent> event = std::make_unique<LayeredAudioEvent>(
            name, filePaths, params, this);

        // イベントの登録
        audioEvents[name] = std::move(event);

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

void AudioManager::Play(const std::string& name, bool looping) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        // 再生
        it->second->Play(looping);

        // 再生リストに追加
        AudioSource* source = it->second.get();
        if (std::find(playingSources.begin(), playingSources.end(), source) == playingSources.end()) {
            playingSources.push_back(source);
        }
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        // 再生
        it3D->second->Play(looping);

        // 再生リストに追加
        AudioSource* source = it3D->second.get();
        if (std::find(playingSources.begin(), playingSources.end(), source) == playingSources.end()) {
            playingSources.push_back(source);
        }
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        // 再生
        itEvent->second->Play();
        return;
    }
}

void AudioManager::Stop(const std::string& name) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->Stop();
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->Stop();
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        itEvent->second->Stop();
        return;
    }
}

void AudioManager::Pause(const std::string& name) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->Pause();
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->Pause();
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        itEvent->second->Pause();
        return;
    }
}

void AudioManager::Resume(const std::string& name) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->Resume();
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->Resume();
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        itEvent->second->Resume();
        return;
    }
}

void AudioManager::SetVolume(const std::string& name, float volume) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->SetVolume(volume);
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->SetVolume(volume);
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        itEvent->second->SetVolume(volume);
        return;
    }
}

void AudioManager::SetPitch(const std::string& name, float pitch) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->SetPitch(pitch);
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->SetPitch(pitch);
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        itEvent->second->SetPitch(pitch);
        return;
    }
}

void AudioManager::SetPan(const std::string& name, float pan) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->SetPan(pan);
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        // 3Dオーディオのパンは設定できない（位置で制御される）
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        itEvent->second->SetPan(pan);
        return;
    }
}

void AudioManager::FadeIn(const std::string& name, float duration, std::function<void()> completeCallback) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        // フェードイン
        it->second->FadeIn(duration, completeCallback);

        // 再生リストに追加
        AudioSource* source = it->second.get();
        if (std::find(playingSources.begin(), playingSources.end(), source) == playingSources.end()) {
            playingSources.push_back(source);
        }
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        // フェードイン
        it3D->second->FadeIn(duration, completeCallback);

        // 再生リストに追加
        AudioSource* source = it3D->second.get();
        if (std::find(playingSources.begin(), playingSources.end(), source) == playingSources.end()) {
            playingSources.push_back(source);
        }
        return;
    }

    // オーディオイベントは現状フェード未対応
}

void AudioManager::FadeOut(const std::string& name, float duration, std::function<void()> completeCallback) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        it->second->FadeOut(duration, completeCallback);
        return;
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->FadeOut(duration, completeCallback);
        return;
    }

    // オーディオイベントは現状フェード未対応
}

void AudioManager::SetPosition(const std::string& name, const Vector3& position) {
    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->SetPosition(position);
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end() && itEvent->second->GetParams().is3D) {
        itEvent->second->SetPosition(position);
        return;
    }
}

void AudioManager::SetVelocity(const std::string& name, const Vector3& velocity) {
    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        it3D->second->SetVelocity(velocity);
        return;
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end() && itEvent->second->GetParams().is3D) {
        itEvent->second->SetVelocity(velocity);
        return;
    }
}

void AudioManager::SetListenerPosition(const Vector3& position) {
    if (!is3DAudioInitialized) {
        return;
    }

    listener.Position = { position.x, position.y, position.z };
}

void AudioManager::SetListenerOrientation(const Vector3& forward, const Vector3& up) {
    if (!is3DAudioInitialized) {
        return;
    }

    listener.OrientFront = { forward.x, forward.y, forward.z };
    listener.OrientTop = { up.x, up.y, up.z };
}

void AudioManager::SetListenerVelocity(const Vector3& velocity) {
    if (!is3DAudioInitialized) {
        return;
    }

    listener.Velocity = { velocity.x, velocity.y, velocity.z };
}

void AudioManager::SetDopplerFactor(float factor) {
    dopplerFactor = factor;

    if (is3DAudioInitialized) {
        // X3DAudioInitializeを再度呼び出して設定を反映
        X3DAudioInitialize(SPEAKER_STEREO, dopplerFactor * speedOfSound, x3dAudioHandle);
    }
}

void AudioManager::SetSpeedOfSound(float speed) {
    speedOfSound = speed;

    if (is3DAudioInitialized) {
        // X3DAudioInitializeを再度呼び出して設定を反映
        X3DAudioInitialize(SPEAKER_STEREO, dopplerFactor * speedOfSound, x3dAudioHandle);
    }
}

void AudioManager::SetMasterVolume(float volume) {
    // 0.0f～1.0fの範囲に制限
    masterVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

    if (masteringVoice) {
        masteringVoice->SetVolume(masterVolume);
    }
}

float AudioManager::GetMasterVolume() const {
    return masterVolume;
}

void AudioManager::PauseAll() {
    // 既に一時停止中なら何もしない
    if (isPaused) {
        return;
    }

    // 全てのオーディオソースを一時停止
    for (const auto& source : playingSources) {
        if (source->IsPlaying() && !source->IsPaused()) {
            source->Pause();
        }
    }

    // 全てのオーディオイベントを一時停止
    for (auto& pair : audioEvents) {
        if (pair.second->IsPlaying() && !pair.second->IsPaused() &&
            !pair.second->GetParams().ignorePause) {
            pair.second->Pause();
        }
    }

    isPaused = true;
}

void AudioManager::ResumeAll() {
    // 一時停止中でなければ何もしない
    if (!isPaused) {
        return;
    }

    // 全てのオーディオソースを再開
    for (const auto& source : playingSources) {
        if (source->IsPlaying() && source->IsPaused()) {
            source->Resume();
        }
    }

    // 全てのオーディオイベントを再開
    for (auto& pair : audioEvents) {
        if (pair.second->IsPlaying() && pair.second->IsPaused()) {
            pair.second->Resume();
        }
    }

    isPaused = false;
}

void AudioManager::PlayGroup(const std::string& groupName, bool looping) {
    for (auto& pair : audioSources) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Play(looping);

            // 再生リストに追加
            AudioSource* source = pair.second.get();
            if (std::find(playingSources.begin(), playingSources.end(), source) == playingSources.end()) {
                playingSources.push_back(source);
            }
        }
    }

    for (auto& pair : audioSources3D) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Play(looping);

            // 再生リストに追加
            AudioSource* source = pair.second.get();
            if (std::find(playingSources.begin(), playingSources.end(), source) == playingSources.end()) {
                playingSources.push_back(source);
            }
        }
    }
}

void AudioManager::StopGroup(const std::string& groupName) {
    for (auto& pair : audioSources) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Stop();
        }
    }

    for (auto& pair : audioSources3D) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Stop();
        }
    }
}

void AudioManager::PauseGroup(const std::string& groupName) {
    for (auto& pair : audioSources) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Pause();
        }
    }

    for (auto& pair : audioSources3D) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Pause();
        }
    }
}

void AudioManager::ResumeGroup(const std::string& groupName) {
    for (auto& pair : audioSources) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Resume();
        }
    }

    for (auto& pair : audioSources3D) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->Resume();
        }
    }
}

void AudioManager::SetGroupVolume(const std::string& groupName, float volume) {
    for (auto& pair : audioSources) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->SetVolume(volume);
        }
    }

    for (auto& pair : audioSources3D) {
        if (pair.second->GetGroupName() == groupName) {
            pair.second->SetVolume(volume);
        }
    }
}

void AudioManager::PlayBus(const std::string& busName, bool looping) {
    auto it = audioBuses.find(busName);
    if (it != audioBuses.end()) {
        it->second->PlayAll(looping);
    }
}

void AudioManager::StopBus(const std::string& busName) {
    auto it = audioBuses.find(busName);
    if (it != audioBuses.end()) {
        it->second->StopAll();
    }
}

void AudioManager::PauseBus(const std::string& busName) {
    auto it = audioBuses.find(busName);
    if (it != audioBuses.end()) {
        it->second->PauseAll();
    }
}

void AudioManager::ResumeBus(const std::string& busName) {
    auto it = audioBuses.find(busName);
    if (it != audioBuses.end()) {
        it->second->ResumeAll();
    }
}

void AudioManager::SetBusVolume(const std::string& busName, float volume) {
    auto it = audioBuses.find(busName);
    if (it != audioBuses.end()) {
        it->second->SetVolume(volume);
    }
}

void AudioManager::PlayEvent(const std::string& eventName) {
    auto it = audioEvents.find(eventName);
    if (it != audioEvents.end()) {
        it->second->Play();
    }
}

void AudioManager::StopEvent(const std::string& eventName) {
    auto it = audioEvents.find(eventName);
    if (it != audioEvents.end()) {
        it->second->Stop();
    }
}

void AudioManager::PauseEvent(const std::string& eventName) {
    auto it = audioEvents.find(eventName);
    if (it != audioEvents.end()) {
        it->second->Pause();
    }
}

void AudioManager::ResumeEvent(const std::string& eventName) {
    auto it = audioEvents.find(eventName);
    if (it != audioEvents.end()) {
        it->second->Resume();
    }
}

void AudioManager::SetEventVolume(const std::string& eventName, float volume) {
    auto it = audioEvents.find(eventName);
    if (it != audioEvents.end()) {
        it->second->SetVolume(volume);
    }
}

bool AudioManager::IsPlaying(const std::string& name) {
    // 通常のオーディオソースを検索
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        return it->second->IsPlaying();
    }

    // 3Dオーディオソースを検索
    auto it3D = audioSources3D.find(name);
    if (it3D != audioSources3D.end()) {
        return it3D->second->IsPlaying();
    }

    // オーディオイベントを検索
    auto itEvent = audioEvents.find(name);
    if (itEvent != audioEvents.end()) {
        return itEvent->second->IsPlaying();
    }

    return false;
}

AudioSource* AudioManager::GetAudioSource(const std::string& name) {
    auto it = audioSources.find(name);
    if (it != audioSources.end()) {
        return it->second.get();
    }
    return nullptr;
}

AudioSource3D* AudioManager::GetAudioSource3D(const std::string& name) {
    auto it = audioSources3D.find(name);
    if (it != audioSources3D.end()) {
        return it->second.get();
    }
    return nullptr;
}

AudioBus* AudioManager::GetAudioBus(const std::string& name) {
    auto it = audioBuses.find(name);
    if (it != audioBuses.end()) {
        return it->second.get();
    }
    return nullptr;
}

AudioEvent* AudioManager::GetAudioEvent(const std::string& name) {
    auto it = audioEvents.find(name);
    if (it != audioEvents.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool AudioManager::Initialize3DAudio() {
    // XAudio2が初期化されているかチェック
    if (!xAudio2 || !masteringVoice) {
        return false;
    }

    // X3DAudioの初期化
    HRESULT hr = X3DAudioInitialize(SPEAKER_STEREO, dopplerFactor * speedOfSound, x3dAudioHandle);
    if (FAILED(hr)) {
        return false;
    }

    // リスナーの初期化
    ZeroMemory(&listener, sizeof(X3DAUDIO_LISTENER));
    listener.OrientFront = { 0.0f, 0.0f, 1.0f }; // 前向き（Z+方向）
    listener.OrientTop = { 0.0f, 1.0f, 0.0f };   // 上向き（Y+方向）

    is3DAudioInitialized = true;
    return true;
}

AudioFileFormat AudioManager::GetAudioFileFormat(const std::string& filePath) {
    // 拡張子を取得
    std::string extension = std::filesystem::path(filePath).extension().string();

    // 小文字に変換
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // 拡張子に応じた形式を返す
    if (extension == ".wav") {
        return AudioFileFormat::WAV;
    }
    else if (extension == ".mp3") {
        return AudioFileFormat::MP3;
    }

    return AudioFileFormat::Unknown;
}

bool AudioManager::LoadAudioFileImpl(const std::string& name, const std::string& filePath, AudioFileFormat format, bool streaming) {
    // 既に読み込み済みかチェック
    if (audioSources.find(name) != audioSources.end()) {
        return true; // 既に読み込み済み
    }

    // オーディオソースの作成
    std::unique_ptr<AudioSource> audioSource = std::make_unique<AudioSource>();

    // ストリーミングモードの場合
    if (streaming) {
        if (audioSource->InitializeStreaming(xAudio2.Get(), filePath)) {
            audioSources[name] = std::move(audioSource);
            return true;
        }
        return false;
    }

    // ファイル形式に応じた読み込み
    bool result = false;

    if (format == AudioFileFormat::WAV) {
        // WAVファイルの読み込み
        std::unique_ptr<WaveFile> waveFile = std::make_unique<WaveFile>();
        if (waveFile->Load(filePath)) {
            result = audioSource->Initialize(xAudio2.Get(), waveFile.get());
        }
    }
    else if (format == AudioFileFormat::MP3 && mfInitialized) {
        // MP3ファイルの読み込み
        std::unique_ptr<Mp3File> mp3File = std::make_unique<Mp3File>();
        if (mp3File->Load(filePath)) {
            result = audioSource->Initialize(xAudio2.Get(), mp3File.get());
        }
    }

    // 初期化に成功したら登録
    if (result) {
        audioSources[name] = std::move(audioSource);
        return true;
    }

    return false;
}

float AudioManager::CalculateDeltaTime() {
    // 現在時刻を取得
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    // 前回からの経過時間を計算
    float deltaTime = static_cast<float>(currentTime.QuadPart - lastUpdateTime.QuadPart) /
        static_cast<float>(performanceFrequency.QuadPart);

    // 現在時刻を記録
    lastUpdateTime = currentTime;

    return deltaTime;
}