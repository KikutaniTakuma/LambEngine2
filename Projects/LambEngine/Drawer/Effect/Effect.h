#pragma once
#include <Effekseer.h>

#include <string>

#include <Transform/Transform.h>

/// <summary>
/// エフェクト制御クラス
/// </summary>
class Effect {
public:
	Effect();
	Effect(const Effect&) = default;
	Effect(Effect&&) = default;
	~Effect() = default;

	Effect& operator=(const Effect&) = default;
	Effect& operator=(Effect&&) = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Load(const std::string& fileName);

	/// <summary>
	/// エフェクトの更新
	/// </summary>
	void Update();

	/// <summary>
	/// エフェクト再生
	/// </summary>
	void Play();

	/// <summary>
	/// エフェクト停止
	/// </summary>
	void Stop();

	/// <summary>
	/// 一時停止
	/// </summary>
	void Pause();

	/// <summary>
	/// 一時停止したものを途中から再生する
	/// </summary>
	void Restart();

	/// <summary>
	/// ループ設定
	/// </summary>
	/// <param name="isLoop">ループする場合はtrue</param>
	void SetIsLoop(bool isLoop);

	/// <summary>
	/// 色設定
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(uint32_t color);

	/// <summary>
	/// 色設定
	/// </summary>
	/// <param name="color">色</param>
	void SetColor(Vector4 color);

	/// <summary>
	/// 今再生しているか
	/// </summary>
	/// <returns>再生していたらtrue</returns>
	bool IsPlay();

	/// <summary>
	/// デバッグ用
	/// </summary>
	/// <param name="guiName"></param>
	void Debug(const std::string& guiName);

private:
	Effekseer::Color InvertEfkColor_(Effekseer::Color color);

public:
	QuaternionTransform transform;

private:
	::Effekseer::EffectRef effect_;
	::Effekseer::ManagerRef efkManager_;
	::Effekseer::Handle playHandle_;
	bool isLoop_ = false;
	bool isStart_ = false;

	union Color {
		uint32_t uint;
		Effekseer::Color efk;
	} color_;

#ifdef USE_DEBUG_CODE
	Vector4 debugColor_;
#endif // USE_DEBUG_CODE

};