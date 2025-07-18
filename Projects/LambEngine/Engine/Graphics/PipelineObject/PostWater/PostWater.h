/// =============================
/// ==  PostWaterクラスの宣言  ==
/// =============================


#pragma once
#include "../PipelineObject.h"
#include "../PeraPipeline/PeraPipeline.h"
#include "Math/Vector2.h"
#include "Math/MathCommon.h"
#include "Engine/Graphics/GraphicsStructs.h"

#include "Engine/Graphics/Tonemap/Tonemap.h"

class PostWater final : public PeraPipeline {
public:
	PostWater() = default;
	PostWater(const PostWater&) = delete;
	PostWater(PostWater&&) = delete;
	~PostWater();

	PostWater& operator=(const PostWater&) = delete;
	PostWater& operator=(PostWater&&) = delete;

public:
	void Init(
		const std::string& vsShader = "./Shaders/PostShader/Post.VS.hlsl",
		const std::string& psShader = "./Shaders/PostShader/PostWater.PS.hlsl",
		std::initializer_list<DXGI_FORMAT> formtats = { 
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			DXGI_FORMAT_R32G32B32A32_FLOAT 
		}
	) override;

public:
	void Use(Pipeline::Blend blendType, bool isDepth) override;

	/// <summary>
	/// レンダーターゲットのフォーマット設定
	/// </summary>
	/// <param name="format"></param>
	void SetRtvFormt(DXGI_FORMAT format);

	void SetDistortionTexHandle(D3D12_GPU_DESCRIPTOR_HANDLE distortionTexHandle) {
		distortionTexHandle_ = distortionTexHandle;
	}

	void SetDepthTexHandle(D3D12_GPU_DESCRIPTOR_HANDLE depthTexHandle) {
		depthTexHandle_ = depthTexHandle;
	}

	void SetCausticsTexHandle(D3D12_GPU_DESCRIPTOR_HANDLE causticsTexHandle) {
		causticsTexHandle_ = causticsTexHandle;
	}

	void SetWorldPositionTexHandle(D3D12_GPU_DESCRIPTOR_HANDLE worldPositionTexHandle) {
		worldPositionTexHandle_ = worldPositionTexHandle;
	}

	void SetTonemapParams(const TonemapParams& tonemapParams);
	void SetWaterWorldMatrixInverse(const float32_t4x4& waterWorldMatrixInverse);

private:
	std::array<std::unique_ptr<ConstantBuffer<float32_t4x4>>, DirectXSwapChain::kBackBufferNumber> scrollUVBuf_;
	std::array<std::unique_ptr<ConstantBuffer<float32_t>>, DirectXSwapChain::kBackBufferNumber> depthFloatBuf_;
	std::array<std::unique_ptr<ConstantBuffer<TonemapParams>>, DirectXSwapChain::kBackBufferNumber> tonemapParamasBuf_;
	std::array<std::unique_ptr<ConstantBuffer<float32_t4x4>>, DirectXSwapChain::kBackBufferNumber> waterInverseBuf_;
	float32_t4x4 scrollUV_;
	float32_t depthFloat_ = 0.423f;
	TonemapParams tonemapParamas_;
	float32_t4x4 waterWorldMatrixInverse_;

	Vector3 translate_;
	Vector3 uvScrollSpeed_ = Vector3::kXIdentity * 0.05f + Vector3::kYIdentity * 0.05f;

	DXGI_FORMAT format_ = DXGI_FORMAT_R32G32B32A32_FLOAT;
	D3D12_GPU_DESCRIPTOR_HANDLE distortionTexHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE depthTexHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE causticsTexHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE worldPositionTexHandle_ = {};
};
