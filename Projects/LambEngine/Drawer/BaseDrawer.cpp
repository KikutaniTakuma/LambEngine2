/// ==============================
/// ==  BaseDrawerクラスの定義  ==
/// ==============================

#include "BaseDrawer.h"
#include "Utils/SafePtr.h"

#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"

#include "Utils/EngineInfo.h"

#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE


BaseDrawer::BaseDrawer() :
	pRenderSet(nullptr),
	isUseMeshShader_(Lamb::IsCanUseMeshShader())
{}

void BaseDrawer::Draw(const Mat4x4& worldMatrix, const Mat4x4& camera, uint32_t color, BlendType blend)
{
	Lamb::SafePtr<RenderData> render;
	if (isUseMeshShader_ and pMeshRenderSet) {
		render = pMeshRenderSet->GetRenderContext(blend);
	}
	else {
		render = pRenderSet->GetRenderContext(blend);
	}

	render->SetWVPMatrix({
		.worldMat = worldMatrix,
		.cameraMat = camera
		}
	);

	render->SetColor(color);

	render->AddDrawCount();
}
