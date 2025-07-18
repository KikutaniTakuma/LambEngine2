/// ==============================
/// ==  CloudTex2Dクラスの定義  ==
/// ==============================

#include "CloudTex2D.h"

#include "Engine/Graphics/TextureManager/TextureManager.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"
#include "Utils/SafePtr.h"
#include "Utils/EngineInfo.h"

#include "Engine/Graphics/RenderingManager/RenderingManager.h"



const LoadFileNames CloudTex2D::kFileNames_ =
LoadFileNames{
	.resourceFileName{"./Resources/EngineResources/Texture2D/Texture2D.obj"},
	.shaderName{
		.vsFileName = "./Shaders/CloudTex2D/CloudTex2D.VS.hlsl",
		.psFileName = "./Shaders/CloudTex2D/CloudTex2D.PS.hlsl"
	}
};

const MeshLoadFileNames CloudTex2D::kMeshFileNames_ = MeshLoadFileNames{
	.resourceFileName{"./Resources/EngineResources/Texture2D/Texture2D.obj"},
	.shaderName{
		.asFileName = "./Shaders/CloudTex2D/CloudMeshTex2D.AS.hlsl",
		.msFileName = "./Shaders/CloudTex2D/CloudMeshTex2D.MS.hlsl",
		.psFileName = "./Shaders/CloudTex2D/CloudMeshTex2D.PS.hlsl"
	}
};

CloudTex2D::CloudTex2D() :
	BaseDrawer()
{}

void CloudTex2D::Load() {
	Lamb::SafePtr renderContextManager = RenderContextManager::GetInstance();

	// メッシュシェーダーが読み込む
	if (Lamb::IsCanUseMeshShader()) {
		// リソースとメッシュシェーダー読み込み
		renderContextManager->LoadMesh<CloudTex2D::ShaderData, CloudTex2D::kMaxDrawCount>(kMeshFileNames_, 4);

		pMeshRenderSet = renderContextManager->Get(kMeshFileNames_);
	}

	// リソースとシェーダー読み込み
	renderContextManager->Load<CloudTex2DRenderContext>(kFileNames_, 4);

	pRenderSet = renderContextManager->Get(kFileNames_);
}

void CloudTex2D::Draw(
	const Mat4x4& worldMatrix, 
	const Mat4x4& uvTransform, 
	const Mat4x4& camera, 
	uint32_t textureID, 
	float32_t3 randomVec,
	uint32_t color, 
	BlendType blend
) {
#ifdef USE_DEBUG_CODE
	isUseMeshShader_ = RenderingManager::GetInstance()->GetIsUseMeshShader();
#endif // USE_DEBUG_CODE

	if (isUseMeshShader_ and pMeshRenderSet) {
		Lamb::SafePtr renderContext = pMeshRenderSet->GetRenderContextDowncast<MeshRenderContext<CloudTex2D::ShaderData, CloudTex2D::kMaxDrawCount>>(blend);

		renderContext->SetShaderStruct(
			ShaderData{
				.uvTransform = uvTransform,
				.randomVec = randomVec,
				.textureID = textureID
			}
		);
	}
	else {
		Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<CloudTex2DRenderContext>(blend);

		renderContext->SetShaderStruct(
			ShaderData{
				.uvTransform = uvTransform,
				.randomVec = randomVec,
				.textureID = textureID
			}
		);
	}

	BaseDrawer::Draw(worldMatrix, camera, color, blend);
}

void CloudTex2D::Draw(const CloudTex2D::Data& data) {
#ifdef USE_DEBUG_CODE
	isUseMeshShader_ = RenderingManager::GetInstance()->GetIsUseMeshShader();
#endif // USE_DEBUG_CODE

	if (isUseMeshShader_ and pMeshRenderSet) {
		Lamb::SafePtr renderContext = pMeshRenderSet->GetRenderContextDowncast<MeshRenderContext<CloudTex2D::ShaderData, CloudTex2D::kMaxDrawCount>>(data.blend);

		renderContext->SetShaderStruct(
			ShaderData{
				.uvTransform = data.uvTransform,
				.randomVec = data.vec,
				.textureID = data.textureID
			}
		);
	}
	else {
		Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<CloudTex2DRenderContext>(data.blend);

		renderContext->SetShaderStruct(
			ShaderData{
				.uvTransform = data.uvTransform,
				.randomVec = data.vec,
				.textureID = data.textureID
			}
		);
	}

	BaseDrawer::Draw(data.worldMatrix, data.camera, data.color, data.blend);
}
