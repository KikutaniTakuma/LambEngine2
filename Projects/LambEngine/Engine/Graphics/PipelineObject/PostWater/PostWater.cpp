/// =============================
/// ==  PostWaterクラスの定義  ==
/// =============================


#include "PostWater.h"
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


void PostWater::SetRtvFormt(DXGI_FORMAT format) {
	format_ = format;
}

void PostWater::SetTonemapParams(const TonemapParams& tonemapParams) {
	tonemapParamas_ = tonemapParams;
}

void PostWater::SetWaterWorldMatrixInverse(const float32_t4x4& waterWorldMatrixInverse) {
	waterWorldMatrixInverse_ = waterWorldMatrixInverse;
}

void PostWater::Use(Pipeline::Blend blendType, bool isDepth) {
	//translate_ += uvScrollSpeed_ * Lamb::DeltaTime();
	scrollUV_ = Mat4x4::MakeAffin(Vector3::kIdentity * 10.0f, Quaternion::kIdentity, translate_);

	scrollUVBuf_[Lamb::GetGraphicBufferIndex()]->MemCpy(&scrollUV_);
	depthFloatBuf_[Lamb::GetGraphicBufferIndex()]->MemCpy(&depthFloat_);
	tonemapParamasBuf_[Lamb::GetGraphicBufferIndex()]->MemCpy(&tonemapParamas_);
	waterInverseBuf_[Lamb::GetGraphicBufferIndex()]->MemCpy(&waterWorldMatrixInverse_);


	if (isDepth) {
		pipelines_[blendType]->Use();
	}
	else {
		pipelinesNoDepth_[blendType]->Use();
	}
	auto* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	render_->UseThisRenderTargetShaderResource();
	commandList->SetGraphicsRootDescriptorTable(1, distortionTexHandle_);
	commandList->SetGraphicsRootDescriptorTable(2, depthTexHandle_);
	commandList->SetGraphicsRootDescriptorTable(3, causticsTexHandle_);
	commandList->SetGraphicsRootDescriptorTable(4, worldPositionTexHandle_);
	commandList->SetGraphicsRootConstantBufferView(5, colorBuf_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(6, scrollUVBuf_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(7, depthFloatBuf_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(8, tonemapParamasBuf_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
	commandList->SetGraphicsRootConstantBufferView(9, waterInverseBuf_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
}

void PostWater::Init(
	const std::string& vsShader,
	const std::string& psShader,
	std::initializer_list<DXGI_FORMAT> formtats
) {
	if (width_ == 0u) {
		width_ = static_cast<uint32_t>(Lamb::ClientSize().x);
	}
	if (height_ == 0u) {
		height_ = static_cast<uint32_t>(Lamb::ClientSize().y);
	}

	render_.reset(new RenderTarget{ width_, height_ });

	this->LoadShader(
		vsShader,
		psShader
	);

	std::array<D3D12_DESCRIPTOR_RANGE, 1> renderRange = {};
	renderRange[0].BaseShaderRegister = 0;
	renderRange[0].NumDescriptors = 1;
	renderRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	renderRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> distortionRange = {};
	distortionRange[0].BaseShaderRegister = 1;
	distortionRange[0].NumDescriptors = 1;
	distortionRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	distortionRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::array<D3D12_DESCRIPTOR_RANGE, 1> depthRange = {};
	depthRange[0].BaseShaderRegister = 2;
	depthRange[0].NumDescriptors = 1;
	depthRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	depthRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> causticsRange = {};
	causticsRange[0].BaseShaderRegister = 3;
	causticsRange[0].NumDescriptors = 1;
	causticsRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	causticsRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::array<D3D12_DESCRIPTOR_RANGE, 1> worldPositionTexRange = {};
	worldPositionTexRange[0].BaseShaderRegister = 4;
	worldPositionTexRange[0].NumDescriptors = 1;
	worldPositionTexRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	worldPositionTexRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::array<D3D12_ROOT_PARAMETER, 10> rootParameter = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[0].DescriptorTable.pDescriptorRanges = renderRange.data();
	rootParameter[0].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(renderRange.size());

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[1].DescriptorTable.pDescriptorRanges = distortionRange.data();
	rootParameter[1].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(distortionRange.size());

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = depthRange.data();
	rootParameter[2].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(depthRange.size());

	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].DescriptorTable.pDescriptorRanges = causticsRange.data();
	rootParameter[3].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(causticsRange.size());

	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[4].DescriptorTable.pDescriptorRanges = worldPositionTexRange.data();
	rootParameter[4].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(worldPositionTexRange.size());

	rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[5].Descriptor.ShaderRegister = 0;

	rootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[6].Descriptor.ShaderRegister = 1;

	rootParameter[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[7].Descriptor.ShaderRegister = 2;

	rootParameter[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[8].Descriptor.ShaderRegister = 3;

	rootParameter[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[9].Descriptor.ShaderRegister = 4;


	RootSignature::Desc desc;
	desc.rootParameter = rootParameter.data();
	desc.rootParameterSize = rootParameter.size();
	desc.samplerDeacs.push_back(
		CreateLinearSampler(0)
	);

	desc.samplerDeacs.push_back(
		CreateBorderPointSampler(1)
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


	CbvSrvUavHeap* const srvHeap = CbvSrvUavHeap::GetInstance();

	srvHeap->BookingHeapPos(1u);
	srvHeap->CreateView(render_.get());
	

	std::for_each(
		colorBuf_.begin(),
		colorBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<Vector4>>();
		}
	);

	std::for_each(
		 scrollUVBuf_.begin(),
		 scrollUVBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<float32_t4x4>>();
		}
	);

	std::for_each(
		depthFloatBuf_.begin(),
		depthFloatBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<float32_t>>();
		}
	);

	std::for_each(
		tonemapParamasBuf_.begin(),
		tonemapParamasBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<TonemapParams>>();
		}
	);

	std::for_each(
		waterInverseBuf_.begin(),
		waterInverseBuf_.end(),
		[](auto& n) {
			n = std::make_unique<ConstantBuffer<float32_t4x4>>();
		}
	);
}

PostWater::~PostWater() {
	if (render_) {
		auto* const srvHeap = CbvSrvUavHeap::GetInstance();
		srvHeap->ReleaseView(render_->GetHandleUINT());
	}

	std::for_each(
		colorBuf_.begin(),
		colorBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);


	std::for_each(
		scrollUVBuf_.begin(),
		scrollUVBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	std::for_each(
		depthFloatBuf_.begin(),
		depthFloatBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	std::for_each(
		tonemapParamasBuf_.begin(),
		tonemapParamasBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	std::for_each(
		waterInverseBuf_.begin(),
		waterInverseBuf_.end(),
		[](auto& n) {
			n.reset();
		}
	);

	render_.reset();
}