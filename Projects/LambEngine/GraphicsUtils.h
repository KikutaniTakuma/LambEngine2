#pragma once

#include <string>

#include "Engine/Graphics/GraphicsStructs.h"


// 描画関連
namespace Lamb {
	struct ModelShaderData {
		int32_t isLighting = 1;
		int32_t isEffect = 0;
		struct GaussianState {
			float32_t2 dir;
			float sigma = 0.0f;
			int32_t kernelSize = 0;
		} gausState;
	};



	// テクスチャID
	using TextureID = uint32_t;
	// モデルID
	using ModelID = size_t;

	/// <summary>
	/// テクスチャ読み込み
	/// </summary>
	/// <param name="fileName">テクスチャファイルパス</param>
	/// <returns>テクスチャID</returns>
	TextureID LoadTexture(const std::string& fileName);

	/// <summary>
	/// モデル読み込み
	/// </summary>
	/// <param name="fileName">モデル読み込み</param>
	/// <returns>モデルID</returns>
	ModelID LoadModel(const std::string& fileName);

	/// <summary>
	/// テクスチャ描画
	/// </summary>
	/// <param name="textureID">テクスチャID</param>
	/// <param name="worldMatrix">ワールドマトリックス</param>
	/// <param name="uvTransform">uvマトリックス</param>
	/// <param name="camera">カメラマトリックス</param>
	/// <param name="color">色(RGBA)</param>
	/// <param name="blend">ブレンドタイプ</param>
	void DrawTexture(
		TextureID textureID,
		const Mat4x4& worldMatrix,
		const Mat4x4& uvTransform,
		const Mat4x4& camera,
		uint32_t color,
		BlendType blend
	);

	/// <summary>
	/// モデル描画
	/// </summary>
	/// <param name="modelID">モデルID</param>
	/// <param name="worldMatrix">ワールドマトリックス</param>
	/// <param name="camera">カメラマトリックス</param>
	/// <param name="color">色(RGBA)</param>
	/// <param name="blend">ブレンドタイプ</param>
	/// <param name="shaderData">shaderData</param>
	void DrawModel(
		ModelID modelID,
		const Mat4x4& worldMatrix,
		const Mat4x4& camera,
		uint32_t color,
		BlendType blend,
		const ModelShaderData& shaderData = {}
	);


	const SafePtr<const Descriptor> GetTextureDescriptor(const std::string& fileName);

	Vector2 GetTexturePixelSize(const std::string& fileName);

	/// <summary>
	/// EffekseerのUpdate(1frameに1度のみ呼び出すこと)
	/// </summary>
	/// <param name="time">時間(s)</param>
	/// <param name="viewMatrix">ビューマトリックス</param>
	/// <param name="projectionMatrix">プロジェクションマトリックス</param>
	void EffekseerUpdate(float time, const Mat4x4& viewMatrix, const Mat4x4& projectionMatrix);

	/// <summary>
	/// Effetの更新頻度を設定
	/// </summary>
	/// <param name="freq">更新頻度(2.0fの場合、2.0frameで1回更新される)</param>
	void EffekseerSetUpdateInterval(float freq);
}