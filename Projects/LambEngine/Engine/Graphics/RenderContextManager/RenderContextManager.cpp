/// ========================================
/// ==  RenderContextManagerクラスの定義  ==
/// ========================================



#include "RenderContextManager.h"
#include "Utils/SafeDelete.h"

#include "../Shader/ShaderManager/ShaderManager.h"

#include "../../Core/DescriptorHeap/CbvSrvUavHeap.h"

#include "../PipelineManager/PipelineManager.h"

Lamb::SafePtr<RenderContextManager> RenderContextManager::pInstance_ = nullptr;

RenderContextManager::~RenderContextManager()
{
}

RenderContextManager* const RenderContextManager::GetInstance() {
	return pInstance_.get();
}

void RenderContextManager::Initialize() {
	pInstance_.reset(new RenderContextManager());
	Lamb::AddLog("Initialize RenderContextManager succeeded");
}

void RenderContextManager::Finalize() {
	pInstance_.reset();
	Lamb::AddLog("Finalize RenderContextManager succeeded");
}

RenderSet* const RenderContextManager::Get(const LoadFileNames& fileNames)
{
	auto isExist = renderData_.find(fileNames);
	if (isExist != renderData_.end()) {
		return renderData_[fileNames].get();
	}
	else if (isExist == renderData_.end() and not isNowThreading_) {
		Load(fileNames);
		return renderData_[fileNames].get();
	}
	else {
		isExist = threadRenderData_.find(fileNames);
		if (isExist == threadRenderData_.end()) {
			Load(fileNames);
		}

		return threadRenderData_[fileNames].get();
	}
}

RenderSet* const RenderContextManager::Get(const MeshLoadFileNames& fileNames) {
	if (not isNowThreading_) {
		auto isExist = meshRenderData_.find(fileNames);
		if (isExist == meshRenderData_.end()) {
			LoadMesh(fileNames);
		}
		return meshRenderData_[fileNames].get();
	}
	else {
		auto isExist = threadMeshRenderData_.find(fileNames);
		if (isExist == threadMeshRenderData_.end()) {
			LoadMesh(fileNames);
		}

		return threadMeshRenderData_[fileNames].get();
	}
}

void RenderContextManager::SetIsNowThreading(bool isNowThreading) {
	isNowThreading_ = isNowThreading;

	if (not isNowThreading_) {
		for (auto& i : threadRenderData_) {
			renderData_.insert(std::make_pair(i.first, i.second.release()));
		}

		for (auto& i : threadMeshRenderData_) {
			meshRenderData_.insert(std::make_pair(i.first, i.second.release()));
		}

		ResizeRenderList();

		threadRenderData_.clear();
		threadMeshRenderData_.clear();
	}
}

std::pair<size_t, const std::list<RenderData*>&> RenderContextManager::CreateRenderList(BlendType blend)
{
	if (blend == BlendType::kAlphaEffect) {
		blend = BlendType::kNum;
	}

	std::pair<size_t, const std::list<RenderData*>&> result = { 0llu, renderDataLists_[blend] };
	size_t count = 0;
	auto itr = renderDataLists_[blend].begin();

	for (auto& i : renderData_) {
		if (i.second->IsDraw(blend)) {
			if (itr == renderDataLists_[blend].end()) {
				break;
			}
			*itr = i.second->GetRenderData(blend);
			itr++;
			count++;
		}
	}

	for (auto& i : meshRenderData_) {
		if (i.second->IsDraw(blend)) {
			if (itr == renderDataLists_[blend].end()) {
				break;
			}
			*itr = i.second->GetRenderData(blend);
			itr++;
			count++;
		}
	}

	result.first = count;

	return result;
}

void RenderContextManager::ResizeRenderList() {
	for (auto& i : renderDataLists_) {
		i.resize(renderData_.size() + meshRenderData_.size());
	}
}

