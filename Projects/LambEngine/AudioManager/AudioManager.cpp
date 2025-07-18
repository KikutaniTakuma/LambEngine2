/// ================================
/// ==  AudioManagerクラスの定義  ==
/// ================================

#include "AudioManager.h"
#include "Utils/ExecutionLog.h"
#include "Utils/EngineInfo.h"
#include <filesystem>
#include "Error/Error.h"
#include "Utils/SafeDelete.h"

Lamb::SafePtr<AudioManager> AudioManager::pInstance_ = nullptr;
void AudioManager::Inititalize() {
	pInstance_.reset(new AudioManager());
}
void AudioManager::Finalize() {
	pInstance_.reset();
}

AudioManager::AudioManager() :
	pxAudio2_(),
	pMasterVoice_(nullptr),
	audios_{}
{
	// Media Foundationの初期化
	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);


	HRESULT hr = XAudio2Create(pxAudio2_.GetAddressOf(), 0u, XAUDIO2_DEFAULT_PROCESSOR);
	if (not SUCCEEDED(hr)) {
		throw Lamb::Error::Code<AudioManager>("XAudio2Create()", ErrorPlace);
	}

	hr = pxAudio2_->CreateMasteringVoice(&pMasterVoice_);
	if (not SUCCEEDED(hr)) {
		throw Lamb::Error::Code<AudioManager>("CreateMasteringVoicey()", ErrorPlace);
	}

	Lamb::AddLog("Initialize AudioManager succeeded");
}
AudioManager::~AudioManager() {
	pxAudio2_.Reset();
	MFShutdown();
	Lamb::AddLog("Finalize AudioManager succeeded");
}

void AudioManager::Load(const std::string& fileName) {
	if (!std::filesystem::exists(std::filesystem::path(fileName))) {
		throw Lamb::Error::Code<AudioManager>("There is not this file -> " + fileName, ErrorPlace);
	}


	auto itr = audios_.find(fileName);

	if (itr == audios_.end()) {
		auto audio = std::make_unique<Audio>();
		audio->Load(fileName);
		audios_.insert({ fileName, std::move(audio) });

	}
}

Audio* const AudioManager::Get(const std::string& fileName)
{
	auto itr = audios_.find(fileName);

	if (itr == audios_.end()) {
		throw Lamb::Error::Code<AudioManager>("This file is not loaded -> " + fileName, ErrorPlace);
	}

	return itr->second.get();
}

void AudioManager::Unload(const std::string& fileName)
{
	auto isExisit = audios_.find(fileName);

	if (isExisit != audios_.end()) {
		audios_.erase(isExisit);
	}
}

void AudioManager::Unload(Audio* audio)
{
	if (!audio) {
		return;
	}

	auto isExisit = audios_.find(audio->fileName_);

	if (isExisit != audios_.end()) {
		audios_.erase(isExisit);
	}
}