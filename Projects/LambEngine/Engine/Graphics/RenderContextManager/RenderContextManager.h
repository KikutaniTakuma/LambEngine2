/// ========================================
/// ==  RenderContextManagerクラスの宣言  ==
/// ========================================



#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <list>

#include "RenderContext/RenderContext.h"
#include "../VertexIndexDataManager/VertexIndexDataManager.h"

class RenderContextManager final {
private:
	using Key = LoadFileNames;
	using MeshKey = MeshLoadFileNames;

private:
	RenderContextManager() = default;
	RenderContextManager(const RenderContextManager&) = delete;
	RenderContextManager(RenderContextManager&&) = delete;
public:
	~RenderContextManager();

	RenderContextManager& operator=(const RenderContextManager&) = delete;
	RenderContextManager& operator=(RenderContextManager&&) = delete;

public:
	static [[nodiscard]] RenderContextManager* const GetInstance();

	static void Initialize();

	static void Finalize();

private:
	static Lamb::SafePtr<RenderContextManager> pInstance_;


public:
	/// <summary>
	/// 描画に必要なものをロードしてコンテナに追加(1度追加してたら追加しない)
	/// </summary>
	/// <typeparam name="RenderContextType">レンダーコンテキストタイプ</typeparam>
	/// <param name="fileNames">リソースファイル名</param>
	/// <param name="numRenderTarget">レンダーターゲットの数</param>
	template<IsBasedRenderContext RenderContextType = RenderContext<>>
	void Load(const LoadFileNames& fileNames, uint32_t numRenderTarget = 1) {
		
		// すでにコンテナに追加しているか
		auto isExist = renderData_.find(fileNames);

		if (isExist != renderData_.end()) {
			return;
		}

		// 現在のコンテナを参照
		auto& currentRenderData = (isNowThreading_ ? threadRenderData_ : renderData_);

		// 追加
		currentRenderData.insert(std::make_pair(fileNames, std::make_unique<RenderSet>()));

		// 追加したのを参照
		RenderSet& currentRenderSet = *currentRenderData[fileNames];

		// shaderロード
		Shader shader = LoadShader(fileNames.shaderName);

		// 描画パイプライン作成
		const std::array<Pipeline*, BlendType::kNum>& pipelines = CreateGraphicsPipelines(shader, numRenderTarget);
		Pipeline* shadowPipeline = CreateShadowPipeline();

		// モデルデータ
		Lamb::SafePtr vertexIndexDataManager = VertexIndexDataManager::GetInstance();
		vertexIndexDataManager->LoadModel(fileNames.resourceFileName);
		VertexIndexData* vertexIndexData = vertexIndexDataManager->GetVertexIndexData(fileNames.resourceFileName);
		ModelData* modelData = vertexIndexDataManager->GetModelData(fileNames.resourceFileName);

		// 各ブレンドモードごとにデータを設定
		for (uint32_t i = 0; i < BlendType::kNum; i++) {
			std::unique_ptr<RenderContextType> renderContext = std::make_unique<RenderContextType>();

			renderContext->SetShadowPipeline(shadowPipeline);
			renderContext->SetVertexIndexData(vertexIndexData);
			renderContext->SetModelData(modelData);
			renderContext->SetPipeline(pipelines[i]);
			currentRenderSet.Set(renderContext.release(), BlendType(i));
		}

		std::unique_ptr<RenderContextType> renderContext = std::make_unique<RenderContextType>();

		renderContext->SetShadowPipeline(shadowPipeline);
		renderContext->SetVertexIndexData(vertexIndexData);
		renderContext->SetModelData(modelData);
		renderContext->SetPipeline(pipelines[0]);
		currentRenderSet.Set(renderContext.release(), BlendType::kAlphaEffect);

		// レンダーリストをリサイズ
		ResizeRenderList();
	}

