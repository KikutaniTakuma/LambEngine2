/// ===========================
/// ==  Outlineクラスの宣言  ==
/// ===========================



#pragma once
#include "../PeraPipeline/PeraPipeline.h"
#include "Math/MathCommon.h"
#include "Engine/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/DepthBuffer/DepthBuffer.h"

class Outline final : public PeraPipeline {
public:
	struct OutlineData{
		float32_t weight = 1.0f;
		float32_t4x4 projectionInverse;
	};

public:
	Outline() = default;
	Outline(const Outline&) = delete;
	Outline(Outline&&) = delete;
	~Outline();

	Outline& operator=(const Outline&) = delete;
	Outline& operator=(Outline&&) = delete;

public:
	void Init(
		const std::string& vsShader = "./Shaders/PostShader/Post.VS.hlsl",
		const std::string& psShader = "./Shaders/PostShader/PostOutline.PS.hlsl",
		std::initializer_list<DXGI_FORMAT> formtats = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB }
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

	void Debug(const std::string& guiName);

	void SetWeight(float32_t weight);
	void SetProjectionInverse(const float32_t4x4& projectionInverse);

	void ChangeDepthBufferState();

	void SetDepthBuffer(DepthBuffer* depthBuffer);

private:
	std::array<std::unique_ptr<ConstantBuffer<OutlineData>>, DirectXSwapChain::kBackBufferNumber> outlineDataBuf_;
	OutlineData outlineData_;

	Lamb::SafePtr<DepthBuffer> depthBuffer_;
};
