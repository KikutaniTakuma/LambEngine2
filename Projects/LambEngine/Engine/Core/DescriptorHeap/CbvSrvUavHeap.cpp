/// =================================
/// ==  CbvSrvUavHeapクラスの定義  ==
/// =================================

#include "CbvSrvUavHeap.h"
#include "Utils/ConvertString.h"
#include "Engine/Core/WindowFactory/WindowFactory.h"
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Graphics/DepthBuffer/DepthBuffer.h"
#include "Engine/Graphics/TextureManager/Texture/Texture.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "Utils/SafeDelete.h"

#include "Engine/Core/DescriptorHeap/Descriptor.h"

Lamb::SafePtr<CbvSrvUavHeap> CbvSrvUavHeap::pInstance_ = nullptr;

CbvSrvUavHeap::~CbvSrvUavHeap() {
	Lamb::AddLog("Finalize CbvSrvUavHeap succeeded");
}

void CbvSrvUavHeap::Initialize(UINT heapSize, UINT maxTexture) {
	pInstance_.reset(new CbvSrvUavHeap(heapSize, maxTexture));
}

void CbvSrvUavHeap::Finalize() {
	pInstance_.reset();
}

CbvSrvUavHeap* const CbvSrvUavHeap::GetInstance() {
	return pInstance_.get();
}

CbvSrvUavHeap::CbvSrvUavHeap(UINT numDescriptor, UINT maxTexture) :
	DescriptorHeap{},
	currentTextureHeapIndex_{0u},
	releaseTextureHeapIndex_{},
	kMaxTextureHadle_{maxTexture}
{
	CreateDescriptorHeap(numDescriptor);

	CreateHeapHandles();

	bookingHandle_.clear();

	currentHandleIndex_ += kMaxTextureHadle_;

	Lamb::AddLog("Initialize CbvSrvUavHeap succeeded : heap size is " + std::to_string(heapSize_) + " : Max Texture heap range is " + std::to_string(kMaxTextureHadle_));
}

void CbvSrvUavHeap::CreateDescriptorHeap(uint32_t heapSize) {
	// 1～(10^6-1)でクランプ
	heapSize_ = std::clamp(heapSize, 1u, static_cast<UINT>(std::pow(10u, 6u)) - 1u);
	pHeap_ = DirectXDevice::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, heapSize_, true);
	pHeap_.SetName<CbvSrvUavHeap>();
}

void CbvSrvUavHeap::CreateHeapHandles()
{
	uint64_t incrementCbvSrvUavHeap = static_cast<uint64_t>(DirectXDevice::GetInstance()->GetIncrementSRVCBVUAVHeap());

	heapHandles_.resize(heapSize_, { pHeap_->GetCPUDescriptorHandleForHeapStart(), pHeap_->GetGPUDescriptorHandleForHeapStart() });
	for (uint64_t i = 0; i < static_cast<uint64_t>(heapSize_); i++) {
		heapHandles_[i].first.ptr += incrementCbvSrvUavHeap * i;
		heapHandles_[i].second.ptr += incrementCbvSrvUavHeap * i;
	}
}

void CbvSrvUavHeap::Use(D3D12_GPU_DESCRIPTOR_HANDLE handle, UINT rootParmIndex) {
	static auto commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();
	commandlist->SetGraphicsRootDescriptorTable(rootParmIndex, handle);
}
void CbvSrvUavHeap::Use(uint32_t handleIndex, UINT rootParmIndex) {
	auto commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();
	commandlist->SetGraphicsRootDescriptorTable(rootParmIndex, heapHandles_[handleIndex].second);
}

uint32_t CbvSrvUavHeap::CreateView(Descriptor* buffer) {
	if (currentHandleIndex_ >= heapSize_) {
		throw Lamb::Error::Code<CbvSrvUavHeap>("Over Heap Size", ErrorPlace);
	}

	if (bookingHandle_.empty()) {
		useHandle_.push_back(currentHandleIndex_);
		buffer->CreateView(heapHandles_[currentHandleIndex_].first, heapHandles_[currentHandleIndex_].second, currentHandleIndex_);
		currentHandleIndex_++;
		return currentHandleIndex_ - 1u;
	}
	else {
		uint32_t nowCreateViewHandle = bookingHandle_.front();
		useHandle_.push_back(nowCreateViewHandle);
		buffer->CreateView(heapHandles_[nowCreateViewHandle].first, heapHandles_[nowCreateViewHandle].second, nowCreateViewHandle);
		bookingHandle_.pop_front();
		return nowCreateViewHandle;
	}
}

uint32_t CbvSrvUavHeap::CreateTextureView(Descriptor* tex)
{
	// viewを作れる上限か
	if (kMaxTextureHadle_ <= currentTextureHeapIndex_) {
		throw Lamb::Error::Code<CbvSrvUavHeap>("Over Heap Size", ErrorPlace);
	}


	if (releaseTextureHeapIndex_.empty()) {
		tex->CreateView(heapHandles_[currentTextureHeapIndex_].first, heapHandles_[currentTextureHeapIndex_].second, currentTextureHeapIndex_);
		currentTextureHeapIndex_++;
		return currentTextureHeapIndex_ - 1u;
	}
	else {
		uint32_t nowCreateViewHandle = releaseTextureHeapIndex_.front();
		tex->CreateView(heapHandles_[nowCreateViewHandle].first, heapHandles_[nowCreateViewHandle].second, nowCreateViewHandle);
		releaseTextureHeapIndex_.pop_front();
		return nowCreateViewHandle;
	}
}

void CbvSrvUavHeap::ReleaseView(Texture* tex)
{
	// すでにコンテナに追加してのか
	auto isExist = std::find(releaseTextureHeapIndex_.begin(), releaseTextureHeapIndex_.end(), tex->GetHandleUINT());

	// 追加してない
	if (isExist == releaseTextureHeapIndex_.end()) {
		releaseTextureHeapIndex_.push_back(tex->GetHandleUINT());
	}
}

void CbvSrvUavHeap::ReleaseView(UINT viewHandle)
{
	DescriptorHeap::ReleaseView(viewHandle);
}

uint32_t CbvSrvUavHeap::GetMaxTexture() const
{
	return kMaxTextureHadle_;
}
