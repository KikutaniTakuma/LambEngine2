/// ==========================
/// ==  Water2Dクラスの定義 ==
/// ==========================

#include "WaterTex2D.h"
#include "Engine/Graphics/TextureManager/TextureManager.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"
#include "Engine/Graphics/RenderingManager/RenderingManager.h"
#include "Utils/SafePtr.h"

#ifdef USE_DEBUG_CODE
#include "Utils/FileUtils.h"

#include "imgui.h"
#endif // USE_DEBUG_CODE


const LoadFileNames WaterTex2D::kFileNames_ =
LoadFileNames{
	.resourceFileName{"./Resources/Water/WaterSurface.glb"},
	.shaderName = ShaderFileNames{
		.vsFileName = "./Shaders/WaterTex2DShader/WaterTex2D.VS.hlsl",
		.psFileName = "./Shaders/WaterTex2DShader/WaterTex2D.PS.hlsl",
		.gsFileName = "./Shaders/WaterTex2DShader/WaterTex2D.GS.hlsl",
		.dsFileName = "./Shaders/WaterTex2DShader/WaterTex2D.DS.hlsl",
		.hsFileName = "./Shaders/WaterTex2DShader/WaterTex2D.HS.hlsl",
	}
};

WaterTex2D::WaterTex2D() :
	BaseDrawer()
{}

void WaterTex2D::Load()
{
	Lamb::SafePtr renderContextManager = RenderContextManager::GetInstance();

	renderContextManager->Load<WaterRenderContext>(kFileNames_, 4);

	pRenderSet = renderContextManager->Get(kFileNames_);

	Lamb::SafePtr textureManager = TextureManager::GetInstance();
	textureManager->LoadTexture("./Resources/Water/caustics_01.bmp");
}

void WaterTex2D::Draw(
	const Mat4x4& worldMatrix,
	const Mat4x4& camera,
	float32_t time,
	float32_t2 randomVec,
	float32_t density,
	const float32_t3& cameraPos,
	float32_t divisionMinLength,
	uint32_t edgeDivision,
	uint32_t insideDivision,
	WaveData waveData,
	const float32_t3& pointLightPos,
	float32_t pointLightRange,
	float32_t pointLightAngle,
	uint32_t color,
	BlendType blend
) {
	Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<WaterRenderContext>(blend);

	Vector2 clientSize = Lamb::ClientSize();

	renderContext->SetShaderStruct(
		ShaderData{
			.randomVec = randomVec,
			.density = density,
			.edgeDivision = std::clamp(edgeDivision, 1u, 64u),
			.insideDivision = std::clamp(insideDivision, 1u, 64u),
			.camerPosition = cameraPos,
			.distanceThreshold = divisionMinLength,
			.cameraDirection = RenderingManager::GetInstance()->GetCameraDirection(),
			.viewportMatrix = float32_t4x4::MakeViewPort(0.0f, 0.0f, clientSize.x, clientSize.y, 0.0f, 1.0f),
			.waveData = waveData,
			.atomosphericParam = RenderingManager::GetInstance()->GetAtmosphericParams(),
			.time = time,
			.pointLightPos = pointLightPos,
			.pointLightRange = pointLightRange,
			.pointLightAngle = pointLightAngle,
		}
	);

	BaseDrawer::Draw(worldMatrix, camera, color, blend);
}