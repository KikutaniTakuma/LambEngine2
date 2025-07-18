/// ==================================
/// ==  TextureManagerクラスの宣言  ==
/// ==================================


#pragma once
#include "Texture/Texture.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Utils/SafePtr.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <queue>
#include <thread>

/// <summary>
/// テクスチャの管理
/// </summary>
class TextureManager {
public:
	static const std::string kWhiteTexturePath;

private:
	TextureManager();
	TextureManager(const TextureManager&) = delete;
	TextureManager(TextureManager&&) noexcept = delete;

	TextureManager& operator=(const TextureManager&) = delete;
	TextureManager& operator=(TextureManager&&) noexcept = delete;
public:
	~TextureManager();

public:
	static TextureManager* const GetInstance();

	static void Initialize();

	static void Finalize();

private:
	static Lamb::SafePtr<TextureManager> pInstance_;


/// <summary>
/// ロード
/// </summary>
public:
	void LoadTexture(const std::string& fileName);

public:
	[[nodiscard]] uint32_t GetHandle(const std::string& fileName);

	[[nodiscard]] Texture* const GetTexture(const std::string& fileName);

public:
	/// <summary>
	/// ホワイトテクスチャのハンドルを渡す
	/// </summary>
	/// <returns>基本的に0が返ってくる</returns>
	[[nodiscard]] uint32_t GetWhiteTex();

	/// <summary>
	/// VRAMにテクスチャデータを渡す
	/// </summary>
	void UploadTextureData();

	/// <summary>
	/// dramから解放
	/// </summary>
	void ReleaseIntermediateResource();

	/// <summary>
	/// テクスチャセット
	/// </summary>
	/// <param name="texIndex">テクスチャインデックス</param>
	/// <param name="rootParam">ルートパラメータインデックス</param>
	void Use(uint32_t texIndex, UINT rootParam);


private:
	class CbvSrvUavHeap* srvHeap_;

	/// <summary>
	/// Textureのコンテナ(キー値: ファイルネーム  コンテナデータ型: Texture*)
	/// </summary>
	std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
	bool thisFrameLoadFlg_;

	std::unique_ptr<DirectXCommand> directXCommand_;
};