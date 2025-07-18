#include "EffectDataManager.h"
#include <filesystem>
#include <Engine/Core/EffekseerControler/EffekseerControler.h>

#include <Error/Error.h>

Lamb::SafePtr<EffectDataManager> EffectDataManager::pInstance_;

EffectDataManager::EffectDataManager() {
	effekseerContoler_ = EffekseerControler::GetInstance();
}

const Lamb::SafePtr<EffectDataManager>& EffectDataManager::GetInstance() {
	return pInstance_;
}

void EffectDataManager::Initialize() {
	pInstance_.reset(new EffectDataManager());
}

void EffectDataManager::Finalize() {
	pInstance_.reset();
}

void EffectDataManager::Load(const std::string& fileName) {
	auto element = effects_.find(fileName);

	if (element == effects_.end()) {
		effects_[fileName] = Effekseer::Effect::Create(
			effekseerContoler_->GetEfkManager(),
			std::filesystem::path(fileName).u16string().c_str()
		);
	}
}

const Effekseer::EffectRef& EffectDataManager::Get(const std::string& fileName) {
	auto element = effects_.find(fileName);

	if (element == effects_.end()) {
		throw Lamb::Error::Code<EffectDataManager>(fileName + " is not loaded. Please load thi file", ErrorPlace);
	}

	return element->second;
}