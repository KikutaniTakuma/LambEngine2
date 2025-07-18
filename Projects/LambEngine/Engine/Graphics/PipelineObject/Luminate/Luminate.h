/// ============================
/// ==  Luminateクラスの宣言  ==
/// ============================


#pragma once
#include "../PipelineObject.h"
#include "../PeraPipeline/PeraPipeline.h"
#include "Math/Vector2.h"
#include "Math/MathCommon.h"
#include "Engine/Graphics/GraphicsStructs.h"

class Luminate final : public PeraPipeline {
public:
	Luminate() = default;
	Luminate(const Luminate&) = delete;
	Luminate(Luminate&&) = delete;
	~Luminate();

	Luminate& operator=(const Luminate&) = delete;
	Luminate& operator=(Luminate&&) = delete;

public:
	void Init(
		const std::string& vsShader = "./Shaders/PostShader/Post.VS.hlsl",
		const std::string& psShader = "./Shaders/PostShader/PostLuminate.PS.hlsl",
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

	void SetLuminanceThreshold(float32_t luminanceThreshold) {
		luminanceThreshold_ = luminanceThreshold;
	}

	void SetRtvFormt(DXGI_FORMAT format);

	void Debug();

private:
	std::array<std::unique_ptr<ConstantBuffer<float32_t>>, DirectXSwapChain::kBackBufferNumber> luminanceThresholdBuf_;
	float32_t luminanceThreshold_ = 0.0f;
	DXGI_FORMAT format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
};
