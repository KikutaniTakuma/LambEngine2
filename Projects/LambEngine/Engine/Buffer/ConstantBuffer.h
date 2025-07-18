/// ==================================
/// ==  ConstantBufferクラスの定義  ==
/// ==================================

#pragma once
#include "Engine/Core/DescriptorHeap/Descriptor.h"
#include "ShaderBuffer.h"


/// <summary>
/// コンスタントバッファ
/// </summary>
/// <typeparam name="ValueType">ポインタと参照型以外をサポート</typeparam>
template<Lamb::IsNotReferenceAndPtr ValueType>
class ConstantBuffer final : public Descriptor, public ShaderBuffer<ValueType> {
public:
	using value_type = ValueType;

	using reference_type = value_type&;
	using const_reference_type = const value_type&;

	using pointer = value_type*;
	using const_pointer = const value_type*;

public:
	inline ConstantBuffer() {
		this->bufferSize_ = 1u;

		// バイトサイズは256アライメントする(vramを効率的に使うための仕組み)
		this->bufferResource_ = DirectXDevice::GetInstance()->CreateBufferResuorce((sizeof(ValueType) + 0xff) & ~0xff);
#ifdef USE_DEBUG_CODE
		this->bufferResource_.SetName<ConstantBuffer>();
#endif // USE_DEBUG_CODE
	}

	~ConstantBuffer() = default;

// コピーやムーブはしないので削除
private:
	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer(ConstantBuffer&&) noexcept = delete;

	ConstantBuffer& operator=(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(ConstantBuffer&&) = delete;

public:
	/// <summary>
	/// view作成
	/// </summary>
	/// <param name="heapHandleCPU"></param>
	/// <param name="heapHandleGPU"></param>
	/// <param name="heapHandle"></param>
	void CreateView(
		D3D12_CPU_DESCRIPTOR_HANDLE heapHandleCPU,
		D3D12_GPU_DESCRIPTOR_HANDLE heapHandleGPU,
		UINT heapHandle
	) noexcept override
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC  cbvDesc;
		cbvDesc.BufferLocation = this->bufferResource_->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = UINT(this->bufferResource_->GetDesc().Width);

		Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
		device->CreateConstantBufferView(&cbvDesc, heapHandleCPU);
		this->heapHandleCPU_ = heapHandleCPU;
		this->heapHandleGPU_ = heapHandleGPU;
		this->heapHandle_ = heapHandle;
		this->isCreateView_ = true;
	}
};