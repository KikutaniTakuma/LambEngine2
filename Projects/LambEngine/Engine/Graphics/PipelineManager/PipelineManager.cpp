/// ===================================
/// ==  PipelineManagerクラスの定義  ==
/// ===================================


#include "PipelineManager.h"
#include <cassert>
#include <algorithm>
#include "Engine/Graphics/RootSignature/RootSignature.h"
#include "Utils/SafeDelete.h"
#include "Utils/ExecutionLog.h"
#include "Error/Error.h"

Lamb::SafePtr<PipelineManager> PipelineManager::pInstance_ = nullptr;

void PipelineManager::Initialize() {
	if (pInstance_) {
		return;
	}
	pInstance_.reset(new PipelineManager());
	if(pInstance_){
		Lamb::AddLog("Initialize PipelineManager succeeded");
	}
	else {
		throw Lamb::Error::Code<PipelineManager>("instance is nullptr", ErrorPlace);
	}
}
void PipelineManager::Finalize() {
	pInstance_.reset();

	Lamb::AddLog("Finalize PipelineManager succeeded");
}

PipelineManager* const PipelineManager::GetInstance()
{
	return pInstance_.get();
}

RootSignature* PipelineManager::CreateRootSgnature(const RootSignature::Desc& desc)
{
	// すでに作ってあるか
	auto IsSame = [&desc](const std::unique_ptr<RootSignature>& rootSignature_) {
		return rootSignature_->IsSame(desc);
		};
	
	// 判定
	auto rootSignatureItr = std::find_if(pInstance_->rootSignatures_.begin(), pInstance_->rootSignatures_.end(), IsSame);

	// 作ってなかったら
	if (rootSignatureItr == pInstance_->rootSignatures_.end()) {
		auto rootSignature = std::make_unique<RootSignature>();

		rootSignature->Create(desc);

		pInstance_->rootSignatures_.push_back(std::move(rootSignature));

		return pInstance_->rootSignatures_.back().get();
	}
	else {
		return rootSignatureItr->get();
	}
}

void PipelineManager::SetDesc(const Pipeline::Desc& desc) {
	pipelineDesc_ = desc;
}

void PipelineManager::SetDesc(const Pipeline::MeshDesc& desc)
{
	pipelineMeshDesc_ = desc;
}

Pipeline* const PipelineManager::Create() {
	// すでに作ってあるか
	auto IsSmae = [this](const std::unique_ptr<Pipeline>& pipeline) {
			return pipeline->IsSame(pipelineDesc_);
		};

	auto pipelineItr = std::find_if(pInstance_->pipelines_.begin(), pInstance_->pipelines_.end(), IsSmae);

	// 作ってなかったら
	if (pipelineItr == pInstance_->pipelines_.end()) {
		auto pipeline = std::make_unique<Pipeline>();
		pipeline->Create(pipelineDesc_);

		if (not pipeline->graphicsPipelineState_) {
			return nullptr;
		}

		pInstance_->pipelines_.push_back(std::move(pipeline));

		return pInstance_->pipelines_.back().get();
	}
	else {
		return pipelineItr->get();
	}
}

Pipeline* const PipelineManager::CreateCubeMap()
{
	// すでに作ってあるか
	auto IsSmae = [this](const std::unique_ptr<Pipeline>& pipeline) {
		return pipeline->IsSame(pipelineDesc_);
		};

	auto pipelineItr = std::find_if(pInstance_->pipelines_.begin(), pInstance_->pipelines_.end(), IsSmae);

	// 作ってなかったら
	if (pipelineItr == pInstance_->pipelines_.end()) {
		auto pipeline = std::make_unique<Pipeline>();
		pipeline->CreateCubeMap(pipelineDesc_);

		if (not pipeline->graphicsPipelineState_) {
			return nullptr;
		}

		pInstance_->pipelines_.push_back(std::move(pipeline));

		return pInstance_->pipelines_.back().get();
	}
	else {
		return pipelineItr->get();
	}
}

Pipeline* const PipelineManager::CreateMesh()
{
	// すでに作ってあるか
	auto IsSmae = [this](const std::unique_ptr<Pipeline>& pipeline) {
		return pipeline->IsSame(pipelineMeshDesc_);
		};

	auto pipelineItr = std::find_if(pInstance_->pipelines_.begin(), pInstance_->pipelines_.end(), IsSmae);

	// 作ってなかったら
	if (pipelineItr == pInstance_->pipelines_.end()) {
		auto pipeline = std::make_unique<Pipeline>();
		pipeline->Create(pipelineMeshDesc_);

		if (not pipeline->graphicsPipelineState_) {
			return nullptr;
		}

		pInstance_->pipelines_.push_back(std::move(pipeline));

		return pInstance_->pipelines_.back().get();
	}
	else {
		return pipelineItr->get();
	}
}

void PipelineManager::StateReset() {
	pipelineDesc_ = {};
	pipelineMeshDesc_ = {};
}

PipelineManager::PipelineManager() :
	pipelines_(),
	rootSignatures_(),
	pipelineDesc_(),
	pipelineMeshDesc_()
{}