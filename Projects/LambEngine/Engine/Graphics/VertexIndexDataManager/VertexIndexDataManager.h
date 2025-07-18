/// ==========================================
/// ==  VertexIndexDataManagerクラスの宣言  ==
/// ==========================================



#pragma once
#include <memory>
#include <thread>
#include <queue>

#include "Utils/SafePtr.h"
#include "../GraphicsStructs.h"

/// <summary>
/// メッシュを管理
/// </summary>
class VertexIndexDataManager final {
private:
	VertexIndexDataManager() = default;
	VertexIndexDataManager(const VertexIndexDataManager&) = delete;
	VertexIndexDataManager(VertexIndexDataManager&&) = delete;

	VertexIndexDataManager& operator=(const VertexIndexDataManager&) = delete;
	VertexIndexDataManager& operator=(VertexIndexDataManager&&) = delete;
public:
	~VertexIndexDataManager();

public:
	static VertexIndexDataManager* const GetInstance();

	static void Initialize();
	static void Finalize();

private:
	static Lamb::SafePtr<VertexIndexDataManager> pInstance_;

public:
	/// <summary>
	/// モデルをロードしてコンテナに追加
	/// </summary>
	void LoadModel(const std::string& objFileName);
	
/// <summary>
/// ゲッター
/// </summary>
public:
	[[nodiscard]] VertexIndexData* GetVertexIndexData(const std::string& objFileName);
	[[nodiscard]] ModelData* GetModelData(const std::string& objFileName);

private:
	[[nodiscard]] VertexIndexData* CreateMesh(const ModelData& modelData);

private:
	std::unordered_map<std::string, std::unique_ptr<VertexIndexData>> vertexIndexData_;
	std::unordered_map<std::string, std::unique_ptr<ModelData>> modelData_;
};