/// ==============================
/// ==  CloudTex2Dクラスの宣言  ==
/// ==============================

#pragma once
#include "Drawer/BaseDrawer.h"
#include "Engine/Graphics/RenderContextManager/RenderContext/RenderContext.h"

/// <summary>
/// 雲の描画
/// </summary>
class CloudTex2D : public BaseDrawer {
public:
	struct Data {
		Mat4x4 worldMatrix = Mat4x4::kIdentity;
		Mat4x4 uvTransform = Mat4x4::kIdentity;
		Mat4x4 camera = Mat4x4::kIdentity;
		uint32_t textureID = 0;
		float32_t3 vec;
		uint32_t color = 0xffffffff;
		BlendType blend = BlendType::kNone;
	};
	struct Instance {
		QuaternionTransform transform;
		uint32_t textureID = 0;
		uint32_t color = 0xffffffff;
	};
	struct ShaderData {
		Mat4x4 uvTransform;
		float32_t3 randomVec;
		uint32_t textureID = 0u;
	};

public:
	static constexpr uint32_t kMaxDrawCount = 1024u;

private:
	using CloudTex2DRenderContext = RenderContext<ShaderData, kMaxDrawCount>;

	static const LoadFileNames kFileNames_;
	static const MeshLoadFileNames kMeshFileNames_;

public:
	CloudTex2D();
	CloudTex2D(const CloudTex2D&) = default;
	CloudTex2D(CloudTex2D&&) = default;
	~CloudTex2D() = default;

	CloudTex2D& operator=(const CloudTex2D&) = default;
	CloudTex2D& operator=(CloudTex2D&&) = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	void Load();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="uvTransform">uv行列</param>
	/// <param name="camera">カメラ行列</param>
	/// <param name="textureID">テクスチャID</param>
	/// <param name="randomVec">ランダムベクトル</param>
	/// <param name="color">色</param>
	/// <param name="blend">ブレンド</param>
	void Draw(
		const Mat4x4& worldMatrix,
		const Mat4x4& uvTransform,
		const Mat4x4& camera,
		uint32_t textureID,
		float32_t3 randomVec,
		uint32_t color,
		BlendType blend
	);
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="data">データ</param>
	void Draw(const CloudTex2D::Data& data);
};