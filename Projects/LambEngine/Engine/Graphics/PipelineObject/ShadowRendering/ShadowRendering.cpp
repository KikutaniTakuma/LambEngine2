/// ===================================
/// ==  ShadowRenderingクラスの定義  ==
/// ===================================



#include "ShadowRendering.h"
#include <cassert>
#include "Engine/Graphics/PipelineManager/PipelineManager.h"
#include "Utils/Random.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Graphics/TextureManager/TextureManager.h"

#include "Utils/EngineInfo.h"

#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE



void ShadowRendering::Draw() {
	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	pipelinesNoDepth_[Pipeline::Blend::None]->Use();

	commandList->SetGraphicsRootDescriptorTable(0, depthTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(1, depthShadowTextureHandle_);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void ShadowRendering::Use(Pipeline::Blend blendType, bool isDepth) {
	if (isDepth) {
		pipelines_[blendType]->Use();
	}
	else {
		pipelinesNoDepth_[blendType]->Use();
	}
	auto* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	commandList->SetGraphicsRootDescriptorTable(0, depthTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(1, depthShadowTextureHandle_);
}

void ShadowRendering::Init(
	const std::string& vsShader,
	const std::string& psShader,
	std::initializer_list<DXGI_FORMAT> formtats
) {
	this->LoadShader(
		vsShader,
		psShader
	);

	std::array<D3D12_DESCRIPTOR_RANGE, 1> diffColorTextureRange = {};
	diffColorTextureRange[0].BaseShaderRegister = 0;
	diffColorTextureRange[0].NumDescriptors = 1;
	diffColorTextureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	diffColorTextureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> depth = {};
	depth[0].BaseShaderRegister = 0;
	depth[0].NumDescriptors = 1;
	depth[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	depth[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> depthShadow = {};
	depthShadow[0].BaseShaderRegister = 1;
	depthShadow[0].NumDescriptors = 1;
	depthShadow[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	depthShadow[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	std::array<D3D12_ROOT_PARAMETER, 2> rootParameter = {};


	
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[0].DescriptorTable.pDescriptorRanges = depth.data();
	rootParameter[0].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(depth.size());

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[1].DescriptorTable.pDescriptorRanges = depthShadow.data();
	rootParameter[1].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(depthShadow.size());



	RootSignature::Desc desc;
	desc.rootParameter = rootParameter.data();
	desc.rootParameterSize = rootParameter.size();
	desc.samplerDeacs.push_back(
		CreateBorderPointSampler(0)
	);

	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::Desc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.vsInputData.clear();
	pipelineDesc.shader = shader_;
	pipelineDesc.isDepth = false;
	pipelineDesc.blend[0] = Pipeline::None;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::Back;
	pipelineDesc.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.numRenderTarget = uint32_t(formtats.size());
	for (uint32_t count = 0; const auto & i : formtats) {
		pipelineDesc.rtvFormtat[count] = i;
		count++;
		if (8 <= count) {
			break;
		}
	}

	for (int32_t i = Pipeline::Blend::None; i < Pipeline::Blend::BlendTypeNum; i++) {
		for (auto& blend : pipelineDesc.blend) {
			blend = Pipeline::Blend(i);
		}
		pipelineDesc.isDepth = true;
		pipelineManager->SetDesc(pipelineDesc);
		pipelines_[Pipeline::Blend(i)] = pipelineManager->Create();

		pipelineDesc.isDepth = false;
		pipelineManager->SetDesc(pipelineDesc);
		pipelinesNoDepth_[Pipeline::Blend(i)] = pipelineManager->Create();
	}

	pipelineManager->StateReset();

	
}

ShadowRendering::~ShadowRendering() {
	
}