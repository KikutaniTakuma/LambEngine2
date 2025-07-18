/// ====================================
/// ==  AnimationManagerクラスの定義  ==
/// ====================================


#include "AnimationManager.h"
#include "Error/Error.h"
#include "Utils/SafeDelete.h"

#include "../VertexIndexDataLoader/VertexIndexDataLoader.h"

Lamb::SafePtr<AnimationManager> AnimationManager::pInstance_ = nullptr;

void AnimationManager::Initialize() {
	if (pInstance_) {
		throw Lamb::Error::Code<AnimationManager>("Already created", ErrorPlace);
	}

	pInstance_.reset(new AnimationManager());

	Lamb::AddLog("Initialize AnimationManager succeeded");
}

void AnimationManager::Finalize() {
	pInstance_.reset();
	Lamb::AddLog("Finalize AnimationManager succeeded");
}

AnimationManager* const AnimationManager::GetInstance()
{
	return pInstance_.get();
}

void AnimationManager::LoadAnimations(const std::string& fileName) {
	auto isExist = animationData_.find(fileName);

	if (isExist == animationData_.end()) {
		animationData_[fileName].reset(VertexIndexDataLoader::LoadAnimation(fileName));
	}
}

Animations* const AnimationManager::GetAnimations(const std::string& fileName)
{
	if (not animationData_[fileName]) {
		throw Lamb::Error::Code<AnimationManager>("This file does not load -> " + fileName, ErrorPlace);
	}
	return animationData_[fileName].get();
}


