/// =================================
/// ==  ShaderFactoryクラスの宣言  ==
/// =================================


#pragma once
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include <string>

#include "Engine/EngineUtils/LambPtr/LambPtr.h"

class ShaderFactory {
private:
	ShaderFactory();
	ShaderFactory(const ShaderFactory&) = delete;
	ShaderFactory(ShaderFactory&&)  =delete;
	~ShaderFactory() = default;

	ShaderFactory& operator=(const ShaderFactory&) = delete;
	ShaderFactory& operator=(ShaderFactory&&) = delete;

public:
	static void Initialize();

	static void Finalize();

	static ShaderFactory* const GetInstance();

private:
	static ShaderFactory* pInstance_;

public:
	/// <summary>
	/// hlslをコンパイルする
	/// </summary>
	/// <param name="filePath"> CompilerするShaderファイルへのパス</param>
	/// <param name="profile">プロファイル</param>
	/// <returns>バイナリコード</returns>
	[[nodiscard]] IDxcBlob* CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile
	);

private:
	Lamb::LambPtr<IDxcUtils> dxcUtils_;
	Lamb::LambPtr<IDxcCompiler3> dxcCompiler_;
	Lamb::LambPtr<IDxcIncludeHandler> includeHandler_;
};