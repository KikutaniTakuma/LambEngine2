/// ====================================
/// ==  StructuredBufferクラスの定義  ==
/// ====================================

#pragma once
#include "Engine/Core/DescriptorHeap/Descriptor.h"
#include "ShaderBuffer.h"


/// <summary>
/// ストラクチャードバッファ
/// </summary>
/// <typeparam name="ValueType">ポインタと参照型以外をサポート</typeparam>
template<Lamb::IsNotReferenceAndPtr ValueType>
class StructuredBuffer final : public Descriptor, public ShaderBuffer<ValueType> {
public:
	using value_type = ValueType;

	using reference_type = value_type&;
	using const_reference_type = const value_type&;

	using pointer = value_type*;
	using const_pointer = const value_type*;


public:
	StructuredBuffer() = default;

	~StructuredBuffer() = default;

	// コピーやムーブはしないので削除
private:
	StructuredBuffer(const StructuredBuffer&) = delete;
	StructuredBuffer(StructuredBuffer&&) = delete;

	StructuredBuffer& operator=(const StructuredBuffer&) = delete;
	StructuredBuffer& operator=(StructuredBuffer&&) = delete;

/// <summary>
/// リソース系
/// </summary>
public:
	/// <summary>
	/// バッファー作成
	/// </summary>
	/// <param name="bufferSize">バッファサイズ</param>
	void CreateBuffer(uint32_t bufferSize) {
		this->bufferSize_ = bufferSize;

		this->bufferResource_ = DirectXDevice::GetInstance()->CreateBufferResuorce(sizeof(value_type) * this->size());
#ifdef USE_DEBUG_CODE
		this->bufferResource_.SetName<StructuredBuffer>();
#endif // USE_DEBUG_CODE
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVtlAdrs() const noexcept {
		return this->bufferResource_->GetGPUVirtualAddress();
	}

	/// <summary>
	/// view作成
	/// </summary>
	/// <param name="heapHandleCPU"></param>
	/// <param name="heapHandleGPU"></param>
	/// <param name="heapHandle"></param>
	void CreateView(
		D3D12_CPU_DESCRIPTOR_HANDLE heapHandleCPU,
		D3D12_GPU_DESCRIPTOR_HANDLE heapHandleGPU,
		UINT heapHandle) noexcept override
	{
		Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.NumElements = this->size();
		srvDesc.Buffer.StructureByteStride = sizeof(value_type);

		this->heapHandleCPU_ = heapHandleCPU;
		this->heapHandleGPU_ = heapHandleGPU;
		this->heapHandle_ = heapHandle;

		device->CreateShaderResourceView(this->bufferResource_.Get(), &srvDesc, heapHandleCPU_);

		isCreateView_ = true;
	}
};