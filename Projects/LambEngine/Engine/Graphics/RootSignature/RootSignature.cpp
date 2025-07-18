/// =================================
/// ==  RootSignatureクラスの定義  ==
/// =================================



#include "RootSignature.h"
#include <cassert>
#include "Utils/ExecutionLog.h"
#include "Engine/Engine.h"
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Error/Error.h"

RootSignature::RootSignature():
	rootSignature_{},
	desc_()
{
	rootParamater_ = {};
}
RootSignature::RootSignature(const RootSignature& right) {
	*this = right;
}
RootSignature::RootSignature(RootSignature&& right) noexcept {
	*this = std::move(right);
}
RootSignature& RootSignature::operator=(const RootSignature& right) {
	rootSignature_ = right.rootSignature_;
	rootParamater_ = right.rootParamater_;

	return *this;
}
RootSignature& RootSignature::operator=(RootSignature&& right) noexcept {
	rootSignature_ = std::move(right.rootSignature_);
	rootParamater_ = std::move(right.rootParamater_);

	return *this;
}

bool RootSignature::operator==(const RootSignature& right) const {
	if (rootParamater_.size() != right.rootParamater_.size()) {
		return false;
	}
	for (size_t i = 0; i < rootParamater_.size(); i++) {
		if (rootParamater_[i].first != right.rootParamater_[i].first) {
			return false;
		}
	}
	return  desc_ == right.desc_;
}
bool RootSignature::operator!=(const RootSignature& right) const {
	return !(*this == right);
}

void RootSignature::Create(const Desc& desc) {
	desc_ = desc;

	// RootSignatureの生成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = desc_.flag;

	rootParamater_.clear();
	rootParamater_.resize(desc_.rootParameterSize);
	for (size_t i = 0; i < desc_.rootParameterSize; i++) {
		std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
		if (desc_.rootParameter[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			ranges.resize(desc_.rootParameter[i].DescriptorTable.NumDescriptorRanges, D3D12_DESCRIPTOR_RANGE{});

			for (uint32_t j = 0; j < desc_.rootParameter[i].DescriptorTable.NumDescriptorRanges;j++) {
				ranges[j] = desc_.rootParameter[i].DescriptorTable.pDescriptorRanges[j];
			}
		}
		rootParamater_[i] = { desc_.rootParameter[i], std::move(ranges) };
	}

	std::vector<D3D12_ROOT_PARAMETER> params = {};
	params.reserve(rootParamater_.size());
	for (auto& i : rootParamater_) {
		if (i.first.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			i.first.DescriptorTable.pDescriptorRanges = i.second.data();
			i.first.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(i.second.size());
		}
		params.push_back(i.first);
	}

	descriptionRootSignature.pParameters = params.data();
	descriptionRootSignature.NumParameters = static_cast<UINT>(params.size());

	bool isTexture = not desc_.samplerDeacs.empty();

	descriptionRootSignature.pStaticSamplers = isTexture ? desc_.samplerDeacs.data() : nullptr;
	descriptionRootSignature.NumStaticSamplers = UINT(isTexture ? desc_.samplerDeacs.size() : 0llu);

	// シリアライズしてバイナリにする
	Lamb::LambPtr<ID3DBlob> signatureBlob;
	Lamb::LambPtr<ID3DBlob> errorBlob;
	HRESULT  hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, signatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr)) {
		throw Lamb::Error::Code<RootSignature>(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), ErrorPlace);
	}
	// バイナリをもとに生成
	if (rootSignature_) {
		rootSignature_.Reset();
	}
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<RootSignature>("CreateRootSignature failed", ErrorPlace);
	}
}

D3D12_STATIC_SAMPLER_DESC CreateLinearSampler(uint32_t shaderRegister)
{
	D3D12_STATIC_SAMPLER_DESC staticSampler{};

	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = shaderRegister;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return staticSampler;
}

