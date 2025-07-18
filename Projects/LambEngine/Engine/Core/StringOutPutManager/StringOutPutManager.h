/// =======================================
/// ==  StringOutPutManagerクラスの宣言  ==
/// =======================================


#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "../externals/DirectXTK12/Inc/SpriteFont.h"
#include "../externals/DirectXTK12/Inc/ResourceUploadBatch.h"
#pragma comment(lib, "DirectXTK12.lib")
#include "Utils/SafePtr.h"

class StringOutPutManager {
private:
	StringOutPutManager();
	StringOutPutManager(const StringOutPutManager&) = delete;
	StringOutPutManager(StringOutPutManager&) = delete;

	StringOutPutManager& operator=(const StringOutPutManager&) = delete;
	StringOutPutManager& operator=(StringOutPutManager&&) = delete;
public:
	~StringOutPutManager();

public:
	static void Initialize();

	static void Finalize();

	static StringOutPutManager* const GetInstance();

private:
	static Lamb::SafePtr<StringOutPutManager> pInstance_;

public:
	/// <summary>
	/// フォントをロードする
	/// </summary>
	/// <param name="fontName">フォントファイルパス</param>
	void LoadFont(const std::string& fontName);

	DirectX::SpriteFont* const GetFont(const std::string& fontName);

	DirectX::SpriteBatch* const GetBatch(const std::string& fontName);

	/// <summary>
	/// メモリコミット
	/// </summary>
	void GmemoryCommit();

private:
	std::unique_ptr<DirectX::GraphicsMemory> gmemory_;
	std::unordered_map<std::string, std::unique_ptr<DirectX::SpriteFont>> spriteFonts_;
	std::unordered_map<std::string, std::unique_ptr<DirectX::SpriteBatch>> spriteBatch_;
};