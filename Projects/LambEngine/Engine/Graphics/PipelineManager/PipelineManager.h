/// ===================================
/// ==  PipelineManagerクラスの宣言  ==
/// ===================================



#pragma once
#include "Pipeline/Pipeline.h"
#include "../RootSignature/RootSignature.h"
#include "Utils/SafePtr.h"
#include <list>
#include <vector>
#include <memory>
#include <tuple>
class RootSignature;

/// <summary>
/// グラフィックスパイプラインとルートシグネチャの管理
/// </summary>
class PipelineManager {
/// <summary>
/// コンストラクタ
/// </summary>
private:
	PipelineManager();
	PipelineManager(const PipelineManager&) = delete;
	PipelineManager(PipelineManager&&) = delete;

	PipelineManager& operator=(const PipelineManager&) = delete;
	PipelineManager& operator=(PipelineManager&&) = delete;
public:
	~PipelineManager() = default;

/// <summary>
/// 静的関数
/// </summary>
public:
	static void Initialize();
	static void Finalize();

	static PipelineManager* const GetInstance();

	///
	/// 下の4つの関数を使った後にCreateする
	/// 
	
	/// <summary>
	/// ルートシグネチャを生成
	/// </summary>
	/// <param name="desc">設定</param>
	/// <param name="isTexture_">テクスチャを使う場合はtrue</param>
	[[nodiscard]] RootSignature* CreateRootSgnature(const RootSignature::Desc& desc);

	void SetDesc(const Pipeline::Desc& desc);
	void SetDesc(const Pipeline::MeshDesc& desc);

	/// <summary>
	/// 設定したものでPSOの生成
	/// </summary>
	/// <returns>psoのポインタ(勝手にdeleteしてはいけない)</returns>
	Pipeline* const Create();
	/// <summary>
	/// cubemapように最適化したパイプラインを作成する
	/// </summary>
	/// <returns></returns>
	Pipeline* const CreateCubeMap();

	/// <summary>
	/// MeshShaderを使ったパイプラインを作成する
	/// </summary>
	/// <returns></returns>
	Pipeline* const CreateMesh();

	/// <summary>
	/// Descをリセット
	/// </summary>
	void StateReset();

/// <summary>
/// シングルトンインスタンス
/// </summary>
private:
	static Lamb::SafePtr<PipelineManager> pInstance_;

/// <summary>
/// メンバ変数
/// </summary>
private:
	std::list<std::unique_ptr<Pipeline>> pipelines_;
	std::list<std::unique_ptr<RootSignature>> rootSignatures_;

	Pipeline::Desc pipelineDesc_;
	Pipeline::MeshDesc pipelineMeshDesc_;
};