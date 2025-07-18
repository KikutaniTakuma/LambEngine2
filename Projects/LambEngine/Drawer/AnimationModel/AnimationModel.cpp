/// ==================================
/// ==  AnimationModelクラスの定義  ==
/// ==================================

#include "AnimationModel.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"

AnimationModel::AnimationModel(const std::string& fileName) :
	AnimationModel()
{
	Load(fileName);
}

void AnimationModel::Load(const std::string& fileName) {
	Lamb::SafePtr renderContextManager = RenderContextManager::GetInstance();

	renderContextManager->LoadSkinAnimationModel<uint32_t, 1>(
		LoadFileNames{
			.resourceFileName = fileName,
			.shaderName{
				.vsFileName = "./Shaders/ModelShader/ModelAnimation.VS.hlsl",
				.psFileName = "./Shaders/ModelShader/Model.PS.hlsl",
			}
		},
		2
	);

	pRenderSet = renderContextManager->Get(
		LoadFileNames{
			.resourceFileName = fileName,
			.shaderName{
				.vsFileName = "./Shaders/ModelShader/ModelAnimation.VS.hlsl",
				.psFileName = "./Shaders/ModelShader/Model.PS.hlsl",
			}
		}
	);


	pAnimator_.reset();
	pAnimator_ = std::make_unique<Animator>();
	pAnimator_->Load(fileName);

	pSkeleton_ = std::make_unique<Skeleton>(Lamb::CreateSkeleton(pRenderSet->GetNode()));
	pSkinCluster_.reset(SkinCluster::CreateSkinCluster(*pSkeleton_, *pRenderSet->GetModelData()));
}

void AnimationModel::Update() {
	pAnimator_->Debug("animation model");
	pAnimator_->Update(*pSkeleton_);
	pSkeleton_->Update();
	pSkinCluster_->Update(*pSkeleton_);
}

void AnimationModel::Draw(
	const Mat4x4& worldMatrix,
	const Mat4x4& camera,
	uint32_t color,
	BlendType blend,
	Model::ShaderData shaderdata
) {
	Lamb::SafePtr renderContext = pRenderSet->GetRenderContextDowncast<SkinRenderContext<Model::ShaderData, 1>>(blend);
	renderContext->SetSkinCluster(pSkinCluster_.get());
	renderContext->SetShaderStruct(shaderdata);

	BaseDrawer::Draw(worldMatrix, camera, color, blend);
	pSkeleton_->Draw(worldMatrix, camera);
}
