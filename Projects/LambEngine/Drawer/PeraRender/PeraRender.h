/// ==============================
/// ==  PeraRenderクラスの宣言  ==
/// ==============================

#pragma once
#include <string>
#include <array>
#include <initializer_list>

#include "Engine/Graphics/PipelineObject/PeraPipeline/PeraPipeline.h"

#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix.h"

/// <summary>
/// ポストエフェクトの描画
/// </summary>
class PeraRender {
public:
	struct PeraVertexData {
		Vector3 position;
		Vector2 uv;
	};

public:
	PeraRender();
	PeraRender(uint32_t width, uint32_t height);
	~PeraRender();

	PeraRender(const PeraRender&) = delete;
	PeraRender(PeraRender&&) = delete;
	PeraRender& operator=(const PeraRender&) = delete;
	PeraRender& operator=(PeraRender&&) = delete;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="psFileName">PixelShaderのファイルパス</param>
	/// <param name="formtats">GBufferの設定</param>
	void Initialize(const std::string& psFileName, std::initializer_list<DXGI_FORMAT> formtats);
	void Initialize(PeraPipeline* pipelineObject);

public:
	/// <summary>
	/// レンダーターゲットの設定
	/// </summary>
	/// <param name="depthHandle"></param>
	void PreDraw(const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="blend">ブレンドタイプ</param>
	/// <param name="depthHandle"></param>
	void Draw(
		Pipeline::Blend blend, 
		D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle
	);

	Texture* GetTex() const {
		return pPeraPipelineObject_->GetRender().GetTex();
	}

	/// <summary>
	/// 書き込みとのステート変更
	/// </summary>
	void ChangeResourceState() {
		pPeraPipelineObject_->GetRender().ChangeResourceState();
	}

	void SetMainRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle) {
		pPeraPipelineObject_->GetRender().SetMainRenderTarget(depthHandle);
	}

	/// <summary>
	/// デバッグ
	/// </summary>
	/// <param name="guiName"></param>
	void Debug(const std::string& guiName);

	/// <summary>
	/// パイプラインオブジェクトを変更する
	/// </summary>
	/// <param name="pipelineObject"></param>
	void ResetPipelineObject(PeraPipeline* pipelineObject);

	RenderTarget& GetRender();
	const RenderTarget& GetRender() const;

public:
	uint32_t color;

private:
	std::unique_ptr<PeraPipeline> pPeraPipelineObject_;
};