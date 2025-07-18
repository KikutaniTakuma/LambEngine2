/// ===================================
/// ==  ShadowRenderingクラスの宣言  ==
/// ===================================



#pragma once
#include "../PipelineObject.h"
#include "../PeraPipeline/PeraPipeline.h"
#include "Math/Vector2.h"
#include "Math/MathCommon.h"
#include "Engine/Graphics/GraphicsStructs.h"
#include "Drawer/AirSkyBox/AirSkyBox.h"

class ShadowRendering final : public PipelineObject {
public:
	struct DeferredRenderingData {
		Vector3 eyePos;
		uint32_t rightNum = 0;
		DirectionLight directionLight;
		uint32_t isDirectionLight = 1u;
		float32_t environmentCoefficient = 1.0f;
	};
public:
	ShadowRendering() = default;
	ShadowRendering(const ShadowRendering&) = delete;
	ShadowRendering(ShadowRendering&&) = delete;
	~ShadowRendering();

	ShadowRendering& operator=(const ShadowRendering&) = delete;
	ShadowRendering& operator=(ShadowRendering&&) = delete;

public:
	void Init(
		const std::string& vsShader = "./Shaders/ShadowRendering/ShadowRendering.VS.hlsl",
		const std::string& psShader = "./Shaders/ShadowRendering/ShadowRendering.PS.hlsl",
		std::initializer_list<DXGI_FORMAT> formtats = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB }
	) override;

public:
	void Use(Pipeline::Blend blendType, bool isDepth) override;

	/// <summary>
	///	描画
	/// </summary>
	void Draw();

	void SetColorHandle(D3D12_GPU_DESCRIPTOR_HANDLE colorTextureHandle) {
		colorTextureHandle_ = colorTextureHandle;
	}
	void SetDepthHandle(D3D12_GPU_DESCRIPTOR_HANDLE depthTextureHandle) {
		depthTextureHandle_ = depthTextureHandle;
	}
	void SetDepthShadowHandle(D3D12_GPU_DESCRIPTOR_HANDLE depthShadowTextureHandle) {
		depthShadowTextureHandle_ = depthShadowTextureHandle;
	}

private:
	D3D12_GPU_DESCRIPTOR_HANDLE colorTextureHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE depthTextureHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE depthShadowTextureHandle_ = {};
};
