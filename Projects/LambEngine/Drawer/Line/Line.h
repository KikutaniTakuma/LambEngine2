/// ========================
/// ==  Lineクラスの宣言  ==
/// ========================

#pragma once
#include "Math/Vector3.h"
#include "Math/Matrix.h"
#include "Math/Vector4.h"
#include "Engine/Buffer/StructuredBuffer.h"
#include "Engine/Graphics/Shader/ShaderManager/ShaderManager.h"

#include "Utils/SafePtr.h"

#include <memory>

/// <summary>
/// 線の描画
/// </summary>
class Line {
private:
	struct VertxData {
		Mat4x4 wvp;
		Vector4 color;
	};

private:
	static constexpr uint32_t kVertexNum = 2u;
	static constexpr uint32_t kDrawMaxNumber_ = 65535u;
	static uint32_t depthDrawCount_;
	static uint32_t nodepthDrawCount_;

public:
	/// <summary>
	/// バッファーの生成
	/// </summary>
	static void Initialize();

	/// <summary>
	/// バッファーの開放
	/// </summary>
	static void Finalize();

	/// <summary>
	/// 線の描画
	/// </summary>
	/// <param name="isDepth"></param>
	static void DrawAll(bool isDepth);

private:
	static Shader shader_;

	static Lamb::SafePtr<class Pipeline> pDepthPipeline_;
	static Lamb::SafePtr<class Pipeline> pNodepthPipeline_;

	static std::array<std::unique_ptr<StructuredBuffer<VertxData>>, DirectXSwapChain::kBackBufferNumber> depthVertData_;
	static std::array<std::unique_ptr<StructuredBuffer<VertxData>>, DirectXSwapChain::kBackBufferNumber> nodepthVertData_;

public:
	Line();
	Line(const Line& right) = default;
	Line(Line&& right) noexcept = default;
	~Line() = default;

	Line& operator=(const Line& right) = default;
	Line& operator=(Line&& right)noexcept = default;

public:
	/// <summary>
	/// imgui
	/// </summary>
	/// <param name="guiName"></param>
	void Debug(const std::string& guiName);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="viewProjection">ワールド行列</param>
	/// <param name="isDepth">深度値を有効にするか</param>
	void Draw(const Mat4x4& viewProjection, bool isDepth = true);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="start">開始位置</param>
	/// <param name="end">終わり位置</param>
	/// <param name="viewProjection">カメラ行列</param>
	/// <param name="color">色</param>
	/// <param name="isDepth">深度値</param>
	static void Draw(
		const Vector3& start, 
		const Vector3& end,
		const Mat4x4& viewProjection, 
		uint32_t color = 0xffffffff, 
		bool isDepth = true
	);

public:
	Vector3 start;
	Vector3 end;
	uint32_t color;
};