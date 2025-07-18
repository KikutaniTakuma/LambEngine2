/// ============================
/// ==  Pipelineクラスの宣言  ==
/// ============================


#pragma once
#include "Engine/Graphics/Shader/ShaderManager/ShaderManager.h"
#include <Utils/SafePtr.h>
#include <array>
class RootSignature;

/// <summary>
/// グラフィックスパイプライン
/// </summary>
class Pipeline {
public:
	/// <summary>
	/// フレンドクラス
	/// </summary>
	friend class PipelineManager;

/// <summary>
/// サブクラス
/// </summary>
public:
	enum Blend : uint16_t {
		None,
		Normal,
		Add,
		Sub,
		Mul,

		BlendTypeNum,
	};

	enum class CullMode {
		None = D3D12_CULL_MODE_NONE,
		Back = D3D12_CULL_MODE_BACK,
		Front = D3D12_CULL_MODE_FRONT,
	};
	enum class SolidState {
		Wireframe = D3D12_FILL_MODE_WIREFRAME,
		Solid = D3D12_FILL_MODE_SOLID,
	};

	struct VSInputData {
		std::string semanticName;
		uint32_t semanticIndex = 0;
		DXGI_FORMAT format;
		uint32_t inputSlot = 0;
		[[nodiscard]] bool operator==(const VSInputData& right) const = default;
	};

	struct Desc {
		Lamb::SafePtr<RootSignature> rootSignature;
		std::array<Pipeline::Blend, 8> blend = {};
		std::array<DXGI_FORMAT, 8> rtvFormtat = {};
		uint32_t numRenderTarget = 1;
		bool isDepth = true;
		Pipeline::CullMode cullMode = {};
		Pipeline::SolidState solidState = {};
		Shader shader;

		std::vector<VSInputData> vsInputData;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = {};

		[[nodiscard]] bool operator==(const Desc& right) const;
	};

	struct MeshDesc {
		// IAがない分いろいろいらん

		Lamb::SafePtr<RootSignature> rootSignature;
		std::array<Pipeline::Blend, 8> blend = {};
		std::array<DXGI_FORMAT, 8> rtvFormtat = {};
		uint32_t numRenderTarget = 1;
		bool isDepth = true;
		Pipeline::CullMode cullMode = {};
		Pipeline::SolidState solidState = {};
		MeshShader shader;

		[[nodiscard]] bool operator==(const MeshDesc& right) const;
	};

/// <summary>
/// コンストラクタ
/// </summary>
public:
	Pipeline();
	~Pipeline() = default;
	Pipeline(Pipeline&& right) noexcept;
	Pipeline(const Pipeline&) = delete;
/// <summary>
/// 演算子のオーバーロード
/// </summary>
public:
	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&& right) noexcept;

	bool operator==(const Pipeline& right) const;
	bool operator!=(const Pipeline& right) const;

/// <summary>
/// メンバ関数
/// </summary>
public:
	void AddVertexInput(
		const VSInputData& vsInpuptData
	);

	void Create(
		const Desc& desc
	);

	void CreateCubeMap(
		const Desc& desc
	);

	void Create(
		const MeshDesc& desc
	);

	void Use() const;

	bool IsSame(
		const Desc& desc
	);
	bool IsSame(
		const MeshDesc& desc
	);

	bool GetIsMesh() const;

/// <summary>
/// メンバ変数
/// </summary>
private:
	Lamb::LambPtr<ID3D12PipelineState> graphicsPipelineState_;

	std::vector<D3D12_INPUT_ELEMENT_DESC> vertexInput_;

	bool isMesh_ = false;
	
	std::unique_ptr<Desc> desc_;
	std::unique_ptr<MeshDesc> meshDesc_;
};