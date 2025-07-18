/// ==========================
/// ==  SkyBoxクラスの宣言  ==
/// ==========================

#pragma once
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Math/Matrix.h"
#include "Math/Vector4.h"
#include "Engine/Buffer/ConstantBuffer.h"

/// <summary>
/// CubeMapの描画
/// </summary>
class SkyBox {
public:
	struct ShaderData {
		Mat4x4 worldMat;
		Mat4x4 viewProjectionMat;
		Vector4 color;
	};

public:
	SkyBox() = default;
	SkyBox(const SkyBox&) = delete;
	SkyBox(SkyBox&&) = delete;
	~SkyBox();

public:
	SkyBox& operator=(const SkyBox&) = delete;
	SkyBox& operator=(SkyBox&&) = delete;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void Load(const std::string& fileName);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMat">ワールド行列</param>
	/// <param name="cameraMat">カメラ行列</param>
	/// <param name="color">色</param>
	void Draw(const Mat4x4& worldMat, const Mat4x4& cameraMat, uint32_t color);

	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle() const;

private:
	/// <summary>
	/// パイプライン作成
	/// </summary>
	void CreateGraphicsPipeline_();

private:
	D3D12_VERTEX_BUFFER_VIEW vertexView_ = {};
	Lamb::LambPtr<ID3D12Resource> pVertexResource_;

	D3D12_INDEX_BUFFER_VIEW indexView_ = {};
	Lamb::LambPtr<ID3D12Resource> pIndexResource_;

	static constexpr uint32_t kIndexNumber_ = 36u;
	std::unique_ptr<ConstantBuffer<ShaderData>> pShaderData_;
	Lamb::SafePtr<class Texture> pTexture_;
	Lamb::SafePtr<class Pipeline> pPipeline_;
};