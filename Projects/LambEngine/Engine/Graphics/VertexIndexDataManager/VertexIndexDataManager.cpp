/// ==========================================
/// ==  VertexIndexDataManagerクラスの定義  ==
/// ==========================================



#include "VertexIndexDataManager.h"
#include "Utils/EngineInfo.h"
#include "Utils/SafeDelete.h"
#include "../VertexIndexDataLoader/VertexIndexDataLoader.h"
#include "Error/Error.h"

Lamb::SafePtr<VertexIndexDataManager> VertexIndexDataManager::pInstance_ = nullptr;

VertexIndexDataManager* const VertexIndexDataManager::GetInstance() {
	return pInstance_.get();
}

VertexIndexDataManager::~VertexIndexDataManager() {
	vertexIndexData_.clear();
	Lamb::AddLog("Finalize MeshManager succeeded");
}

void VertexIndexDataManager::Initialize() {
	pInstance_.reset(new VertexIndexDataManager());
	if (pInstance_) {
		Lamb::AddLog("Initialize MeshManager succeeded");
	}
	else {
		throw Lamb::Error::Code<VertexIndexDataManager>("instance is nullptr", ErrorPlace);
	}
}
void VertexIndexDataManager::Finalize() {
	pInstance_.reset();
}

void VertexIndexDataManager::LoadModel(const std::string& fileName) {
	auto mesh = vertexIndexData_.find(fileName);

	if (mesh == vertexIndexData_.end()) {
		modelData_.insert(std::make_pair(fileName, std::make_unique<ModelData>(VertexIndexDataLoader::LoadModel(fileName))));
		vertexIndexData_.insert(std::make_pair(fileName, CreateMesh(*modelData_[fileName])));
	}
}

VertexIndexData* VertexIndexDataManager::GetVertexIndexData(const std::string& fileName)
{
	auto mesh = vertexIndexData_.find(fileName);

	if (mesh == vertexIndexData_.end()) {
		throw Lamb::Error::Code<VertexIndexDataManager>("This file is not loaded -> " + fileName, ErrorPlace);
	}

	return mesh->second.get();
}

ModelData* VertexIndexDataManager::GetModelData(const std::string& fileName)
{
	auto modelData = modelData_.find(fileName);

	if (modelData == modelData_.end()) {
		throw Lamb::Error::Code<VertexIndexDataManager>("This file is not loaded -> " + fileName, ErrorPlace);
	}

	return modelData->second.get();
}

VertexIndexData* VertexIndexDataManager::CreateMesh(const ModelData& modelData)
{
	Lamb::SafePtr result = Lamb::MakeSafePtr<VertexIndexData>();
	Lamb::SafePtr directXDevice = DirectXDevice::GetInstance();


	uint32_t indexSizeInBytes = static_cast<uint32_t>(sizeof(uint32_t) * modelData.indices.size());
	uint32_t vertexSizeInBytes = static_cast<uint32_t>(sizeof(Vertex) * modelData.vertices.size());

	result->node = modelData.rootNode;
	
	// インデックス
	result->indexResource = directXDevice->CreateBufferResuorce(indexSizeInBytes);

	Lamb::SafePtr<uint32_t> indexMap = nullptr;
	result->indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexMap));
	std::copy(modelData.indices.begin(), modelData.indices.end(), indexMap);
	result->indexResource->Unmap(0, nullptr);

	result->indexNumber = static_cast<uint32_t>(modelData.indices.size());
	result->indexView.SizeInBytes = indexSizeInBytes;
	result->indexView.Format = DXGI_FORMAT_R32_UINT;
	result->indexView.BufferLocation = result->indexResource->GetGPUVirtualAddress();

	// 頂点
	result->vertexResource = directXDevice->CreateBufferResuorce(vertexSizeInBytes);

	Lamb::SafePtr<Vertex> vertMap = nullptr;
	result->vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertMap));
	std::copy(modelData.vertices.begin(), modelData.vertices.end(), vertMap);
	result->vertexResource->Unmap(0, nullptr);

	result->vertexNumber = static_cast<uint32_t>(modelData.vertices.size());
	result->vertexView.SizeInBytes = vertexSizeInBytes;
	result->vertexView.StrideInBytes = static_cast<uint32_t>(sizeof(Vertex));
	result->vertexView.BufferLocation = result->vertexResource->GetGPUVirtualAddress();

	return result.release();
}