	/// <summary>
	/// スキンアニメーション用。描画に必要なものをロードしてコンテナに追加(1度追加してたら追加しない)
	/// </summary>
	/// <typeparam name="RenderContextType">レンダーコンテキストタイプ</typeparam>
	/// <param name="fileNames">リソースファイル名</param>
	/// <param name="numRenderTarget">レンダーターゲットの数</param>
	template<class T = uint32_t, uint32_t bufferSize = RenderData::kMaxDrawInstance>
	void LoadSkinAnimationModel(const LoadFileNames& fileNames, uint32_t numRenderTarget = 1) {

		// すでにコンテナに追加しているか
		auto isExist = renderData_.find(fileNames);

		if (isExist != renderData_.end()) {
			return;
		}

		// 現在のコンテナを参照
		auto& currentRenderData = (isNowThreading_ ? threadRenderData_ : renderData_);

		// 追加
		currentRenderData.insert(std::make_pair(fileNames, std::make_unique<RenderSet>()));

		// 追加したのを参照
		RenderSet& currentRenderSet = *currentRenderData[fileNames];

		// shaderロード
		Shader shader = LoadShader(fileNames.shaderName);

		// 描画パイプライン作成
		const std::array<Pipeline*, BlendType::kNum>& pipelines = CreateSkinAnimationGraphicsPipelines(shader, numRenderTarget);
		Pipeline* shadowPipeline = CreateSkinAnimationShadowPipeline();

		// モデルデータ
		Lamb::SafePtr vertexIndexDataManager = VertexIndexDataManager::GetInstance();
		vertexIndexDataManager->LoadModel(fileNames.resourceFileName);
		VertexIndexData* vertexIndexData = vertexIndexDataManager->GetVertexIndexData(fileNames.resourceFileName);
		ModelData* modelData = vertexIndexDataManager->GetModelData(fileNames.resourceFileName);

		// 各ブレンドモードごとにデータを設定
		for (uint32_t i = 0; i < BlendType::kNum; i++) {
			std::unique_ptr<SkinRenderContext<T, bufferSize>> renderContext = std::make_unique<SkinRenderContext<T, bufferSize>>();

			renderContext->SetShadowPipeline(shadowPipeline);
			renderContext->SetVertexIndexData(vertexIndexData);
			renderContext->SetModelData(modelData);
			renderContext->SetPipeline(pipelines[i]);
			currentRenderSet.Set(renderContext.release(), BlendType(i));
		}

		std::unique_ptr<SkinRenderContext<T, bufferSize>> renderContext = std::make_unique<SkinRenderContext<T, bufferSize>>();

		renderContext->SetShadowPipeline(shadowPipeline);
		renderContext->SetVertexIndexData(vertexIndexData);
		renderContext->SetModelData(modelData);
		renderContext->SetPipeline(pipelines[0]);
		currentRenderSet.Set(renderContext.release(), BlendType::kAlphaEffect);

		// レンダーリストをリサイズ
		ResizeRenderList();
	}

