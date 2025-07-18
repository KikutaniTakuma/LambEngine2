/// ==============================
/// ==  ブレンド設定を作成関数  ==
/// ==============================


#pragma once
#include "PipelineManager/Pipeline/Pipeline.h"

/// <summary>
/// レンダーターゲットに書き込む設定構造体の作成
/// </summary>
/// <param name="blend">ブレンドタイプ</param>
/// <returns>設定構造体</returns>
inline D3D12_RENDER_TARGET_BLEND_DESC CreateRenderTargetBlendDesc(Pipeline::Blend blend) {
	D3D12_RENDER_TARGET_BLEND_DESC result = {};

	result.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	switch (blend)
	{
	case Pipeline::Blend::None:
	default:
		result.BlendEnable = false;
		break;
	case Pipeline::Blend::Normal:
		result.BlendEnable = true;
		result.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		result.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		result.BlendOp = D3D12_BLEND_OP_ADD;
		break;
	case Pipeline::Blend::Add:
		result.BlendEnable = true;
		result.SrcBlend = D3D12_BLEND_ONE;
		result.DestBlend = D3D12_BLEND_ONE;
		result.BlendOp = D3D12_BLEND_OP_ADD;
		break;
	case Pipeline::Blend::Sub:
		result.BlendEnable = true;
		result.SrcBlend = D3D12_BLEND_ONE;
		result.DestBlend = D3D12_BLEND_ONE;
		result.BlendOp = D3D12_BLEND_OP_SUBTRACT;
		break;
	case Pipeline::Blend::Mul:
		result.BlendEnable = true;
		result.SrcBlend = D3D12_BLEND_ZERO;
		result.DestBlend = D3D12_BLEND_SRC_COLOR;
		result.BlendOp = D3D12_BLEND_OP_ADD;
		break;
	}

	result.SrcBlendAlpha = D3D12_BLEND_ONE;
	result.DestBlendAlpha = D3D12_BLEND_ZERO;
	result.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	return result;
}