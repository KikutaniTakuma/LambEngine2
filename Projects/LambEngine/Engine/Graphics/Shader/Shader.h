#pragma once

/// <summary>
/// シェーダーをまとめる構造体
/// </summary>
struct Shader {
	struct IDxcBlob* vertex = nullptr;
	struct IDxcBlob* hull = nullptr;
	struct IDxcBlob* domain = nullptr;
	struct IDxcBlob* geometory = nullptr;
	struct IDxcBlob* pixel = nullptr;

	[[nodiscard]] bool operator==(const Shader& right) const = default;
};

/// <summary>
/// シェーダーをまとめる構造体
/// </summary>
struct MeshShader {
	struct IDxcBlob* amplification = nullptr; // 増幅シェーダー
	struct IDxcBlob* mesh = nullptr;          // メッシュシェーダー
	struct IDxcBlob* pixel = nullptr;         // ピクセルシェーダー

	[[nodiscard]] bool operator==(const MeshShader& right) const = default;
};