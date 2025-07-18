/// ==================================
/// ==  AnimationModelクラスの宣言  ==
/// ==================================

#pragma once
#include "../Model/Model.h"
#include "Engine/Graphics/Animator/Animator.h"
#include "Engine/Graphics/Skeleton.h"

#include <memory>

/// <summary>
/// スキンアニメーションの描画クラス
/// </summary>
class AnimationModel : public Model {

public:
	AnimationModel() = default;
	AnimationModel(const std::string& fileName);
	AnimationModel(const AnimationModel&) = default;
	AnimationModel(AnimationModel&&) = default;
	virtual ~AnimationModel() = default;

	AnimationModel& operator=(const AnimationModel&) = default;
	AnimationModel& operator=(AnimationModel&&) = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイル</param>
	virtual void Load(const std::string& fileName) override;

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="camera">カメラ行列</param>
	/// <param name="color">色</param>
	/// <param name="blend">アルファブレンド</param>
	/// <param name="isLighting">ライティングするか(ブレンドタイプがkNoneだったらDeferredRenderingの設定依存)</param>
	virtual void Draw(
		const Mat4x4& worldMatrix,
		const Mat4x4& camera,
		uint32_t color,
		BlendType blend,
		Model::ShaderData shaderdata
	) override;

public:
	inline Animator& GetAnimator() {
		return *pAnimator_;
	}

private:
	std::unique_ptr<Animator> pAnimator_;

	std::unique_ptr<Skeleton> pSkeleton_;
	std::unique_ptr<SkinCluster> pSkinCluster_;
};