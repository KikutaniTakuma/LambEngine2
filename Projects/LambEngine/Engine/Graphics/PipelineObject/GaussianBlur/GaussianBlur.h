/// ================================
/// ==  GaussianBlurクラスの宣言  ==
/// ================================



#pragma once
#include "../PipelineObject.h"
#include "../PeraPipeline/PeraPipeline.h"
#include "Math/Vector2.h"
#include "Math/MathCommon.h"
#include "Engine/Graphics/VertexIndexDataManager/VertexIndexDataManager.h"
#include "Engine/Graphics/GraphicsStructs.h"

class GaussianBlur final : public PeraPipeline {
public:
	struct State {
		Vector2 dir;
		float32_t sigma;
		int32_t kernelSize;
	};
public:
	GaussianBlur() = default;
	GaussianBlur(const GaussianBlur&) = delete;
	GaussianBlur(GaussianBlur&&) = delete;
	~GaussianBlur();

	GaussianBlur& operator=(const GaussianBlur&) = delete;
	GaussianBlur& operator=(GaussianBlur&&) = delete;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Init(
		const std::string& vsShader = "./Shaders/PostShader/Post.VS.hlsl",
		const std::string& psShader = "./Shaders/PostShader/PostGaussian.PS.hlsl",
		std::initializer_list<DXGI_FORMAT> formtats = { DXGI_FORMAT_R32G32B32A32_FLOAT }
	) override;

public:
	/// <summary>
	/// リソースセット
	/// </summary>
	void Use(Pipeline::Blend blendType, bool isDepth) override;

	/// <summary>
	/// バッファにデータを設定する
	/// </summary>
	void SetData()override;

	void SetGaussianState(const State& gaussianBlurState) {
		gaussianBlurState_ = gaussianBlurState;
	}

	void Debug(const std::string& guiName);

private:
	std::array<std::unique_ptr<ConstantBuffer<State>>, DirectXSwapChain::kBackBufferNumber> gaussianBlurStateBuf_;
	State  gaussianBlurState_;
};
