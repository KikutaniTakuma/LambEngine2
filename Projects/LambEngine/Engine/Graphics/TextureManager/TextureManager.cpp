/// ==================================
/// ==  TextureManagerクラスの定義  ==
/// ==================================



#include "TextureManager.h"
#include "Utils/ExecutionLog.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"
#include "Utils/SafeDelete.h"
#include <cassert>
#include <filesystem>

#include "Error/Error.h"


Lamb::SafePtr<TextureManager> TextureManager::pInstance_ = nullptr;
const std::string TextureManager::kWhiteTexturePath = "./Resources/EngineResources/white2x2.png";

TextureManager* const TextureManager::GetInstance() {
	return pInstance_.get();
}

void TextureManager::Initialize() {
	pInstance_.reset(new TextureManager());
	pInstance_.NullCheck<TextureManager>(ErrorPlace);
	pInstance_->LoadTexture(kWhiteTexturePath);
}

void TextureManager::Finalize() {
	pInstance_.reset();
}

TextureManager::TextureManager() :
	textures_(),
	thisFrameLoadFlg_(false)
{
	srvHeap_ = CbvSrvUavHeap::GetInstance();

	directXCommand_ = std::make_unique<DirectXCommand>();

	Lamb::AddLog("Initialize TextureManager succeeded");
}

TextureManager::~TextureManager() {
	textures_.clear();
	Lamb::AddLog("Finalize TextureManager succeeded");
}


void TextureManager::LoadTexture(const std::string& fileName) {
	if (!std::filesystem::exists(fileName)) [[unlikely]] {
		throw Lamb::Error::Code<TextureManager>("This file is not exist -> " + fileName, ErrorPlace);
	}

	auto itr = textures_.find(fileName);
	if (itr == textures_.end()) {
		auto tex = std::make_unique<Texture>();
		tex->Load(fileName, directXCommand_->GetCommandList());

		if (not tex->isLoad_) {
			throw Lamb::Error::Code<TextureManager>("Texture::Load failed -> " + fileName, ErrorPlace);
		}

		if (tex->GetIsCubemap()) {
			srvHeap_->CreateView((tex->GetBaseClassPtr()));
		}
		else {
			srvHeap_->CreateTextureView(tex.get());
		}
		textures_.insert(std::make_pair(fileName, std::move(tex)));

		thisFrameLoadFlg_ = true;

	}
}

uint32_t TextureManager::GetHandle(const std::string& fileName)
{
	auto itr = textures_.find(fileName);

	if (itr == textures_.end()) [[unlikely]] {
		throw Lamb::Error::Code<TextureManager>("This file is not loaded -> " + fileName , ErrorPlace);
	}

	return itr->second->GetHandleUINT();
}

Texture* const TextureManager::GetTexture(const std::string& fileName) {
	return textures_[fileName].get();
}

uint32_t TextureManager::GetWhiteTex() {
	return pInstance_->textures_[kWhiteTexturePath]->GetHandleUINT();
}

void TextureManager::UploadTextureData()
{
	if (thisFrameLoadFlg_) {
		// コマンドリストを確定させる
		directXCommand_->CloseCommandlist();

		// GPUにコマンドリストの実行を行わせる
		directXCommand_->ExecuteCommandList();

		directXCommand_->WaitForFinishCommnadlist();

		directXCommand_->ResetCommandlist();
	}
}

void TextureManager::ReleaseIntermediateResource() {
	if (thisFrameLoadFlg_) {
		for (auto& i : textures_) {
			i.second->ReleaseIntermediateResource();
		}

		thisFrameLoadFlg_ = false;
	}
}

void TextureManager::Use(uint32_t texIndex, UINT rootParam) {
	Lamb::SafePtr mainComlist = DirectXCommand::GetMainCommandlist()->GetCommandList();
	mainComlist->SetGraphicsRootDescriptorTable(
		rootParam, srvHeap_->GetGpuHeapHandle(texIndex));
}