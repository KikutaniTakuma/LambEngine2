/// ======================================
/// ==  RWStructuredBufferクラスの定義  ==
/// ======================================

#pragma once

#include "Engine/Core/DescriptorHeap/Descriptor.h"
#include "ShaderBuffer.h"


template<Lamb::IsNotReferenceAndPtr ValueType>
class RWStructuredBuffer : public Descriptor, public ShaderBuffer<ValueType> {
public:
	using value_type = ValueType;

	using reference_type = value_type&;
	using const_reference_type = const value_type&;

	using pointer = value_type*;
	using const_pointer = const value_type*;

public:
	RWStructuredBuffer() = default;
	~RWStructuredBuffer() = default;

	// コピーやムーブはしないので削除
private:
	RWStructuredBuffer(const RWStructuredBuffer&) = delete;
	RWStructuredBuffer(RWStructuredBuffer&&) = delete;

	RWStructuredBuffer& operator=(const RWStructuredBuffer&) = delete;
	RWStructuredBuffer& operator=(RWStructuredBuffer&&) = delete;

public:
	/// <summary>
	/// バッファー作成
	/// </summary>
	/// <param name="bufferSize">バッファーサイズ</param>
	void CreateBuffer(uint32_t bufferSize) {
		this->bufferSize_ = bufferSize;

		this->bufferResource_ = DirectXDevice::GetInstance()->CreateBufferResuorce(sizeof(value_type) * this->size());
#ifdef USE_DEBUG_CODE
		this->bufferResource_.SetName<RWStructuredBuffer>();
#endif // USE_DEBUG_CODE
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
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = this->size();
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.StructureByteStride = sizeof(value_type);

		this->heapHandleCPU_ = heapHandleCPU;
		this->heapHandleGPU_ = heapHandleGPU;
		this->heapHandle_ = heapHandle;

		Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
		device->CreateUnorderedAccessView(this->bufferResource_.Get(), nullptr, &uavDesc, heapHandleCPU_);

		isCreateView_ = true;
	}
};