/// ===========================
/// ==  DsvHeapクラスの定義  ==
/// ===========================


#include "DsvHeap.h"
#include "Engine/Graphics/DepthBuffer/DepthBuffer.h"
#include "Utils/ExecutionLog.h"
#include <algorithm>
#include "Error/Error.h"

Lamb::SafePtr<DsvHeap> DsvHeap::pInstance_ = nullptr;

DsvHeap::~DsvHeap()
{
	Lamb::AddLog("Finalize DsvHeap succeeded");
}

void DsvHeap::Initialize(UINT heapSize) {
	pInstance_.reset(new DsvHeap( heapSize ));
}

void DsvHeap::Finalize() {
	pInstance_.reset();
}

DsvHeap* const DsvHeap::GetInstance() {
	return pInstance_.get();
}

DsvHeap::DsvHeap(uint32_t heapSize) :
	DescriptorHeap{}
{
	CreateDescriptorHeap(heapSize);

	CreateHeapHandles();

	bookingHandle_.clear();

	Lamb::AddLog("Initialize DsvHeap succeeded : heap size is " + std::to_string(heapSize_));
}

void DsvHeap::CreateDescriptorHeap(uint32_t heapSize) {
	heapSize_ = std::clamp(heapSize, DirectXSwapChain::kBackBufferNumber, 0xffu);
	pHeap_ = DirectXDevice::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, heapSize_, false);
	pHeap_.SetName<DsvHeap>();
}

void DsvHeap::CreateHeapHandles() {
	uint64_t incrementDSVHeap = static_cast<uint64_t>(DirectXDevice::GetInstance()->GetIncrementDSVHeap());

	heapHandles_.resize(heapSize_, { pHeap_->GetCPUDescriptorHandleForHeapStart(), D3D12_GPU_DESCRIPTOR_HANDLE{} });
	for (uint64_t i = 0; i < static_cast<uint64_t>(heapSize_); i++) {
		heapHandles_[i].first.ptr += incrementDSVHeap * i;
	}
}

uint32_t DsvHeap::CreateView(DepthBuffer& depthStencilBuffer) {
	if (currentHandleIndex_ >= heapSize_) {
		throw Lamb::Error::Code<DsvHeap>("Over HeapSize", ErrorPlace);
	}

	if (bookingHandle_.empty()) {
		useHandle_.push_back(currentHandleIndex_);
		depthStencilBuffer.CreateDepthView(heapHandles_[currentHandleIndex_].first, currentHandleIndex_);
		currentHandleIndex_++;
		return currentHandleIndex_ - 1u;
	}
	else {
		uint32_t nowCreateViewHandle = bookingHandle_.front();
		useHandle_.push_back(nowCreateViewHandle);
		depthStencilBuffer.CreateDepthView(heapHandles_[nowCreateViewHandle].first, nowCreateViewHandle);
		bookingHandle_.pop_front();
		return nowCreateViewHandle;
	}
}