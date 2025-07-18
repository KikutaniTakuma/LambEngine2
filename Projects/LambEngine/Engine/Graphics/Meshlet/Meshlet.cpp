/// ===================
/// ==  Meshlet関係  ==
/// ===================



#include "Meshlet.h"

#include "../VertexIndexDataManager/VertexIndexDataManager.h"

#include "DirectXMesh/DirectXMesh.h"

#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"

ResMesh* MeshLoader::LoadMesh(const std::string& fileName)
{
	std::unique_ptr<ResMesh> mesh = std::make_unique<ResMesh>();


	Lamb::SafePtr vertexIndexdDataManager = VertexIndexDataManager::GetInstance();
	vertexIndexdDataManager->LoadModel(fileName);

	Lamb::SafePtr<const ModelData> modelData = vertexIndexdDataManager->GetModelData(fileName);


	ParseMesh(*mesh, modelData);

	return mesh.release();
}

ResMesh* MeshLoader::LoadMesh(Lamb::SafePtr<const ModelData> pSrcMesh)
{
	std::unique_ptr<ResMesh> mesh = std::make_unique<ResMesh>();
	ParseMesh(*mesh, pSrcMesh);

	return mesh.release();
}

void MeshLoader::ParseMesh(ResMesh& dstMesh, Lamb::SafePtr<const ModelData> pSrcMesh) {
	// Nullチェック
	pSrcMesh.NullCheck(FilePlace);

	// 頂点データセット
	dstMesh.vertices = pSrcMesh->vertices;

	// インデックスデータセット
	dstMesh.indices = pSrcMesh->indices;

	// メッシュレット作成用
	std::vector<uint32_t> indices;
	std::vector<DirectX::XMFLOAT3> positions;

	indices.resize(dstMesh.indices.size());
	positions.resize(dstMesh.vertices.size());

	// データ代入
	std::copy(dstMesh.indices.cbegin(), dstMesh.indices.cend(), indices.begin());
	for (size_t count = 0; const auto & vertex : dstMesh.vertices) {
		SetVector3ToDirectXXMFLOAT3(positions[count], vertex.pos.GetVector3());
		count++;
	}

	// 出力変数
	std::vector<DirectX::Meshlet> meshlets;
	std::vector<uint8_t> uniqueVertexIB;
	std::vector<DirectX::MeshletTriangle> primitiveIndices;

	// メッシュレット作成
	HRESULT hr = DirectX::ComputeMeshlets(
		indices.data(), indices.size() / 3,
		positions.data(), positions.size(),
		nullptr,
		meshlets,
		uniqueVertexIB,
		primitiveIndices,
		kMaxMeshletVertices, 
		kMaxMeshletPrimirives
	);

	// 失敗したらthrowする
	if (FAILED(hr)) {
		throw Lamb::Error::Code<MeshLoader>("DirectX::ComputeMeshlets failed", ErrorPlace);
	}


	// データを設定

	dstMesh.meshlets = std::move(meshlets);

	dstMesh.uniqueVertexIndices.resize(uniqueVertexIB.size());
	std::memcpy(dstMesh.uniqueVertexIndices.data(), uniqueVertexIB.data(), sizeof(uint8_t) * uniqueVertexIB.size());

	// パックして値を入れる
	dstMesh.primitiveIndices.resize(primitiveIndices.size());
	for (size_t count = 0; auto& primitiveIndex : primitiveIndices) {
		dstMesh.primitiveIndices[count] = primitiveIndex.i0;
		dstMesh.primitiveIndices[count] <<= 10;
		dstMesh.primitiveIndices[count] += primitiveIndex.i1;
		dstMesh.primitiveIndices[count] <<= 10;
		dstMesh.primitiveIndices[count] += primitiveIndex.i2;


		count++;
	}

	std::memcpy(dstMesh.primitiveIndices.data(), primitiveIndices.data(), sizeof(DirectX::MeshletTriangle) * primitiveIndices.size());
}

std::unique_ptr<MeshletManager> MeshletManager::pInstance_;

MeshletManager* const MeshletManager::GetInstance() {
	return pInstance_.get();
}

void MeshletManager::Initialize() {
	pInstance_.reset(new MeshletManager());
}

void MeshletManager::Finalize() {
	pInstance_.reset();
}

void MeshletManager::LoadMesh(const std::string& fileName) {
	if (not meshlets_.contains(fileName)) {
		meshlets_.insert(std::make_pair(fileName, std::make_pair(MeshLoader::LoadMesh(fileName), std::make_unique<MeshShaderData>())));

		auto& meshAndMeshData = meshlets_[fileName];

		auto& resMesh = meshAndMeshData.first;
		auto& shaderData = meshAndMeshData.second;


		shaderData->gMeshlets.CreateBuffer(static_cast<uint32_t>(resMesh->meshlets.size()));
		shaderData->gMeshlets.MemCpy(resMesh->meshlets.data(), sizeof(DirectX::Meshlet) * resMesh->meshlets.size());
		
		shaderData->gVertices.CreateBuffer(static_cast<uint32_t>(resMesh->vertices.size()));
		shaderData->gVertices.MemCpy(resMesh->vertices.data(), sizeof(Vertex) * resMesh->vertices.size());
		
		shaderData->gUniqueVertexIndices.CreateBuffer(static_cast<uint32_t>(resMesh->uniqueVertexIndices.size()));
		shaderData->gUniqueVertexIndices.MemCpy(resMesh->uniqueVertexIndices.data(), sizeof(uint32_t) * resMesh->uniqueVertexIndices.size());
		
		shaderData->gPrimitiveIndices.CreateBuffer(static_cast<uint32_t>(resMesh->primitiveIndices.size()));
		shaderData->gPrimitiveIndices.MemCpy(resMesh->primitiveIndices.data(), sizeof(uint32_t) * resMesh->primitiveIndices.size());

		shaderData->meshletCount = static_cast<uint32_t>(shaderData->gMeshlets.size());
	}
}

const std::pair<std::unique_ptr<ResMesh>, std::unique_ptr<MeshShaderData>>& MeshletManager::GetMesh(const std::string& fileName)
{
	if (meshlets_.contains(fileName)) {
		return meshlets_[fileName];
	}
	else {
		throw Lamb::Error::Code<MeshletManager>("not loaded meshlet", ErrorPlace);
	}
}
