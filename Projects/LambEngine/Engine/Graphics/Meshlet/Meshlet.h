/// ===================
/// ==  Meshlet関係  ==
/// ===================


#pragma once
#include <cstdint>
#include <vector>
#include <string>

#include <memory>

#include "../Shader/MeshShader/MeshShader.h"

class MeshLoader final {
public:
	static constexpr size_t kMaxMeshletVertices = 64llu;
	static constexpr size_t kMaxMeshletPrimirives = 126llu;
	static constexpr size_t kMaxThreadGroup = 1024llu;

private:
	MeshLoader() = default;
	~MeshLoader() = default;
	
private:
	MeshLoader(const MeshLoader&) = delete;
	MeshLoader(MeshLoader&&) = delete;

	MeshLoader& operator=(const MeshLoader&) = delete;
	MeshLoader& operator=(MeshLoader&&) = delete;


public:
	/// <summary>
	/// モデルをロードしてmeshletを作成する
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	/// <returns>meshletデータ</returns>
	static [[nodiscard]] ResMesh* LoadMesh(const std::string& fileName);

	/// <summary>
	/// モデルデータからmeshletを作成する
	/// </summary>
	/// <param name="pSrcMesh">モデルデータ</param>
	/// <returns></returns>
	static [[nodiscard]] ResMesh* LoadMesh(Lamb::SafePtr<const ModelData> pSrcMesh);

	static void ParseMesh(
		ResMesh& dstMesh, 
		Lamb::SafePtr<const ModelData> pSrcMesh
	);
};

class MeshletManager final {
private:
	MeshletManager() = default;
	MeshletManager(const MeshletManager&) = delete;
	MeshletManager(MeshletManager&&) = delete;

	MeshletManager& operator=(const MeshletManager&) = delete;
	MeshletManager& operator=(MeshletManager&&) = delete;
	
public:
	~MeshletManager() = default;

public:
	static MeshletManager* const GetInstance();

	static void Initialize();

	static void Finalize();

private:
	static std::unique_ptr<MeshletManager> pInstance_;

public:
	/// <summary>
	/// モデルをロードしてメッシュを作成し、コンテナに追加
	/// 
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	void LoadMesh(const std::string& fileName);

	const std::pair<std::unique_ptr<ResMesh>, std::unique_ptr<MeshShaderData>>& GetMesh(const std::string& fileName);

private:
	std::unordered_map<std::string, std::pair<std::unique_ptr<ResMesh>, std::unique_ptr<MeshShaderData>>> meshlets_;
};