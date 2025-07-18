/// =============================
/// ==  AirSkyBoxクラスの宣言  ==
/// =============================

#pragma once
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Math/Matrix.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/MathCommon.h"
#include "Engine/Buffer/ConstantBuffer.h"
#include "Utils/EngineInfo.h"


/// <summary>
/// 大気散乱のSkyBox
/// </summary>
class AirSkyBox {
public:
	struct ShaderData {
		Mat4x4 worldMat;
		Mat4x4 viewProjectionMat;
		Vector4 color = 0xffffffff;
	};

	struct AtmosphericParams {
		float32_t3 cameraPosition;    // カメラの位置
		float32_t pad = 0.0f;                // パディング
		float32_t3 lightDirection = -float32_t3::kYIdentity;    // 太陽光の方向（正規化ベクトル）
		float32_t rayleighScattering = 0.0025f; // Rayleigh散乱係数
		float32_t mieScattering = 0.001f;      // Mie散乱係数
		float32_t mieG = -0.999f;               // Mie散乱位相関数のg値（0から1の範囲）
	};

public:
	AirSkyBox() = default;
	AirSkyBox(const AirSkyBox&) = delete;
	AirSkyBox(AirSkyBox&&) = delete;
	~AirSkyBox();

public:
	AirSkyBox& operator=(const AirSkyBox&) = delete;
	AirSkyBox& operator=(AirSkyBox&&) = delete;

public:
	/// <summary>
	/// ロード
	/// </summary>
	void Load();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMat">ワールド行列</param>
	/// <param name="cameraMat">カメラ行列</param>
	/// <param name="color">色</param>
	void Draw(const Mat4x4& worldMat, const Mat4x4& cameraMat, uint32_t color);

	/// <summary>
	/// 大気散乱の値設定
	/// </summary>
	/// <param name="atmosphericParams"></param>
	void SetAtmosphericParams(const AtmosphericParams& atmosphericParams);

private:
	/// <summary>
	/// パイプライン生成
	/// </summary>
	void CreateGraphicsPipeline();

private:
	D3D12_VERTEX_BUFFER_VIEW vertexView_ = {};
	Lamb::LambPtr<ID3D12Resource> pVertexResource_;

	D3D12_INDEX_BUFFER_VIEW indexView_ = {};
	Lamb::LambPtr<ID3D12Resource> pIndexResource_;

	static constexpr uint32_t kIndexNumber_ = 36u;
	std::array<std::unique_ptr<ConstantBuffer<ShaderData>>, DirectXSwapChain::kBackBufferNumber> shaderData_;
	std::array<std::unique_ptr<ConstantBuffer<AtmosphericParams>>, DirectXSwapChain::kBackBufferNumber> atmosphericParams_;
	Lamb::SafePtr<class Pipeline> pPipeline_;
};