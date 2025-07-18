/// ====================================
/// ==  RenderingManagerクラスの宣言  ==
/// ====================================



#pragma once
#include "Utils/SafePtr.h"
#include "Drawer/PeraRender/PeraRender.h"
#include "Engine/Graphics/DepthBuffer/DepthBuffer.h"
#include "Engine/Graphics/PipelineObject/GaussianBlur/GaussianBlur.h"
#include "Engine/Graphics/PipelineObject/Outline/Outline.h"
#include "Engine/Graphics/RenderContextManager/RenderContext/RenderContext.h"

#include "Engine/Graphics/PipelineObject/DeferredRendering/DeferredRendering.h"
#include "Engine/Graphics/PipelineObject/ShadowRendering/ShadowRendering.h"

#include "Drawer/AirSkyBox/AirSkyBox.h"
#include "json.hpp"

#include "Engine/Graphics/Tonemap/Tonemap.h"

#include "Engine/Graphics/PipelineObject/PostWater/PostWater.h"

#include <list>
#include <functional>

class RenderingManager {
public:
	struct State {
		Vector3 cameraPos;
		Mat4x4 viewMatrix;
		Mat4x4 projectionMatrix;
		bool isLighting = false;
		bool isUseMeshShader = false;
		Vector3 lightRotate;
		Vector3 hsv;
		int32_t bloomHorizontalSize = 1;
		int32_t bloomVerticalSize = 1;
		float32_t luminanceThreshold = 1.0f;
		bool isDrawSkyBox = false;
		float32_t skyBoxEnvironmentCoefficient = 0.0f;
		Vector3 skyBoxScale = Vector3::kIdentity * 500.0f;
		bool isDrawOutline = false;
		float32_t outlineWeight = 0.0f;
	};

public:
	enum GaussianIndex {
		kHorizontal = 0,
		kVertical = 1,

		kNum
	};

private:
	using RenderDataList = std::pair<size_t, const std::list<RenderData*>&>;
	using RenderDataLists = std::vector<RenderDataList>;

public:
	RenderingManager();
	RenderingManager(const RenderingManager&) = delete;
	RenderingManager(RenderingManager&&) = delete;
	~RenderingManager();

public:
	RenderingManager& operator=(const RenderingManager&) = delete;
	RenderingManager& operator=(RenderingManager&&) = delete;

public:
	static void Initialize();
	static void Finalize();

	static const Lamb::SafePtr<RenderingManager> GetInstance();

private:
	static std::unique_ptr<RenderingManager> pInstance_;

public:
	void FrameStart();

	void FrameEnd();

	void FinalFrame();

private:
	void Draw();

	void CalcLightCamera();

	void CalcSunLight();

	void SetGraphicsState();

public:

	DepthBuffer* GetDepthBuffer();

/// <summary>
/// セッター
/// </summary>
public:
	void SetState(const State& state);

	void SetCameraPos(const Vector3& cameraPos);
	void SetWaterMatrix(const Mat4x4& waterMatrix);
	void SetViewMatrix(const Mat4x4& view);
	void SetProjectionMatrix(const Mat4x4& projection);
	void SetHsv(const Vector3& hsv);
	void SetColor(const Vector4& color);
	void SetIsLighting(bool isLighting);
	void SetLightRotate(const Vector3& lightRotate);
	void SetBloomKernelSize(int32_t x, int32_t y);
	void SetEnvironmentCoefficient(float32_t environmentCoefficient);
	void SetTonemapParam(float32_t2 toe, float32_t2 linear, float32_t2 shoulder);
	void SetIsUseMeshShader(bool isUseMesh);

	void Debug(const std::string& guiName);

public:
	void Save(nlohmann::json& jsonFile);
	void Load(nlohmann::json& jsonFile);

/// <summary>
/// ゲッター
/// </summary>
public:
	bool GetIsUseMeshShader() const;

	uint32_t GetBufferIndex()const;

	const AirSkyBox::AtmosphericParams& GetAtmosphericParams() const;

	const Vector3& GetCameraDirection() const;
	
	const Vector3& GetCameraPosition() const;

	UINT GetBackGroundTexture() const;


private:
	// アルファ値がないものを描画
	void DrawRGB(const RenderDataList& renderList);

	// cubemapの描画
	void DrawSkyBox();

	// アルファ値があるものを描画
	void DrawRGBA(const RenderDataLists& rgbaList);

	// 背景に処理を加えるものの描画
	void DrawAlphaEffect(const RenderDataLists& renderList);

	// ディファード描画
	void DrawDeferred();

	void DrawShadow(const RenderDataList& rgbList);

	// ポストエフェクトを描画する
	void DrawPostEffect();

	// 深度値を使わないものの描画
	void DrawNoDepth(const RenderDataLists& nodepthList);

private:

