/// ============================
/// ==  Animatorクラスの宣言  ==
/// ============================


#pragma once
#include <string>
#include "Math/Matrix.h"
#include "Engine/Graphics/GraphicsStructs.h"
#include "Engine/Graphics/Skeleton.h"
#include "Utils/Flg.h"

class Animator {
public:
	Animator();
	Animator(const Animator&)= default;
	Animator(Animator&&) = default;
	~Animator() = default;

	Animator& operator=(const Animator&) = default;
	Animator& operator=(Animator&&) = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Load(const std::string& fileName);

/// <summary>
/// 更新
/// </summary>
public:
	void Update(const VertexIndexData* const mesh);
	void Update(Skeleton& skeleton);
	void Update(const std::string& rootNodeName);

	void Debug(const std::string& guiName);

public:
	// 最初からスタート
	void Start();
	
	// 止めてリセット
	void Stop();

	// リセット
	void Reset();

	// スタート
	void Restart();

	// 一時停止
	void Pause();

	const Lamb::Flg& GetIsActive() const {
		return isActive_;
	}

public:
	/// <summary>
	/// アニメーションのマトリックスを取得
	/// </summary>
	/// <returns></returns>
	[[nodiscard]] const Mat4x4& GetLocalMat4x4() const;

	/// <summary>
	/// アニメーションを全て再生するか
	/// </summary>
	/// <param name="isFullAnimation"></param>
	void SetIsFullAnimation(bool isFullAnimation);

	/// <summary>
	/// <para>再生するアニメーションのインデックス</para>
	/// <para>0～要素数にクランプされる</para>
	/// </summary>
	/// <param name="index"></param>
	void SetAnimationIndex(size_t index);

	/// <summary>
	/// <para>アニメーションをループさせるか</para>
	/// <para>isFullAnimationがfalseの場合、設定されたindexのアニメーションでループする</para>
	/// </summary>
	/// <param name="isLoop"></param>
	void SetLoopAnimation(bool isLoop);

	/// <summary>
	/// アニメーション速度を設定する
	/// </summary>
	/// <param name="speed">アニメーション速度</param>
	void SetAnimationSpeed(float speed);
	
	/// <summary>
	/// 新しいアニメーションを設定する
	/// </summary>
	/// <param name="animations">アニメーション</param>
	void SetAnimations(struct Animations* const animations);

private:
	Vector3 CalaclateValue_(const AnimationCurve<Vector3>& animationCurve, float time);
	Quaternion CalaclateValue_(const AnimationCurve<Quaternion>& animationCurve, float time);

private:
	float animationTime_;
	float animationSpeed_;
	struct Animations* animations_;
	Mat4x4 animationMatrix_;
	size_t currentAnimationIndex_;

	Lamb::Flg isActive_;

	bool isFullAnimation_;
	bool isLoop_;
};