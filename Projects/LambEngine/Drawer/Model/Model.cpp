/// =========================
/// ==  Modelクラスの定義  ==
/// =========================

#include "Model.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"
#include "Engine/Graphics/RenderingManager/RenderingManager.h"
#include "Utils/SafePtr.h"
#include "../DrawerManager.h"

#include "Utils/EngineInfo.h"

Model::Model(const std::string& fileName):
	Model()
{
	Load(fileName);
}

void Model::Load(const std::string& fileName) {
	Lamb::SafePtr renderContextManager = RenderContextManager::GetInstance();

	LoadFileNames fileNames = {
		.resourceFileName = fileName,
		.shaderName{
			.vsFileName = "./Shaders/ModelShader/Model.VS.hlsl",
			.psFileName = "./Shaders/ModelShader/Model.PS.hlsl",
		}
	};

	MeshLoadFileNames meshFileNames = {
		.resourceFileName = fileName,
		.shaderName{
			.asFileName = "./Shaders/ModelShader/MeshModel.AS.hlsl",
			.msFileName = "./Shaders/ModelShader/MeshModel.MS.hlsl",
			.psFileName = "./Shaders/ModelShader/MeshModel.PS.hlsl"
		}
	};

	// メッシュシェーダーが読み込む
	if (Lamb::IsCanUseMeshShader()) {
		// リソースとメッシュシェーダー読み込み
		renderContextManager->LoadMesh<ShaderData, kMaxDrawCount>(meshFileNames, 4);
		
		pMeshRenderSet = renderContextManager->Get(meshFileNames);
	}

	// リソースとシェーダー読み込み
	renderContextManager->Load<RenderContext<ShaderData, kMaxDrawCount>>(fileNames, 4);

	pRenderSet = renderContextManager->Get(fileNames);

	backGroundTextureIndex_ = static_cast<int32_t>(RenderingManager::GetInstance()->GetBackGroundTexture());
}

void Model::Draw(
	const Mat4x4& worldMatrix,
	const Mat4x4& camera, 
	uint32_t color, 
	BlendType blend, 
	ShaderData shaderData
) {
#ifdef USE_DEBUG_CODE
	isUseMeshShader_ = RenderingManager::GetInstance()->GetIsUseMeshShader();
#endif // USE_DEBUG_CODE

	if (blend == BlendType::kNone) {
		shaderData.isLighting = 0;
		shaderData.isEffect = 0;
	}
	
	if (isUseMeshShader_ and pMeshRenderSet) {
		Lamb::SafePtr renderContext = pMeshRenderSet->GetRenderContextDowncast<MeshRenderContext<ShaderData, kMaxDrawCount>>(blend);
		renderContext->SetShaderStruct(
			ShaderData{
				.isLighting = shaderData.isLighting,
				.isEffect = shaderData.isEffect,
				.backGroundTextureIndex = backGroundTextureIndex_,
				.gausState = {
					.dir = shaderData.gausState.dir,
					.sigma = shaderData.gausState.sigma,
					.kernelSize = shaderData.gausState.kernelSize
				}
			}
		);
	}
	else {
		Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<RenderContext<ShaderData, kMaxDrawCount>>(blend);
		renderContext->SetShaderStruct(
			ShaderData{
				.isLighting = shaderData.isLighting,
				.isEffect = shaderData.isEffect,
				.backGroundTextureIndex = backGroundTextureIndex_,
				.gausState = {
					.dir = shaderData.gausState.dir,
					.sigma = shaderData.gausState.sigma,
					.kernelSize = shaderData.gausState.kernelSize
				}
			}
		);
	}

	BaseDrawer::Draw(worldMatrix, camera, color, blend);
}

void Model::Draw(const Data& data) {
#ifdef USE_DEBUG_CODE
	isUseMeshShader_ = RenderingManager::GetInstance()->GetIsUseMeshShader();
#endif // USE_DEBUG_CODE

	ShaderData shaderData = data.shaderData;

	if (data.blend == BlendType::kNone) {
		shaderData.isLighting = 0;
		shaderData.isEffect = 0;
	}

	if (isUseMeshShader_ and pMeshRenderSet) {
		Lamb::SafePtr renderContext = pMeshRenderSet->GetRenderContextDowncast<MeshRenderContext<ShaderData, kMaxDrawCount>>(data.blend);
		renderContext->SetShaderStruct(shaderData);
	}
	else {
		Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<RenderContext<ShaderData, kMaxDrawCount>>(data.blend);
		renderContext->SetShaderStruct(shaderData);
	}

	BaseDrawer::Draw(data.worldMatrix, data.camera, data.color, data.blend);
}

const Node& Model::GetNode() const
{
	return pRenderSet->GetNode();
}

const ModelData& Model::GetModelData() const {
	return *pRenderSet->GetModelData();
}



/// 
/// ==================================================================================================================
/// 

void ModelInstance::Load(const std::string& fileName) {
	DrawerManager::GetInstance()->LoadModel(fileName);
	pModel_ = DrawerManager::GetInstance()->GetModel(fileName);
}

void ModelInstance::Update()
{
	worldMat_ = Mat4x4::MakeAffin(scale, rotate, pos);
	if (pParent_.have()) {
		worldMat_ *= pParent_->worldMat_;
	}
}

void ModelInstance::Draw(const Mat4x4& cameraMat)
{
	if (pModel_.have()) {
		pModel_->Draw(
			worldMat_,
			cameraMat,
			color,
			blend,
			shaderData
		);
	}
}

void ModelInstance::SetParent(ModelInstance* parent)
{
	pParent_ = parent;
}

const Mat4x4& ModelInstance::GetWorldMatrix() const
{
	return worldMat_;
}
