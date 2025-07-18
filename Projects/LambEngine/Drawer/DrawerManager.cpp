/// =================================
/// ==  DrawerManagerクラスの定義  ==
/// =================================

#include "DrawerManager.h"

Lamb::SafePtr<DrawerManager> DrawerManager::pInstance_ = nullptr;

DrawerManager::DrawerManager():
	tex2D_(),
	textureManager_(nullptr),
	models_()
{
	textureManager_ = TextureManager::GetInstance();

	tex2D_ = std::make_unique<Texture2D>();
	tex2D_->Load();

	Lamb::AddLog("Initialize DrawerManager succeeded");
}

DrawerManager::~DrawerManager(){ 
	Lamb::AddLog("Finalize DrawerManager succeeded");
}

DrawerManager* DrawerManager::GetInstance()
{
	return pInstance_.get();
}

void DrawerManager::Initialize() {
	pInstance_.reset(new DrawerManager());
}

void DrawerManager::Finalize() {
	pInstance_.reset();
}

Texture2D* const DrawerManager::GetTexture2D() const
{
	return tex2D_.get();
}

uint32_t DrawerManager::GetTexture(const std::string& fileName)
{
	return textureManager_->GetHandle(fileName);
}

void DrawerManager::LoadTexture(const std::string& fileName) {
	textureManager_->LoadTexture(fileName);
}

size_t DrawerManager::LoadModel(const std::string& fileName) {
	size_t key = std::hash<std::string>()(fileName);
	auto isExist = models_.find(key);

	if (isExist == models_.end()) {
		std::unique_ptr<Model> newModel = std::make_unique<Model>(fileName);

		models_.insert(std::make_pair(key, newModel.release()));
	}

	return key;
}

Model* const DrawerManager::GetModel(const std::string& fileName)
{
	size_t key = std::hash<std::string>()(fileName);
	if (not models_[key]) {
		throw Lamb::Error::Code<DrawerManager>("this model is not loaded -> " + fileName, ErrorPlace);
	}

	return models_[key].get();
}

Model* const DrawerManager::GetModel(size_t key)
{
	if (not models_[key]) {
		throw Lamb::Error::Code<DrawerManager>("this model is not loaded -> " + key, ErrorPlace);
	}

	return models_[key].get();
}


