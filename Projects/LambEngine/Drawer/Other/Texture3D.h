/// =========================
/// ==  Texture3Dクラスの宣言  ==
/// =========================

#pragma once
#include "../BaseDrawer.h"

/// <summary>
/// 3Dモデルの描画
/// </summary>
class Texture3D : public BaseDrawer {
public:
	static constexpr uint32_t kMaxDrawCount = 1024;

public:
	struct ShaderData {
		int32_t isLighting = 1;
		int32_t textureIndex = 0;
		int32_t isEffect = 0;
		int32_t backGroundTextureIndex = 0;
		struct GaussianState {
			float32_t2 dir;
			float32_t sigma = 0.0f;
			int32_t kernelSize = 0;
		} gausState;
	};

	struct Data {
		Mat4x4 worldMatrix = Mat4x4::kIdentity;
		Mat4x4 camera = Mat4x4::kIdentity;
		uint32_t color = 0xffffffff;
		BlendType blend = BlendType::kNone;
		ShaderData shaderData;
	};
	struct Instance {
		QuaternionTransform transform;
		uint32_t color = 0xffffffff;
		ShaderData shaderData;
	};

public:
	Texture3D() = default;
	Texture3D(const std::string& fileName);
	Texture3D(const Texture3D&) = default;
	Texture3D(Texture3D&& right) noexcept = default;
	virtual ~Texture3D() = default;

	Texture3D& operator=(const Texture3D& right) = default;
	Texture3D& operator=(Texture3D&& right) noexcept = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	virtual void Load(const std::string& fileName);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="camera">カメラ行列</param>
	/// <param name="color">色</param>
	/// <param name="blend">ブレンドタイプ</param>
	/// <param name="isLighting">ライティングするか(ブレンドタイプがkNoneだったらDeferredRenderingの設定依存)</param>
	virtual void Draw(
		const Mat4x4& worldMatrix,
		const Mat4x4& camera,
		uint32_t color,
		BlendType blend,
		ShaderData shaderData = {}
	);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="data">データ</param>
	void Draw(const Data& data);

public:
	/// <summary>
	/// ノード
	/// </summary>
	/// <returns></returns>
	const Node& GetNode() const;
	
	/// <summary>
	/// モデルのデータ
	/// </summary>
	/// <returns></returns>
	const ModelData& GetModelData() const;

private:
	int32_t backGroundTextureIndex_ = 0;
};