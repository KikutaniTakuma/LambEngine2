/// ================================
/// ==  ShaderBufferクラスの定義  ==
/// ================================


#pragma once
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Engine/Core/DirectXSwapChain/DirectXSwapChain.h"

#include "Utils/Concepts.h"
#include "Error/Error.h"

#include <cassert>

template<Lamb::IsNotReferenceAndPtr ValueType>
class ShaderBuffer {
public:
	using value_type = ValueType;

	using reference_type = value_type&;
	using const_reference_type = const value_type&;

	using pointer = value_type*;
	using const_pointer = const value_type*;

public:
	ShaderBuffer() = default;
	virtual ~ShaderBuffer() = default;

	// コピーやムーブはしないので削除
private:
	ShaderBuffer(const ShaderBuffer&) = delete;
	ShaderBuffer(ShaderBuffer&&) = delete;

	ShaderBuffer& operator=(const ShaderBuffer&) = delete;
	ShaderBuffer& operator=(ShaderBuffer&&) = delete;


public:
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVtlAdrs() const noexcept {
		return this->bufferResource_->GetGPUVirtualAddress();
	}

	uint32_t size() const noexcept {
		return this->bufferSize_;
	}

public:
	/// <summary>
	/// マップ
	/// </summary>
	void Map() noexcept {
		if (not this->isMap) {
			this->bufferResource_->Map(0, nullptr, this->pData_.GetPtrAdress());
			this->isMap = true;
		}
	}

	/// <summary>
	/// アンマップ
	/// </summary>
	void Unmap() noexcept {
		if (this->isMap) {
			this->bufferResource_->Unmap(0, nullptr);
			this->isMap = false;
		}
	}

	/// <summary>
	/// データのコピー
	/// </summary>
	/// <param name="pSrc">コピー元</param>
	/// <param name="size">コピーサイズ</param>
	void MemCpy(const void* pSrc, size_t size) {
		Map();
		std::memcpy(this->pData_.get(), pSrc, size);
		Unmap();
	}

	/// <summary>
	/// 最初のデータへコピー
	/// </summary>
	/// <param name="pSrc">コピー元</param>
	void MemCpy(const void* pSrc) {
		Map();
		std::memcpy(this->pData_.get(), pSrc, sizeof(value_type));
		Unmap();
	}

	template<Lamb::IsInt IsInt>
	reference_type operator[](IsInt index) {
		if (this->bufferSize_ <= static_cast<uint32_t>(index) or not this->isMap) [[unlikely]] {
#ifdef USE_DEBUG_CODE
			assert(!"Out of array references or did not Map");
#else
			throw Lamb::Error::Code<ShaderBuffer>("Out of array references or did not Map", ErrorPlace);
#endif // USE_DEBUG_CODE
		}

		return this->pData_[index];
	}

	template<Lamb::IsInt IsInt>
	const_reference_type operator[](IsInt index) const {
		if (this->bufferSize_ <= static_cast<uint32_t>(index) or not this->isMap) [[unlikely]] {
#ifdef USE_DEBUG_CODE
			assert(!"Out of array references or did not Map");
#else
			throw Lamb::Error::Code<ShaderBuffer>("Out of array references or did not Map", ErrorPlace);
#endif // USE_DEBUG_CODE
		}
		return this->pData_[index];
	}

	reference_type operator*() {
		if (not this->isMap) [[unlikely]] {
#ifdef USE_DEBUG_CODE
			assert(!"Did not Map");
#else
			throw Lamb::Error::Code<ShaderBuffer>("Did not Map", ErrorPlace);
#endif // USE_DEBUG_CODE
		}
		return *(this->pData_);
	}

	const_reference_type operator*() const {
		if (not this->isMap) [[unlikely]] {
#ifdef USE_DEBUG_CODE
			assert(!"Did not Map");
#else
			throw Lamb::Error::Code<ShaderBuffer>("Did not Map", ErrorPlace);
#endif // USE_DEBUG_CODE
		}
		return *(this->pData_);
	}

	pointer operator->() {
		if (not this->isMap) [[unlikely]] {
#ifdef USE_DEBUG_CODE
			assert(!"Did not Map");
#else
			throw Lamb::Error::Code<ShaderBuffer>("Did not Map", ErrorPlace);
#endif // USE_DEBUG_CODE
		}
		return (this->pData_.get());
	}

	const_pointer operator->() const {
		if (not this->isMap) [[unlikely]] {
#ifdef USE_DEBUG_CODE
			assert(!"Did not Map");
#else
			throw Lamb::Error::Code<ShaderBuffer>("Did not Map", ErrorPlace);
#endif // USE_DEBUG_CODE
		}
		return (this->pData_.get());
	}

protected:
	Lamb::LambPtr<ID3D12Resource> bufferResource_;
	Lamb::SafePtr<value_type> pData_ = nullptr;
	uint32_t bufferSize_ = 0u;
	bool isMap = false;
};