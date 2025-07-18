/// =================================
/// ==  RootSignatureクラスの宣言  ==
/// =================================


#pragma once
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <vector>

#include "Engine/EngineUtils/LambPtr/LambPtr.h"

/// <summary>
/// ルートシグネチャを管理
/// </summary>
class RootSignature {
public:
	struct Desc
	{
		D3D12_ROOT_PARAMETER* rootParameter;
		size_t rootParameterSize;
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDeacs;
		D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		bool operator==(const Desc& right) const;
	};

public:
	RootSignature();
	~RootSignature() = default;

	RootSignature(const RootSignature& right);
	RootSignature(RootSignature&& right) noexcept;

public:
	RootSignature& operator=(const RootSignature& right);
	RootSignature& operator=(RootSignature&& right) noexcept;

	bool operator==(const RootSignature& right) const;
	bool operator!=(const RootSignature& right) const;

public:
	/// <summary>
	/// ルートシグネチャ作成
	/// </summary>
	/// <param name="desc"></param>
	void Create(const Desc& desc);

	inline ID3D12RootSignature* Get() const {
		return rootSignature_.Get();
	}

	bool IsSame(const Desc& desc) const;

private:
	Lamb::LambPtr<ID3D12RootSignature> rootSignature_;
	std::vector<std::pair<D3D12_ROOT_PARAMETER, std::vector<D3D12_DESCRIPTOR_RANGE>>> rootParamater_;

	Desc desc_;
};

D3D12_STATIC_SAMPLER_DESC CreateLinearSampler(uint32_t shaderRegister = 0);
D3D12_STATIC_SAMPLER_DESC CreateBorderLinearSampler(uint32_t shaderRegister = 0);
D3D12_STATIC_SAMPLER_DESC CreatePointSampler(uint32_t shaderRegister = 0);
D3D12_STATIC_SAMPLER_DESC CreateBorderPointSampler(uint32_t shaderRegister = 0);

bool operator==(const D3D12_STATIC_SAMPLER_DESC& left, const D3D12_STATIC_SAMPLER_DESC& right);
bool operator==(const D3D12_ROOT_PARAMETER& left, const D3D12_ROOT_PARAMETER& right);
bool operator!=(const D3D12_ROOT_PARAMETER& left, const D3D12_ROOT_PARAMETER& right);