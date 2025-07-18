/// =================================
/// ==  ShaderManagerクラスの宣言  ==
/// =================================


#pragma once
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include <unordered_map>
#include <string>
#include "Engine/EngineUtils/LambPtr/LambPtr.h"
#include "Utils/SafePtr.h"

#include "../Shader.h"

/// <summary>
/// シェーダーを管理するクラス
/// </summary>
class ShaderManager {
private:
	ShaderManager();
	ShaderManager(const ShaderManager&)  =delete;
	ShaderManager(ShaderManager&&) = delete;
	
	ShaderManager& operator=(const ShaderManager&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;
public:
	~ShaderManager();

public:
	static void Initialize();

	static void Finalize();

	static inline ShaderManager* const GetInstance() {
		return pInstance_.get();
	}

/// <summary>
/// ロード関数(shaderの種類ごと)
/// </summary>
public:
	IDxcBlob* const LoadVertexShader(const std::string& fileName);
	IDxcBlob* const LoadHullShader(const std::string& fileName);
	IDxcBlob* const LoadDomainShader(const std::string& fileName);
	IDxcBlob* const LoadGeometoryShader(const std::string& fileName);
	IDxcBlob* const LoadPixelShader(const std::string& fileName);

	IDxcBlob* const LoadAmplificationShader(const std::string& fileName);
	IDxcBlob* const LoadMeshShader(const std::string& fileName);

private:
	static Lamb::SafePtr<ShaderManager> pInstance_;

private:
	class ShaderFactory* shaderFactory_;

	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> vertexShader_;
	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> hullShader_;
	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> domainShader_;
	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> geometoryShader_;
	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> pixelShader_;

	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> amplificationShader_;
	std::unordered_map<std::string, Lamb::LambPtr<IDxcBlob>> meshShader_;
};