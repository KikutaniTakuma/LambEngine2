#include "PlayerMiddleModel.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"
#include "Engine/Graphics/RenderingManager/RenderingManager.h"
#include "Utils/SafePtr.h"
#include "../DrawerManager.h"

#include "Utils/EngineInfo.h"

#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE



void PlayerMiddleModel::Load(const std::string& fileName)
{
	Lamb::SafePtr renderContextManager = RenderContextManager::GetInstance();

	LoadFileNames fileNames = {
		.resourceFileName = fileName,
		.shaderName{
			.vsFileName = "./Shaders/PlayerMiddleModelShader/PlayerMiddleModel.VS.hlsl",
			.psFileName = "./Shaders/PlayerMiddleModelShader/PlayerMiddleModel.PS.hlsl",
			.gsFileName = "./Shaders/PlayerMiddleModelShader/PlayerMiddleModel.GS.hlsl"
		}
	};

	// リソースとシェーダー読み込み
	renderContextManager->Load<RenderContext<PlayerModelData, kMaxDrawCount>>(fileNames, 4);

	pRenderSet = renderContextManager->Get(fileNames);


	backGroundTextureIndex_ = static_cast<int32_t>(RenderingManager::GetInstance()->GetBackGroundTexture());
}

void PlayerMiddleModel::Draw(
	const Mat4x4& worldMatrix,
	const Mat4x4& camera,
	PlayerModelData playerModelData,
	uint32_t color,
	BlendType blend
) {
	if (blend != BlendType::kAlphaEffect) {
		playerModelData.isEffect = 0;
		playerModelData.backGroundTextureIndex = 0;
	}
	else {
		playerModelData.backGroundTextureIndex = backGroundTextureIndex_;
	}

	Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<RenderContext<PlayerModelData, kMaxDrawCount>>(blend);
	renderContext->SetShaderStruct(playerModelData);

	BaseDrawer::Draw(worldMatrix, camera, color, blend);
}

#ifdef USE_DEBUG_CODE
void PlayerMiddleModel::PlayerModelData::Debug(const std::string& guiName) {
	if (ImGui::TreeNode(guiName.c_str())) {
		ImGui::DragFloat3("playerFrontPosition", this->playerFrontPosition.data(), 0.1f);
		ImGui::DragFloat3("playerBackPosition", this->playerBackPosition.data(), 0.1f);
		ImGui::DragFloat("baseLength", &this->baseLength, 0.001f);
		ImGui::DragFloat("middleSize", &this->middleSize, 0.001f);
		ImGui::DragFloat("sideSize", &this->sideSize, 0.01f);
		ImGui::SliderInt("Lighting", &this->isLighting, 0, 1);

		ImGui::TreePop();
	}

}
#endif // USE_DEBUG_CODE
