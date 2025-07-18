/// ===========================
/// ==  DsvHeapクラスの宣言  ==
/// ===========================


#pragma once
#include "Engine/Core/DescriptorHeap/DescriptorHeap.h"
#include "Utils/SafePtr.h"

class DsvHeap final : public DescriptorHeap{
private:
	DsvHeap(uint32_t heapSize);

	DsvHeap() = delete;
	DsvHeap(const DsvHeap&) = delete;
	DsvHeap(DsvHeap&&) = delete;
	DsvHeap& operator=(const DsvHeap&) = delete;
	DsvHeap& operator=(DsvHeap&&) = delete;

public:
	~DsvHeap();

public:
	static void Initialize(UINT heapSize);

	static void Finalize();

	static DsvHeap* const GetInstance();

private:
	static Lamb::SafePtr<DsvHeap> pInstance_;

private:
	/// <summary>
	/// ヒープ作成
	/// </summary>
	/// <param name="heapSize">ヒープ作成</param>
	void CreateDescriptorHeap(uint32_t heapSize) override;

	/// <summary>
	/// ハンドル作成
	/// </summary>
	void CreateHeapHandles() override;

	[[deprecated("Don`t use this function")]]
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHeapHandle([[maybe_unused]] uint32_t heapIndex) const override {
		return D3D12_GPU_DESCRIPTOR_HANDLE{};
	}

public:
	/// <summary>
	/// ビュー作成
	/// </summary>
	/// <param name="depthStencilBuffer">深度値</param>
	/// <returns></returns>
	uint32_t CreateView(class DepthBuffer& depthStencilBuffer);
};