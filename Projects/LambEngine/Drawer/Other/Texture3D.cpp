/// =========================
/// ==  Texture3Dクラスの定義  ==
/// =========================

#include "Texture3D.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"
#include "Engine/Graphics/RenderingManager/RenderingManager.h"
#include "Utils/SafePtr.h"
#include "../DrawerManager.h"

#include "Utils/EngineInfo.h"

Texture3D::Texture3D(const std::string& fileName):
	Texture3D()
{
	Load(fileName);
}

void Texture3D::Load(const std::string& fileName) {
	Lamb::SafePtr renderContextManager = RenderContextManager::GetInstance();

	LoadFileNames fileNames = {
		.resourceFileName = fileName,
		.shaderName{
			.vsFileName = "./Shaders/Texture3DShader/Texture3D.VS.hlsl",
			.psFileName = "./Shaders/Texture3DShader/Texture3D.PS.hlsl",
		}
	};

	MeshLoadFileNames meshFileNames = {
		.resourceFileName = fileName,
		.shaderName{
			.asFileName = "./Shaders/Texture3DShader/MeshTexture3D.AS.hlsl",
			.msFileName = "./Shaders/Texture3DShader/MeshTexture3D.MS.hlsl",
			.psFileName = "./Shaders/Texture3DShader/MeshTexture3D.PS.hlsl"
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

void Texture3D::Draw(
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

void Texture3D::Draw(const Data& data) {
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

const Node& Texture3D::GetNode() const
{
	return pRenderSet->GetNode();
}

const ModelData& Texture3D::GetModelData() const {
	return *pRenderSet->GetModelData();
}