D3D12_STATIC_SAMPLER_DESC CreateBorderLinearSampler(uint32_t shaderRegister)
{
	D3D12_STATIC_SAMPLER_DESC staticSampler{};

	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = shaderRegister;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return staticSampler;
}

D3D12_STATIC_SAMPLER_DESC CreatePointSampler(uint32_t shaderRegister)
{
	D3D12_STATIC_SAMPLER_DESC staticSampler{};

	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = shaderRegister;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return staticSampler;
}

D3D12_STATIC_SAMPLER_DESC CreateBorderPointSampler(uint32_t shaderRegister)
{
	D3D12_STATIC_SAMPLER_DESC staticSampler{};

	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = shaderRegister;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return staticSampler;
}

bool operator==(const D3D12_STATIC_SAMPLER_DESC& left, const D3D12_STATIC_SAMPLER_DESC& right) {
	return left.Filter == right.Filter
		and left.AddressU == right.AddressU
		and left.AddressV == right.AddressV
		and left.AddressW == right.AddressW
		and left.ComparisonFunc == right.ComparisonFunc
		and left.MaxLOD == right.MaxLOD
		and left.MinLOD == right.MaxLOD
		and left.ShaderRegister == right.ShaderRegister
		and left.RegisterSpace== right.RegisterSpace
		and left.ShaderVisibility == right.ShaderVisibility
		and left.BorderColor == right.BorderColor
		and left.MaxAnisotropy == right.MaxAnisotropy
		and left.MipLODBias == right.MipLODBias
		;
}

bool operator==(const D3D12_ROOT_PARAMETER& left, const D3D12_ROOT_PARAMETER& right) {
	bool isSameDescriptorTable = false;
	if (left.ParameterType != right.ParameterType || 
		left.ShaderVisibility != right.ShaderVisibility) {
		return false;
	}
	else if(left.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE){
		if (left.DescriptorTable.NumDescriptorRanges == right.DescriptorTable.NumDescriptorRanges) {
			for (uint32_t i = 0; i < left.DescriptorTable.NumDescriptorRanges; i++) {
				isSameDescriptorTable =
					left.DescriptorTable.pDescriptorRanges[i].BaseShaderRegister == right.DescriptorTable.pDescriptorRanges[i].BaseShaderRegister
					&& left.DescriptorTable.pDescriptorRanges[i].NumDescriptors == right.DescriptorTable.pDescriptorRanges[i].NumDescriptors
					&& left.DescriptorTable.pDescriptorRanges[i].OffsetInDescriptorsFromTableStart == right.DescriptorTable.pDescriptorRanges[i].OffsetInDescriptorsFromTableStart
					&& left.DescriptorTable.pDescriptorRanges[i].RangeType == right.DescriptorTable.pDescriptorRanges[i].RangeType
					&& left.DescriptorTable.pDescriptorRanges[i].RegisterSpace == right.DescriptorTable.pDescriptorRanges[i].RegisterSpace;

				if (!isSameDescriptorTable) {
					return false;
				}
			}
		}
		else {
			return false;
		}
	}
	else if (left.ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
		if (left.Descriptor.RegisterSpace == right.Descriptor.RegisterSpace&&
			left.Descriptor.ShaderRegister == right.Descriptor.ShaderRegister
			) {

			return true;
		}
	}
	else {
		return false;
	}

	return true;
}

bool operator!=(const D3D12_ROOT_PARAMETER& left, const D3D12_ROOT_PARAMETER& right) {
	return !(left == right);
}

bool RootSignature::IsSame(const Desc& desc) const {
	return desc_ == desc;
}

bool RootSignature::Desc::operator==(const Desc& right) const
{
	if (this->rootParameterSize != right.rootParameterSize) {
		return false;
	}
	if (this->flag != right.flag) {
		return false;
	}
	for (size_t i = 0; i < this->rootParameterSize; i++) {
		if (rootParameter[i] != right.rootParameter[i]) {
			return false;
		}
	}
	return samplerDeacs == right.samplerDeacs;
}
