/// =================================
/// ==  ShaderManagerクラスの定義  ==
/// =================================


#include "ShaderManager.h"
#include "../ShaderFactory/ShaderFactory.h"
#include "Utils/ConvertString.h"
#include "Utils/SafeDelete.h"
#include <cassert>
#include <format>
#include "Utils/ExecutionLog.h"

Lamb::SafePtr<ShaderManager> ShaderManager::pInstance_ = nullptr;

ShaderManager::ShaderManager() {
	vertexShader_.clear();
	hullShader_.clear();
	domainShader_.clear();
	geometoryShader_.clear();
	pixelShader_.clear();

	ShaderFactory::Initialize();

	shaderFactory_ = ShaderFactory::GetInstance();

	Lamb::AddLog("Initialize ShaderManager succeeded");
}

ShaderManager::~ShaderManager() {
	Lamb::AddLog("Finalize ShaderManager succeeded");
}

void ShaderManager::Initialize() {
	pInstance_.reset(new ShaderManager());
}

void ShaderManager::Finalize() {
	pInstance_.reset();
}

IDxcBlob* const ShaderManager::LoadVertexShader(const std::string& fileName) {
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = vertexShader_.find(fileName);
	if (itr == vertexShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"vs_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		vertexShader_.insert(std::make_pair(fileName, shader.get()));
	}
	return vertexShader_[fileName].Get();
}
IDxcBlob* const ShaderManager::LoadHullShader(const std::string& fileName) {
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = hullShader_.find(fileName);
	if (itr == hullShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"hs_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		hullShader_.insert(std::make_pair(fileName, shader.get()));
	}

	return hullShader_[fileName].Get();
}
IDxcBlob* const ShaderManager::LoadDomainShader(const std::string& fileName) {
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = domainShader_.find(fileName);
	if (itr == domainShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"ds_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		domainShader_.insert(std::make_pair(fileName, shader.get()));
	}
	return domainShader_[fileName].Get();
}
IDxcBlob* const ShaderManager::LoadGeometoryShader(const std::string& fileName) {
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = geometoryShader_.find(fileName);
	if (itr == geometoryShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"gs_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		geometoryShader_.insert(std::make_pair(fileName, shader.get()));
	}
	return geometoryShader_[fileName].Get();
}
IDxcBlob* const ShaderManager::LoadPixelShader(const std::string& fileName) {
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = pixelShader_.find(fileName);
	if (itr == pixelShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"ps_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		pixelShader_.insert(std::make_pair(fileName, shader.get()));
	}
	return pixelShader_[fileName].Get();
}

IDxcBlob* const ShaderManager::LoadAmplificationShader(const std::string& fileName)
{
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = amplificationShader_.find(fileName);
	if (itr == amplificationShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"as_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		amplificationShader_.insert(std::make_pair(fileName, shader.get()));
	}
	return amplificationShader_[fileName].Get();
}

IDxcBlob* const ShaderManager::LoadMeshShader(const std::string& fileName)
{
	if (fileName.empty()) {
		return nullptr;
	}

	auto itr = meshShader_.find(fileName);
	if (itr == meshShader_.end()) {
		Lamb::SafePtr shader = shaderFactory_->CompileShader(ConvertString(fileName), L"ms_6_5");
		shader.NullCheck<ShaderManager>(ErrorPlace);
		meshShader_.insert(std::make_pair(fileName, shader.get()));
	}
	return meshShader_[fileName].Get();
}
