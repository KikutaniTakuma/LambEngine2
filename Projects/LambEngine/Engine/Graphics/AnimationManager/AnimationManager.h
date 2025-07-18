/// ====================================
/// ==  AnimationManagerクラスの宣言  ==
/// ====================================

#pragma once
#include "../GraphicsStructs.h"

#include <unordered_map>
#include <memory>

#include "Utils/SafePtr.h"

class AnimationManager {
private:
	AnimationManager() = default;
	
	AnimationManager(const AnimationManager&);
	AnimationManager(AnimationManager&&);
	
	AnimationManager& operator=(const AnimationManager&) = delete;
	AnimationManager& operator=(AnimationManager&&) = delete;

public:
	~AnimationManager() = default;

public:
	static void Initialize();

	static void Finalize();

 	static [[nodiscard]] AnimationManager* const GetInstance();

private:
	static Lamb::SafePtr<AnimationManager> pInstance_;

public:
	/// <summary>
	/// アニメーションファイル読み込み
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void LoadAnimations(const std::string& fileName);
	
public:
	[[nodiscard]] Animations* const GetAnimations(const std::string& fileName);

private:
	std::unordered_map<std::string, std::unique_ptr<Animations>> animationData_;
};