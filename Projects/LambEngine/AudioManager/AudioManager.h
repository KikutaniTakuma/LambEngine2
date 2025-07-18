/// ================================
/// ==  AudioManagerクラスの宣言  ==
/// ================================


#pragma once
#include <memory>
#include <unordered_map>
#include "Audio/Audio.h"
#include "Utils/SafePtr.h"
#include "Engine/EngineUtils/LambPtr/LambPtr.h"

/// <summary>
/// Audioを管理
/// </summary>
class AudioManager {
	friend Audio;

private:
	AudioManager();
	AudioManager(const AudioManager&) = delete;
	AudioManager(AudioManager&&) noexcept = delete;
	AudioManager& operator=(const AudioManager&) = delete;
	AudioManager& operator=(AudioManager&&) noexcept = delete;
public:
	~AudioManager();

public:
	static void Inititalize();
	static void Finalize();
	static inline AudioManager* const GetInstance() {
		return pInstance_.get();
	}

private:
	static Lamb::SafePtr<AudioManager> pInstance_;

public:
	/// <summary>
	/// ロードしてコンテナに追加(同じものは再ロードされない)
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Load(const std::string& fileName);

	/// <summary>
	/// ロードしたものを取ってくる(ロードしてなかったらエラーが起きる)
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	/// <returns>オーディオポインタ(デリートしてはいけない)</returns>
	Audio* const Get(const std::string& fileName);

	/// <summary>
	/// アンロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Unload(const std::string& fileName);
	/// <summary>
	/// アンロード
	/// </summary>
	/// <param name="audio">Audioポインタ</param>
	void Unload(Audio* audio);


	IXAudio2MasteringVoice* GetMasterVoice() {
		return pMasterVoice_.get();
	}

private:
	Lamb::LambPtr<IXAudio2> pxAudio2_;
	Lamb::SafePtr<IXAudio2MasteringVoice> pMasterVoice_;

	std::unordered_map<std::string, std::unique_ptr<Audio>> audios_;
};