	/// <summary>
	/// 描画に必要なものをロードしてコンテナに追加(1度追加してたら追加しない)
	/// </summary>
	/// <typeparam name="RenderContextType">レンダーコンテキストタイプ</typeparam>
	/// <param name="fileNames">リソースファイル名</param>
	/// <param name="numRenderTarget">レンダーターゲットの数</param>
	template<class T = uint32_t, uint32_t bufferSize = RenderData::kMaxDrawInstance>
	void LoadMesh(const MeshLoadFileNames& fileNames, uint32_t numRenderTarget = 1) {
		// すでにコンテナに追加しているか
		auto isExist = meshRenderData_.find(fileNames);

		if (isExist != meshRenderData_.end()) {
			return;
		}

		// 現在のコンテナを参照
		auto& currentRenderData = (isNowThreading_ ? threadMeshRenderData_ : meshRenderData_);

		// 追加
		currentRenderData.insert(std::make_pair(fileNames, std::make_unique<RenderSet>()));

		// 追加したのを参照
		RenderSet& currentRenderSet = *currentRenderData[fileNames];

		// shaderロード
		MeshShader shader = LoadMeshShader(fileNames.shaderName);

		// 描画パイプライン作成
		const std::array<Pipeline*, BlendType::kNum>& pipelines = CreateMeshShaderGraphicsPipelines(shader, numRenderTarget);
		Pipeline* shadowPipeline = CreateMeshShaderShadowPipeline();

		// モデルデータ
		Lamb::SafePtr vertexIndexDataManager = VertexIndexDataManager::GetInstance();
		vertexIndexDataManager->LoadModel(fileNames.resourceFileName);
		VertexIndexData* vertexIndexData = vertexIndexDataManager->GetVertexIndexData(fileNames.resourceFileName);


		Lamb::SafePtr meshManager = MeshletManager::GetInstance();
		meshManager->LoadMesh(fileNames.resourceFileName);
		
		const auto& mesh = meshManager->GetMesh(fileNames.resourceFileName);

		// 各ブレンドモードごとにデータを設定
		for (uint32_t i = 0; i < BlendType::kNum; i++) {
			std::unique_ptr<MeshRenderContext<T, bufferSize>> renderContext = std::make_unique<MeshRenderContext<T, bufferSize>>();

			renderContext->SetShadowPipeline(shadowPipeline);
			renderContext->SetVertexIndexData(vertexIndexData);
			renderContext->SetMeshShaderData(mesh.second.get());
			renderContext->SetPipeline(pipelines[i]);
			currentRenderSet.Set(renderContext.release(), BlendType(i));
		}

		std::unique_ptr<MeshRenderContext<T, bufferSize>> renderContext = std::make_unique<MeshRenderContext<T, bufferSize>>();

		renderContext->SetShadowPipeline(shadowPipeline);
		renderContext->SetVertexIndexData(vertexIndexData);
		renderContext->SetMeshShaderData(mesh.second.get());
		renderContext->SetPipeline(pipelines[0]);
		currentRenderSet.Set(renderContext.release(), BlendType::kAlphaEffect);

		// レンダーリストをリサイズ
		ResizeRenderList();
	}

	/// <summary>
	/// リソースファイルから描画構造体を取得する(deleteしてはいけない)
	/// </summary>
	/// <param name="fileNames"></param>
	/// <returns></returns>
	[[nodiscard]] RenderSet* const Get(const LoadFileNames& fileNames);

	/// <summary>
	/// リソースファイルから描画構造体を取得する(deleteしてはいけない)
	/// </summary>
	/// <param name="fileNames"></param>
	/// <returns></returns>
	[[nodiscard]] RenderSet* const Get(const MeshLoadFileNames& fileNames);

	void SetIsNowThreading(bool isNowThreading);
public:
	std::pair<size_t, const std::list<RenderData*>&> CreateRenderList(BlendType blend);

private:
	void ResizeRenderList();

	[[nodiscard]] Shader LoadShader(const ShaderFileNames& shaderName);
	[[nodiscard]] MeshShader LoadMeshShader(const MeshShaderFileNames& shaderName);

	[[nodiscard]] std::array<Pipeline*, BlendType::kNum> CreateGraphicsPipelines(Shader shader, uint32_t numRenderTarget = 1);
	[[nodiscard]] std::array<Pipeline*, BlendType::kNum> CreateSkinAnimationGraphicsPipelines(Shader shader, uint32_t numRenderTarget = 1);
	[[nodiscard]] std::array<Pipeline*, BlendType::kNum> CreateMeshShaderGraphicsPipelines(MeshShader shader, uint32_t numRenderTarget = 1);

	[[nodiscard]] Pipeline* CreateShadowPipeline();
	[[nodiscard]] Pipeline* CreateSkinAnimationShadowPipeline();
	[[nodiscard]] Pipeline* CreateMeshShaderShadowPipeline();

private:
	std::unordered_map<Key, std::unique_ptr<RenderSet>> renderData_;
	std::unordered_map<Key, std::unique_ptr<RenderSet>> threadRenderData_;
	std::unordered_map<MeshKey, std::unique_ptr<RenderSet>> meshRenderData_;
	std::unordered_map<MeshKey, std::unique_ptr<RenderSet>> threadMeshRenderData_;
	bool isNowThreading_ = false;

	std::array<std::list<RenderData*>, BlendType::kNum + 1> renderDataLists_;
};