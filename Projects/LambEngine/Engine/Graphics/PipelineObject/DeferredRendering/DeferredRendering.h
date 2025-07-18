/// ====================================
/// ==  DeferredRederingクラスの宣言  ==
/// ====================================


#pragma once

#pragma once
#include "../PipelineObject.h"
#include "../PeraPipeline/PeraPipeline.h"
#include "Math/Vector2.h"
#include "Math/Matrix.h"
#include "Math/MathCommon.h"
#include "Engine/Graphics/GraphicsStructs.h"
#include "Drawer/AirSkyBox/AirSkyBox.h"

class DeferredRendering final : public PipelineObject {
public:
	struct DeferredRenderingData {
		Vector3 eyePos;
		uint32_t rightNum = 0;
		DirectionLight directionLight;
		uint32_t isDirectionLight = 1u;
		uint32_t isShadow = 0u;
		float32_t environmentCoefficient = 1.0f;
	};
public:
	DeferredRendering() = default;
	DeferredRendering(const DeferredRendering&) = delete;
	DeferredRendering(DeferredRendering&&) = delete;
	~DeferredRendering();

	DeferredRendering& operator=(const DeferredRendering&) = delete;
	DeferredRendering& operator=(DeferredRendering&&) = delete;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Init(
		const std::string& vsShader = "./Shaders/DeferredRendering/DeferredRendering.VS.hlsl",
		const std::string& psShader = "./Shaders/DeferredRendering/DeferredRendering.PS.hlsl",
		std::initializer_list<DXGI_FORMAT> formtats = { DXGI_FORMAT_R32G32B32A32_FLOAT }
	) override;

public:
	/// <summary>
	/// リソースセット
	/// </summary>
	void Use(Pipeline::Blend blendType, bool isDepth) override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

/// <summary>
/// セッター
/// </summary>
public:
	void SetDeferredRenderingData(const DeferredRenderingData& deferredRenderingData) {
		deferredRenderingData_ = deferredRenderingData;
	}
	void SetAtmosphericParams(const AirSkyBox::AtmosphericParams& atomosphericData) {
		atomosphericData_ = atomosphericData;
	}


	void SetColorHandle(D3D12_GPU_DESCRIPTOR_HANDLE colorTextureHandle) {
		colorTextureHandle_ = colorTextureHandle;
	}
	void SetNormalHandle(D3D12_GPU_DESCRIPTOR_HANDLE normalTextureHandle) {
		normalTextureHandle_ = normalTextureHandle;
	}
	void SetWoprldPositionHandle(D3D12_GPU_DESCRIPTOR_HANDLE worldPositionTextureHandle) {
		worldPositionTextureHandle_ = worldPositionTextureHandle;
	}
	void SetDistortionHandle(D3D12_GPU_DESCRIPTOR_HANDLE distortionTextureHandle) {
		distortionTextureHandle_ = distortionTextureHandle;
	}

	void SetCameraMatrix(const float32_t4x4& camera);
	void SetLightCameraMatrix(const float32_t4x4& lightCamera);

private:
	std::array<std::unique_ptr<ConstantBuffer<DeferredRenderingData>>, DirectXSwapChain::kBackBufferNumber> deferredRenderingDataBuf_;
	std::array<std::unique_ptr<ConstantBuffer<AirSkyBox::AtmosphericParams>>, DirectXSwapChain::kBackBufferNumber> atomosphericDataBuf_;
	std::array<std::unique_ptr<ConstantBuffer<float32_t4x4>>, DirectXSwapChain::kBackBufferNumber> cameraBuf_;
	std::array<std::unique_ptr<ConstantBuffer<float32_t4x4>>, DirectXSwapChain::kBackBufferNumber> lightCameraBuf_;

	DeferredRenderingData deferredRenderingData_;
	AirSkyBox::AtmosphericParams atomosphericData_;

	float32_t4x4 cameraMatrix_;
	float32_t4x4 lightCameraMatrix_;

	D3D12_GPU_DESCRIPTOR_HANDLE colorTextureHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE normalTextureHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE worldPositionTextureHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE distortionTextureHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE environmentTextureHandle_ = {};
};
