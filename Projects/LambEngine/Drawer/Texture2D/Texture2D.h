/// =============================
/// ==  Texture2Dクラスの宣言  ==
/// =============================

#pragma once
#include "../BaseDrawer.h"
#include "Engine/Graphics/RenderContextManager/RenderContext/RenderContext.h"

/// <summary>
/// 板ポリ描画
/// </summary>
class Texture2D : public BaseDrawer {
public:
	struct Data {
		Mat4x4 worldMatrix = Mat4x4::kIdentity;
		Mat4x4 uvTransform = Mat4x4::kIdentity;
		Mat4x4 camera = Mat4x4::kIdentity;
		uint32_t textureID = 0;
		uint32_t color = 0xffffffff;
		BlendType blend = BlendType::kNone;
	};
	struct Instance {
		QuaternionTransform transform;
		uint32_t textureID = 0;
		uint32_t color = 0xffffffff;
	};
	struct ShaderData{
		Mat4x4 uvTransform;
		Vector3 pad; // <- huh?
		uint32_t textureID = 0u;
	};

public:
	static constexpr uint32_t kMaxDrawCount = 1024u;

private:
	using Texture2DRenderContext = RenderContext<ShaderData, kMaxDrawCount>;

	static const LoadFileNames kFileNames_;
	static const MeshLoadFileNames kMeshFileNames_;

public:
	Texture2D();
	Texture2D(const Texture2D&) = default;
	Texture2D(Texture2D&&) = default;
	~Texture2D() = default;

	Texture2D& operator=(const Texture2D&) = default;
	Texture2D& operator=(Texture2D&&) = default;

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
	/// <param name="color">色</param>
	/// <param name="blend">ブレンド</param>
	void Draw(
		const Mat4x4& worldMatrix, 
		const Mat4x4& uvTransform, 
		const Mat4x4& camera, 
		uint32_t textureID, 
		uint32_t color, 
		BlendType blend
	);
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="data">データ</param>
	void Draw(const Texture2D::Data& data);
};

/// <summary>
/// 板ポリインスタンス
/// </summary>
class Tex2DInstance {
public:
	Tex2DInstance() = default;
	~Tex2DInstance() = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">テクスチャファイルパス</param>
	void Load(const std::string& fileName);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="cameraMat">カメラ行列</param>
	void Draw(const Mat4x4& cameraMat);

	/// <summary>
	/// テクスチャのサイズ
	/// </summary>
	/// <returns></returns>
	Vector2 GetTexSize() const {
		if (pTex_.have()) {
			return pTex_->getSize();
		}
		else {
			return Vector2::kZero;
		}
	}

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="otherPos">ポジション</param>
	/// <returns></returns>
	bool Collision(const Vector3& otherPos);

public:
	Vector3 scale = Vector3::kIdentity;
	Vector3 rotate;
	Vector3 pos;

	Vector2 uvPibot;
	Vector2 uvSize = Vector2::kIdentity;

	uint32_t color = 0xffffffff;
	BlendType blend = BlendType::kNone;

private:
	Lamb::SafePtr<Texture> pTex_;

private:
	Lamb::SafePtr<Texture2D> pTex2D_;
};