Shader RenderContextManager::LoadShader(const ShaderFileNames& shaderName)
{
	Shader result;

	ShaderManager* const shaderMaanger = ShaderManager::GetInstance();


	result.vertex = shaderMaanger->LoadVertexShader(shaderName.vsFileName);
	result.pixel = shaderMaanger->LoadPixelShader(shaderName.psFileName);
	result.geometory = shaderMaanger->LoadGeometoryShader(shaderName.gsFileName);
	result.hull = shaderMaanger->LoadHullShader(shaderName.hsFileName);
	result.domain = shaderMaanger->LoadDomainShader(shaderName.dsFileName);

	return result;
}

MeshShader RenderContextManager::LoadMeshShader(const MeshShaderFileNames& shaderName)
{
	MeshShader result;

	ShaderManager* const shaderMaanger = ShaderManager::GetInstance();


	result.mesh = shaderMaanger->LoadMeshShader(shaderName.msFileName);
	result.pixel = shaderMaanger->LoadPixelShader(shaderName.psFileName);
	result.amplification = shaderMaanger->LoadAmplificationShader(shaderName.asFileName);

	return result;
}

std::array<Pipeline*, BlendType::kNum> RenderContextManager::CreateGraphicsPipelines(Shader shader, uint32_t numRenderTarget)
{
	std::array<Pipeline*, BlendType::kNum> result = {nullptr};

	auto srvHeap = CbvSrvUavHeap::GetInstance();


	std::array<D3D12_DESCRIPTOR_RANGE, 1> texRange = {};
	texRange[0].NumDescriptors = srvHeap->GetMaxTexture();
	texRange[0].BaseShaderRegister = 3;
	texRange[0].OffsetInDescriptorsFromTableStart = D3D12_APPEND_ALIGNED_ELEMENT;
	texRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;


	std::array<D3D12_ROOT_PARAMETER, 6> rootPrams = {};
	rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[0].Descriptor.ShaderRegister = 0;
	rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[1].Descriptor.ShaderRegister = 1;
	rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootPrams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[2].Descriptor.ShaderRegister = 0;
	rootPrams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[3].Descriptor.ShaderRegister = 1;
	rootPrams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootPrams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[4].Descriptor.ShaderRegister = 2;
	rootPrams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootPrams[5].DescriptorTable.NumDescriptorRanges = UINT(texRange.size());
	rootPrams[5].DescriptorTable.pDescriptorRanges = texRange.data();
	rootPrams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootSignature::Desc desc;
	desc.rootParameter = rootPrams.data();
	desc.rootParameterSize = rootPrams.size();
	desc.samplerDeacs.push_back(
		CreateLinearSampler()
	);


	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::Desc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.vsInputData.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT });
	pipelineDesc.shader = shader;
	pipelineDesc.isDepth = false;
	pipelineDesc.blend[0] = Pipeline::None;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::Back;
	pipelineDesc.topologyType = (shader.hull != nullptr ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pipelineDesc.numRenderTarget = numRenderTarget;


	for (size_t i = 0; i < size_t(BlendType::kNum); i++) {
		size_t blendType = i < Pipeline::Blend::BlendTypeNum ? i : i - Pipeline::Blend::BlendTypeNum;

		pipelineDesc.isDepth = i < Pipeline::Blend::BlendTypeNum;
		for (uint32_t renderCount = 0; renderCount < pipelineDesc.numRenderTarget; renderCount++) {
			if (pipelineDesc.isDepth) {
				pipelineDesc.rtvFormtat[renderCount] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else {
				pipelineDesc.rtvFormtat[renderCount] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			}
		}
		
		for (uint32_t blendIndex = 0; blendIndex < pipelineDesc.numRenderTarget; blendIndex++) {
			pipelineDesc.blend[blendIndex] = Pipeline::Blend(blendType);
		}
		pipelineManager->SetDesc(pipelineDesc);
		result[i] = pipelineManager->Create();
	}


	pipelineManager->StateReset();

	return result;
}

std::array<Pipeline*, BlendType::kNum> RenderContextManager::CreateSkinAnimationGraphicsPipelines(Shader shader, uint32_t numRenderTarget)
{
	std::array<Pipeline*, BlendType::kNum> result = { nullptr };

	auto srvHeap = CbvSrvUavHeap::GetInstance();


	std::array<D3D12_DESCRIPTOR_RANGE, 1> texRange = {};
	texRange[0].NumDescriptors = srvHeap->GetMaxTexture();
	texRange[0].BaseShaderRegister = 3;
	texRange[0].OffsetInDescriptorsFromTableStart = D3D12_APPEND_ALIGNED_ELEMENT;
	texRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;



	std::array<D3D12_ROOT_PARAMETER, 7> rootPrams = {};
	rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[0].Descriptor.ShaderRegister = 0;
	rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[1].Descriptor.ShaderRegister = 1;
	rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[2].Descriptor.ShaderRegister = 0;
	rootPrams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[3].Descriptor.ShaderRegister = 1;
	rootPrams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[4].Descriptor.ShaderRegister = 2;
	rootPrams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootPrams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[5].Descriptor.ShaderRegister = 4;
	rootPrams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootPrams[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootPrams[6].DescriptorTable.NumDescriptorRanges = UINT(texRange.size());
	rootPrams[6].DescriptorTable.pDescriptorRanges = texRange.data();
	rootPrams[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootSignature::Desc desc;
	desc.rootParameter = rootPrams.data();
	desc.rootParameterSize = rootPrams.size();
	desc.samplerDeacs.push_back(
		CreateLinearSampler()
	);

	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::Desc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.vsInputData.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT });

	pipelineDesc.vsInputData.push_back({ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1 });
	pipelineDesc.vsInputData.push_back({ "WEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1 });
	pipelineDesc.vsInputData.push_back({ "INDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1 });
	pipelineDesc.vsInputData.push_back({ "INDEX", 1, DXGI_FORMAT_R32G32B32A32_SINT, 1 });
	pipelineDesc.shader = shader;
	pipelineDesc.isDepth = false;
	pipelineDesc.blend[0] = Pipeline::None;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::None;
	pipelineDesc.topologyType = (shader.hull != nullptr ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pipelineDesc.numRenderTarget = numRenderTarget;

	for (size_t i = 0; i < size_t(BlendType::kNum); i++) {
		size_t blendType = i < Pipeline::Blend::BlendTypeNum ? i : i - Pipeline::Blend::BlendTypeNum;

		pipelineDesc.isDepth = i < Pipeline::Blend::BlendTypeNum;

		for (uint32_t renderCount = 0; renderCount < pipelineDesc.numRenderTarget; renderCount++) {
			if (pipelineDesc.isDepth) {
				pipelineDesc.rtvFormtat[renderCount] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else {
				pipelineDesc.rtvFormtat[renderCount] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			}
		}

		for (uint32_t blendIndex = 0; blendIndex < pipelineDesc.numRenderTarget; blendIndex++) {
			pipelineDesc.blend[blendIndex] = Pipeline::Blend(blendType);
		}
		pipelineManager->SetDesc(pipelineDesc);
		result[i] = pipelineManager->Create();
	}


	pipelineManager->StateReset();

	return result;
}

std::array<Pipeline*, BlendType::kNum> RenderContextManager::CreateMeshShaderGraphicsPipelines(MeshShader shader, uint32_t numRenderTarget)
{
	std::array<Pipeline*, BlendType::kNum> result = { nullptr };

	auto srvHeap = CbvSrvUavHeap::GetInstance();

	std::array<D3D12_DESCRIPTOR_RANGE, 1> texRange = {};
	texRange[0].NumDescriptors = srvHeap->GetMaxTexture();
	texRange[0].BaseShaderRegister = 7;
	texRange[0].OffsetInDescriptorsFromTableStart = D3D12_APPEND_ALIGNED_ELEMENT;
	texRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;


	std::array<D3D12_ROOT_PARAMETER, 11> rootPrams = {};
	// Light
	rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[0].Descriptor.ShaderRegister = 0;
	rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// cameraPos
	rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[1].Descriptor.ShaderRegister = 1;
	rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// インスタンスカウント
	rootPrams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[2].Descriptor.ShaderRegister = 2;
	rootPrams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION;

	// Transform
	rootPrams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[3].Descriptor.ShaderRegister = 0;
	rootPrams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	//  gVertices
	rootPrams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[4].Descriptor.ShaderRegister = 1;
	rootPrams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	// gUniqueVertexIndices
	rootPrams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[5].Descriptor.ShaderRegister = 2;
	rootPrams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	// gPrimitiveIndices
	rootPrams[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[6].Descriptor.ShaderRegister = 3;
	rootPrams[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	// gMeshlets
	rootPrams[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[7].Descriptor.ShaderRegister = 4;
	rootPrams[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	// 色
	rootPrams[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[8].Descriptor.ShaderRegister = 5;
	rootPrams[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// 各shader固有のStructuredBuffer
	rootPrams[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[9].Descriptor.ShaderRegister = 6;
	rootPrams[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Textures
	rootPrams[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootPrams[10].DescriptorTable.NumDescriptorRanges = UINT(texRange.size());
	rootPrams[10].DescriptorTable.pDescriptorRanges = texRange.data();
	rootPrams[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	RootSignature::Desc desc;
	desc.rootParameter = rootPrams.data();
	desc.rootParameterSize = rootPrams.size();
	desc.samplerDeacs.push_back(
		CreateLinearSampler()
	);
	desc.flag  = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	desc.flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	desc.flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	desc.flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::MeshDesc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.shader = shader;
	pipelineDesc.isDepth =false;
	pipelineDesc.blend[0] = Pipeline::None;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::Back;
	pipelineDesc.numRenderTarget = numRenderTarget;


	for (size_t i = 0; i < size_t(BlendType::kNum); i++) {
		size_t blendType = i < Pipeline::Blend::BlendTypeNum ? i : i - Pipeline::Blend::BlendTypeNum;

		pipelineDesc.isDepth = i < Pipeline::Blend::BlendTypeNum;
		for (uint32_t renderCount = 0; renderCount < pipelineDesc.numRenderTarget; renderCount++) {
			if (pipelineDesc.isDepth) {
				pipelineDesc.rtvFormtat[renderCount] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else {
				pipelineDesc.rtvFormtat[renderCount] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			}
		}

		for (uint32_t blendIndex = 0; blendIndex < pipelineDesc.numRenderTarget; blendIndex++) {
			pipelineDesc.blend[blendIndex] = Pipeline::Blend(blendType);
		}
		pipelineManager->SetDesc(pipelineDesc);
		result[i] = pipelineManager->CreateMesh();
	}


	pipelineManager->StateReset();

	return result;
}

Pipeline* RenderContextManager::CreateShadowPipeline()
{

	Pipeline* result = nullptr;

	ShaderManager* const shaderMaanger = ShaderManager::GetInstance();

	Shader shader;
	shader.vertex = shaderMaanger->LoadVertexShader("./Shaders/Shadow/Shadow.VS.hlsl");

	std::array<D3D12_ROOT_PARAMETER, 2> rootPrams = {};
	rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[0].Descriptor.ShaderRegister = 0;
	rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[1].Descriptor.ShaderRegister = 0;
	rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	RootSignature::Desc desc;
	desc.rootParameter = rootPrams.data();
	desc.rootParameterSize = rootPrams.size();


	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::Desc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.vsInputData.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT });
	pipelineDesc.shader = shader;
	pipelineDesc.isDepth = true;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::Back;
	pipelineDesc.topologyType = (shader.hull != nullptr ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	// 深度値のみを描画するので0
	pipelineDesc.numRenderTarget = 0;

	pipelineManager->SetDesc(pipelineDesc);
	result = pipelineManager->Create();


	pipelineManager->StateReset();

	return result;
}

Pipeline* RenderContextManager::CreateSkinAnimationShadowPipeline()
{
	Pipeline* result = nullptr;

	ShaderManager* const shaderMaanger = ShaderManager::GetInstance();

	Shader shader;
	shader.vertex = shaderMaanger->LoadVertexShader("./Shaders/Shadow/ShadowAnimation.VS.hlsl");

	std::array<D3D12_ROOT_PARAMETER, 3> rootPrams = {};
	rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[0].Descriptor.ShaderRegister = 0;
	rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[1].Descriptor.ShaderRegister = 0;
	rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootPrams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[2].Descriptor.ShaderRegister = 1;
	rootPrams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	RootSignature::Desc desc;
	desc.rootParameter = rootPrams.data();
	desc.rootParameterSize = rootPrams.size();


	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::Desc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.vsInputData.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT });
	pipelineDesc.vsInputData.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R32_UINT });
	pipelineDesc.vsInputData.push_back({ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT });

	pipelineDesc.vsInputData.push_back({ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1 });
	pipelineDesc.vsInputData.push_back({ "WEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1 });
	pipelineDesc.vsInputData.push_back({ "INDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 1 });
	pipelineDesc.vsInputData.push_back({ "INDEX", 1, DXGI_FORMAT_R32G32B32A32_SINT, 1 });

	pipelineDesc.shader = shader;
	pipelineDesc.isDepth = true;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::Back;
	pipelineDesc.topologyType = (shader.hull != nullptr ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	// 深度値のみを描画するので0
	pipelineDesc.numRenderTarget = 0;

	pipelineManager->SetDesc(pipelineDesc);
	result = pipelineManager->Create();


	pipelineManager->StateReset();

	return result;
}

Pipeline* RenderContextManager::CreateMeshShaderShadowPipeline()
{
	Pipeline* result = nullptr;

	ShaderManager* const shaderMaanger = ShaderManager::GetInstance();

	MeshShader shader;
	shader.amplification = shaderMaanger->LoadAmplificationShader("./Shaders/Shadow/Shadow.AS.hlsl");
	shader.mesh = shaderMaanger->LoadMeshShader("./Shaders/Shadow/Shadow.MS.hlsl");

	std::array<D3D12_ROOT_PARAMETER, 7> rootPrams = {};
	rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[0].Descriptor.ShaderRegister = 0;
	rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_AMPLIFICATION;

	rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootPrams[1].Descriptor.ShaderRegister = 1;
	rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;


	rootPrams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[2].Descriptor.ShaderRegister = 0;
	rootPrams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	rootPrams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[3].Descriptor.ShaderRegister = 1;
	rootPrams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	rootPrams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[4].Descriptor.ShaderRegister = 2;
	rootPrams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	rootPrams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[5].Descriptor.ShaderRegister = 3;
	rootPrams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	rootPrams[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootPrams[6].Descriptor.ShaderRegister = 4;
	rootPrams[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_MESH;

	RootSignature::Desc desc;
	desc.rootParameter = rootPrams.data();
	desc.rootParameterSize = rootPrams.size();
	desc.flag = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	desc.flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	desc.flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	desc.flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


	auto pipelineManager = PipelineManager::GetInstance();
	Pipeline::MeshDesc pipelineDesc;
	pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
	pipelineDesc.shader = shader;
	pipelineDesc.isDepth = true;
	pipelineDesc.solidState = Pipeline::SolidState::Solid;
	pipelineDesc.cullMode = Pipeline::CullMode::Back;
	pipelineDesc.numRenderTarget = 0;

	pipelineManager->SetDesc(pipelineDesc);
	result = pipelineManager->CreateMesh();


	pipelineManager->StateReset();

	return result;
}
