/// ==============================
/// ==  FrameInfoクラスの宣言  ==
/// ==============================


#pragma once
#include <chrono>
#include <queue>
#include "Drawer/StringOut/StringOut.h"

/// <summary>
/// フレーム情報の管理(fps固定、デルタタイム、平均fps)
/// </summary>
class FrameInfo {
/// <summary>
/// コンストラクタ
/// </summary>
private:
	FrameInfo();
	FrameInfo(const FrameInfo&) = delete;
	FrameInfo(FrameInfo&&) = delete;
	~FrameInfo();

	FrameInfo& operator=(const FrameInfo&) = delete;
	FrameInfo& operator=(FrameInfo&&) = delete;

/// <summary>
/// シングルトン
/// </summary>
public:
	/// <summary>
	/// インスタンス取得
	/// </summary>
	/// <returns>インスタンスのポインタ(deleteしてはいけない)</returns>
	static FrameInfo* const GetInstance();

/// <summary>
/// メンバ関数
/// </summary>
public:
	/// <summary>
	/// フレーム開始時関数
	/// </summary>
	void Start();

	/// <summary>
	/// フレーム終了時関数
	/// </summary>
	void End();

	/// <summary>
	/// フレームインフォ開始
	/// </summary>
	void StartFrameInfo();

/// <summary>
/// getter
/// </summary>
public:
	/// <summary>
	/// fps描画
	/// </summary>
	void DrawFps();

	/// <summary>
	/// fps描画切り替え
	/// </summary>
	void SwitchDarwFlg();

#ifdef USE_DEBUG_CODE
	bool GetIsDebugStop() const {
		return isDebugStopGame_;
	}

	bool GetIsOneFrameActive() const {
		return isOneFrameActive_;
	}

	void SetIsOneFrameActive(bool isOneFramActive) {
		if (isDebugStopGame_) {
			isOneFrameActive_ = isOneFramActive;
		}
	}
#endif // USE_DEBUG_CODE

	/// <summary>
	/// デバッグ関数
	/// </summary>
	void Debug();

/// <summary>
/// getter
/// </summary>
public:
	/// <summary>
	/// デルタタイム取得
	/// </summary>
	/// <returns>デルタタイム</returns>
	inline float GetDelta() const {
#ifdef USE_DEBUG_CODE
		if (isFixedDeltaTime_ || isDebugStopGame_) {
			return 1.0f / fpsLimit_;
		}
		return deltaTime_;
#else
		return deltaTime_;
#endif
	}

	/// <summary>
	/// fps取得
	/// </summary>
	/// <returns>fps</returns>
	inline float GetFps() const {
		return fps_;
	}

	/// <summary>
	/// フレームカウント取得
	/// </summary>
	/// <returns>フレームカウント</returns>
	inline size_t GetFrameCount() const {
		return frameCount_;
	}

	/// <summary>
	/// ゲームスピード取得
	/// </summary>
	/// <returns></returns>
	inline float GetGameSpeedScale() const {
		return gameSpeedSccale_;
	}

	/// <summary>
	/// フレームの最初の時間を取得
	/// </summary>
	/// <returns>フレームの時間</returns>
	std::chrono::steady_clock::time_point GetThisFrameTime() const {
		return frameStartTime_;
	}

	/// <summary>
	/// メインモニターのリフレッシュレートを取得
	/// </summary>
	/// <returns>リフレッシュレート</returns>
	float GetMainMonitorFramerate() const;

	float GetMaxFpsLimit() const;


/// <summary>
/// セッター
/// </summary>
public:
	/// <summary>
	/// fpsの上限値を設定(メインモニターのリフレッシュレートを超えることはない)
	/// </summary>
	/// <param name="fpsLimit">fps上限値</param>
	void SetFpsLimit(float fpsLimit);

	/// <summary>
	/// ゲームスピードのスケールを変更
	/// </summary>
	void SetGameSpeedScale(float gameSpeedSccale);



/// <summary>
/// メンバ変数
/// </summary>
private:
	const float kMaxMonitorFps_;

	std::chrono::steady_clock::time_point frameStartTime_;
	float deltaTime_;
	float fps_;
	float maxFps_;
	float minFps_;
	size_t frameCount_;

	std::chrono::steady_clock::time_point gameStartTime_;

	std::chrono::steady_clock::time_point reference_;
	float fpsLimit_;
	float maxFpsLimit_;

	std::chrono::microseconds minTime_;
	std::chrono::microseconds minCheckTime_;

	float gameSpeedSccale_;

	std::queue<float> frameDatas_;
	std::chrono::seconds frameDataDuration_;
	std::chrono::steady_clock::time_point frameDataDurationStartTime_;
	size_t avgProcDuration_;

	StringOut fpsStringOutPut_;
	bool isDrawFps_;

	bool isStartFrameInfo_;
	

#ifdef USE_DEBUG_CODE
	bool isDebugStopGame_;
	bool isOneFrameActive_;
	bool isFixedDeltaTime_;

	std::array<float, 180> frameRateData_ = {0.0f};
#endif // USE_DEBUG_CODE

};