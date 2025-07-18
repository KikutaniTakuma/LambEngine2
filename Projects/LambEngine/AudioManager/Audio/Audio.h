/// =========================
/// ==  Audioクラスの宣言  ==
/// =========================


#pragma once
#include <array>
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")


#include <string>
#include "Utils/SafePtr.h"

/// <summary>
/// 音の再生や停止、ロードを担う
/// </summary>
class Audio {
	friend class AudioManager;

private:
	struct ChunkHeader {
		std::array<char,4> id_;
		int32_t size_;
	};
	struct RiffHeader {
		ChunkHeader chunk_;
		std::array<char, 4> type_;
	};
	struct FormatChunk
	{
		ChunkHeader chunk_;
		WAVEFORMATEX fmt_;
	};

public:
	Audio();
	Audio(const Audio&) = delete;
	Audio(Audio&&) = delete;
	~Audio();

	Audio& operator=(const Audio&) = delete;
	Audio& operator=(Audio&&) = delete;

public:
	/// <summary>
	/// 最初から再生
	/// </summary>
	/// <param name="volume">音の大きさ(0.0f～1.0f)</param>
	/// <param name="isLoop">ループするか否か</param>
	void Start(float volume, bool isLoop);

	/// <summary>
	/// 一時停止
	/// </summary>
	void Pause();

	/// <summary>
	/// 一時停止した場所から再生
	/// </summary>
	void ReStart();

	/// <summary>
	/// 止める
	/// </summary>
	void Stop();

	/// <summary>
	/// 再生しているか
	/// </summary>
	/// <returns></returns>
	bool IsStart() const {
		return isStart_;
	}

	/// <summary>
	/// 音の大きさ設定
	/// </summary>
	/// <param name="volume">音の大きさ(0.0f～1.0f)</param>
	void SetVolume(float volume);

	/// <summary>
	/// ImGui
	/// </summary>
	void Debug(const std::string& guiName);

private:
	/// <summary>
	/// アンロード
	/// </summary>
	void Unload();

	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Load(const std::string& fileName);

private:
	WAVEFORMATEX wfet_;
	Lamb::SafePtr<BYTE> pBuffer_;
	uint32_t bufferSize_;
	IXAudio2SourceVoice* pSourceVoice_;

	bool isLoop_;
	bool isStart_;
	float volume_;

	bool isLoad_;
	std::string fileName_;
};