	// アルファ値があるものを順番を並び替える
	void ZSort(const RenderDataLists& rgbaList);

private:
	std::function<void(RenderDataList&)> resetDrawCount_;

private:
	// ディファードレンダリング用オフスクリーン
	std::unique_ptr<DeferredRendering> deferredRendering_;
	DeferredRendering::DeferredRenderingData deferredRenderingData_;

	std::unique_ptr<ShadowRendering> shadow_;

	// 法線書き込み用オフスクリーン
	std::unique_ptr<RenderTarget> normalTexture_;
	// 色書き込み用オフスクリーン
	std::unique_ptr<RenderTarget> colorTexture_;
	// ワールドポジション書き込み用オフスクリーン
	std::unique_ptr<RenderTarget> worldPositionTexture_;
	// 歪み書き込み用オフスクリーン
	std::unique_ptr<RenderTarget> distortionTexture_;

	// 歪み書き込み用オフスクリーン(アルファ値付きの歪みを書き込む)
	std::unique_ptr<RenderTarget> distortionTextureRGBA_;

	// ライティング後のrgbaテクスチャを描画
	std::unique_ptr<PeraRender> rgbaTexture_;
	Vector3 hsv_;

	// エフェクトを施したものの描画
	std::unique_ptr<PeraRender> effectTexture_;

	Lamb::SafePtr<PostWater> postWater_;
	float32_t4x4 waterWorldMatrx_;

	// 深度値(法線書き込みと色書き込み、アウトラインで使用する)
	std::unique_ptr<DepthBuffer> depthStencil_;
	// 影のための深度値書き込み
	std::unique_ptr<DepthBuffer> depthStencilShadow_;

	// ブルームで使用する輝度抽出用オフスクリーン
	std::unique_ptr<PeraRender> luminateTexture_;
	Lamb::SafePtr<class Luminate> luminate_;
	float32_t luminanceThreshold = 0.0f;

	// ブルームで使用するガウシアンフィルタ(横)用オフスクリーン
	std::unique_ptr<PeraRender> gaussianHorizontalTexture_;
	// ブルームで使用するガウシアンフィルタ(縦)用オフスクリーン
	std::unique_ptr<PeraRender> gaussianVerticalTexture_;

	GaussianBlur::State gaussianBlurStateHorizontal_;
	GaussianBlur::State gaussianBlurStateVertical_;

	// ガウシアンフィルタ用パイプラインオジェクト
	std::array<Lamb::SafePtr<GaussianBlur>, GaussianIndex::kNum> gaussianPipeline_;


	// アウトライン用オフスクリーン
	std::unique_ptr<PeraRender> outlineTexture_;
	// アウトライン用パイプラインオジェクト
	Lamb::SafePtr<Outline> outlinePipeline_;
	float32_t outlineWeight_ = 0.0f;
	bool isDrawOutLine_ = false;


	// skybox
	std::unique_ptr<AirSkyBox> skyBox_;
	QuaternionTransform skyBoxTransform_;
	AirSkyBox::AtmosphericParams atmosphericParams_;
	const Vector3 kLightRotateBaseVector = -Vector3::kZIdentity;
	Vector3 lightRotate_;
	bool isDrawSkyBox_ = false;
	bool isSkyTimeStart_ = false;
	float32_t sunSpeed_ = 0.0f;

	Mat4x4 viewMatrix_;
	Mat4x4 projectionMatrix_;
	Mat4x4 lightCamera_;
	Vector3 cameraDirection_;

	TonemapParams tonemapParamas_;
	Vector2 tonemapToe_ = Vector2(0.2f, 0.2f);
	Vector2 tonemapLinear_ = Vector2(0.6f, 0.6f);
	Vector2 tonemapShoulder_ = Vector2::kIdentity;

#ifdef USE_DEBUG_CODE
	std::vector<float32_t> toeDataX_;
	std::vector<float32_t> toeDataY_;
	std::array<float32_t, 2> linearDataX_ = {0.0f, 0.0f};
	std::array<float32_t, 2> linearDataY_ = {0.0f, 0.0f};
	std::vector<float32_t> sholderDataX_;
	std::vector<float32_t> sholderDataY_;

	std::vector<float32_t> whiteBorderX_;
	std::vector<float32_t> whiteBorderY_;
#endif // USE_DEBUG_CODE


	/// 
	/// その他ポストエフェクトは増える予定
	/// 

	// ラジアルブラー用オフスクリーン
	//std::unique_ptr<PeraRender>radialTexture_;

	// 色収差用オフスクリーン
	//std::unique_ptr<PeraRender>radialTexture_;


	bool isUseMesh_ = false;


	bool isFirstFrame_ = true;

	uint32_t bufferIndex_ = 0;
	uint32_t preBufferIndex_ = 0;



	/// Effekseer
	Lamb::SafePtr<class EffekseerControler> effekseerControler_;
};