/// ====================================
/// ==  DeferredRederingクラスの定義  ==
/// ====================================



#include "DeferredRendering.h"
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



void DeferredRendering::Draw() {
	const uint32_t kBufferIndex = Lamb::GetGraphicBufferIndex();

	// データ設定

	(*deferredRenderingDataBuf_[kBufferIndex]).Map();
	(*atomosphericDataBuf_[kBufferIndex]).Map();
	(*cameraBuf_[kBufferIndex]).Map();
	(*lightCameraBuf_[kBufferIndex]).Map();

	**deferredRenderingDataBuf_[kBufferIndex] = deferredRenderingData_;
	**atomosphericDataBuf_[kBufferIndex] = atomosphericData_;
	**cameraBuf_[kBufferIndex] = cameraMatrix_;
	**lightCameraBuf_[kBufferIndex] = lightCameraMatrix_;

	(*deferredRenderingDataBuf_[kBufferIndex]).Unmap();
	(*atomosphericDataBuf_[kBufferIndex]).Unmap();
	(*cameraBuf_[kBufferIndex]).Unmap();
	(*lightCameraBuf_[kBufferIndex]).Unmap();

	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	pipelinesNoDepth_[Pipeline::Blend::None]->Use();

	// コマンド

	commandList->SetGraphicsRootConstantBufferView(0, deferredRenderingDataBuf_[kBufferIndex]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(1, atomosphericDataBuf_[kBufferIndex]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(2, cameraBuf_[kBufferIndex]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(3, lightCameraBuf_[kBufferIndex]->GetGPUVtlAdrs());

	commandList->SetGraphicsRootDescriptorTable(4, colorTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(5, normalTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(6, worldPositionTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(7, distortionTextureHandle_);
	commandList->DrawInstanced(3, 1, 0, 0);
}

void DeferredRendering::SetCameraMatrix(const float32_t4x4& camera) {
	cameraMatrix_ = camera;
}

void DeferredRendering::SetLightCameraMatrix(const float32_t4x4& lightCamera) {
	lightCameraMatrix_ = lightCamera;
}

void DeferredRendering::Use(Pipeline::Blend blendType, bool isDepth) {
	const uint32_t kBufferIndex = Lamb::GetGraphicBufferIndex();

	(*deferredRenderingDataBuf_[kBufferIndex]).Map();
	(*atomosphericDataBuf_[kBufferIndex]).Map();

	**deferredRenderingDataBuf_[kBufferIndex] = deferredRenderingData_;
	**atomosphericDataBuf_[kBufferIndex] = atomosphericData_;

	(*deferredRenderingDataBuf_[kBufferIndex]).Unmap();
	(*atomosphericDataBuf_[kBufferIndex]).Unmap();

	if (isDepth) {
		pipelines_[blendType]->Use();
	}
	else {
		pipelinesNoDepth_[blendType]->Use();
	}
	auto* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(0, deferredRenderingDataBuf_[kBufferIndex]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(1, atomosphericDataBuf_[kBufferIndex]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(2, cameraBuf_[kBufferIndex]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(3, lightCameraBuf_[kBufferIndex]->GetGPUVtlAdrs());

	commandList->SetGraphicsRootDescriptorTable(4, colorTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(5, normalTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(6, worldPositionTextureHandle_);
	commandList->SetGraphicsRootDescriptorTable(7, distortionTextureHandle_);
}

void DeferredRendering::Init(
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
	std::array<D3D12_DESCRIPTOR_RANGE, 1> diffNormalTextureRange = {};
	diffNormalTextureRange[0].BaseShaderRegister = 1;
	diffNormalTextureRange[0].NumDescriptors = 1;
	diffNormalTextureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	diffNormalTextureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> diffWorldPositionTextureRange = {};
	diffWorldPositionTextureRange[0].BaseShaderRegister = 2;
	diffWorldPositionTextureRange[0].NumDescriptors = 1;
	diffWorldPositionTextureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	diffWorldPositionTextureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> distortionTextureRange = {};
	distortionTextureRange[0].BaseShaderRegister = 3;
	distortionTextureRange[0].NumDescriptors = 1;
	distortionTextureRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	distortionTextureRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::array<D3D12_ROOT_PARAMETER, 8> rootParameter = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[0].Descriptor.ShaderRegister = 0;

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[1].Descriptor.ShaderRegister = 1;

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[2].Descriptor.ShaderRegister = 2;

	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[3].Descriptor.ShaderRegister = 3;

	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[4].DescriptorTable.pDescriptorRanges = diffColorTextureRange.data();
	rootParameter[4].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(diffColorTextureRange.size());
	
	rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[5].DescriptorTable.pDescriptorRanges = diffNormalTextureRange.data();
	rootParameter[5].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(diffNormalTextureRange.size());

	rootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[6].DescriptorTable.pDescriptorRanges = diffWorldPositionTextureRange.data();
	rootParameter[6].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(diffWorldPositionTextureRange.size());

	rootParameter[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[7].DescriptorTable.pDescriptorRanges = distortionTextureRange.data();
	rootParameter[7].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(distortionTextureRange.size());


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

	std::for_each(
		deferredRenderingDataBuf_.begin(),
		deferredRenderingDataBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<DeferredRenderingData>>();
		}
	);

	std::for_each(
		atomosphericDataBuf_.begin(),
		atomosphericDataBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<AirSkyBox::AtmosphericParams>>();
		}
	);

	std::for_each(
		 cameraBuf_.begin(),
		 cameraBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<float32_t4x4>>();
		}
	);

	std::for_each(
		lightCameraBuf_.begin(),
		lightCameraBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<float32_t4x4>>();
		}
	);
}

DeferredRendering::~DeferredRendering() {
	std::for_each(
		deferredRenderingDataBuf_.begin(),
		deferredRenderingDataBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	std::for_each(
		atomosphericDataBuf_.begin(),
		atomosphericDataBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	std::for_each(
		cameraBuf_.begin(),
		cameraBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	std::for_each(
		lightCameraBuf_.begin(),
		lightCameraBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);
}