/// ===========================
/// ==  RtvHeapクラスの定義  ==
/// ===========================


#include "RtvHeap.h"
#include "Engine/Graphics/RenderTarget/RenderTarget.h"
#include "Utils/ExecutionLog.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Math/Vector4.h"
#include <algorithm>
#include <cassert>
#include "Error/Error.h"

#include "Utils/SafeDelete.h"

Lamb::SafePtr<RtvHeap> RtvHeap::pInstance_ = nullptr;

RtvHeap::~RtvHeap()
{
	Lamb::AddLog("Finalize RtvHeap succeeded");
}

void RtvHeap::Initialize(UINT heapSize) {
	pInstance_.reset(new RtvHeap(heapSize));
}

void RtvHeap::Finalize() {
	pInstance_.reset();
}

RtvHeap* const RtvHeap::GetInstance() {
	return pInstance_.get();
}

RtvHeap::RtvHeap(uint32_t heapSize):
	DescriptorHeap{}
{
	CreateDescriptorHeap(heapSize);

	CreateHeapHandles();

	bookingHandle_.clear();

	Lamb::AddLog("Initialize RtvHeap succeeded : heap size is " + std::to_string(heapSize_));
}

void RtvHeap::CreateDescriptorHeap(uint32_t heapSize) {
	heapSize_ = std::clamp(heapSize, DirectXSwapChain::kBackBufferNumber, 0xffu);
	pHeap_ = DirectXDevice::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, heapSize_, false);
	pHeap_.SetName<RtvHeap>();
}

void RtvHeap::CreateHeapHandles() {
	uint64_t incrementRTVHeap = static_cast<uint64_t>(DirectXDevice::GetInstance()->GetIncrementRTVHeap());

	heapHandles_.resize(heapSize_, { pHeap_->GetCPUDescriptorHandleForHeapStart(), D3D12_GPU_DESCRIPTOR_HANDLE{} });
	for (uint64_t i = 0; i < static_cast<uint64_t>(heapSize_); i++) {
		heapHandles_[i].first.ptr += incrementRTVHeap * i;
	}
}

void RtvHeap::CreateBackBuffer(
	std::array<Lamb::LambPtr<ID3D12Resource>, DirectXSwapChain::kBackBufferNumber>& backBuffer,
	IDXGISwapChain4* const swapChain
) {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (uint32_t i = 0u; i < DirectXSwapChain::kBackBufferNumber; i++) {
		HRESULT hr = swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer[i].GetAddressOf()));

		if (!SUCCEEDED(hr)) {
			throw Lamb::Error::Code<RtvHeap>("GetBuffer() Failed", ErrorPlace);
		}

		device->CreateRenderTargetView(backBuffer[i].Get(), &rtvDesc, heapHandles_[i].first);

		useHandle_.push_back(i);
		currentHandleIndex_++;
	}

	Lamb::AddLog(std::string{ __func__ } + " succeeded");
}

void RtvHeap::SetMainRtv(const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle) {
	IDXGISwapChain4* const swapChain = DirectXSwapChain::GetInstance()->GetSwapChain();
	ID3D12GraphicsCommandList* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	commandList->OMSetRenderTargets(1, &heapHandles_[backBufferIndex].first, false, depthHandle);
}

void RtvHeap::SetRtv(uint32_t heapHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle) {
	SetRtv(&heapHandles_[heapHandle].first, 1, depthHandle);
}
void RtvHeap::SetRtv(std::initializer_list<D3D12_CPU_DESCRIPTOR_HANDLE> heapHandles, const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle) {
	ID3D12GraphicsCommandList* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	
	assert(0llu <= heapHandles.size() || heapHandles.size() <= 8llu);
	if (heapHandles.size() == 0u) {
		commandList->OMSetRenderTargets(0u, nullptr, false, depthHandle);
		return;
	}

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
	handles.resize(heapHandles.size());
	std::copy(heapHandles.begin(), heapHandles.end(), handles.begin());


	commandList->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), handles.data(), false, depthHandle);
}

void RtvHeap::SetRtv(D3D12_CPU_DESCRIPTOR_HANDLE* heapHandles, uint32_t numRenderTargets, const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle)
{
	ID3D12GraphicsCommandList* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	assert(0u <= numRenderTargets and numRenderTargets <= 8u);
	if (numRenderTargets == 0u) {
		commandList->OMSetRenderTargets(numRenderTargets, nullptr, false, depthHandle);
		return;
	}

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
	handles.resize(numRenderTargets);
	for (uint32_t i = 0; i < numRenderTargets; i++) {
		handles[i].ptr = heapHandles[i].ptr;
	}

	// 描画先をRTVを設定する
	commandList->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), handles.data(), false, depthHandle);
}

void RtvHeap::SetRtvAndMain(D3D12_CPU_DESCRIPTOR_HANDLE* heapHandles, uint32_t numRenderTargets, const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle)
{
	assert(0u <= numRenderTargets and numRenderTargets <= 7u);
	IDXGISwapChain4* const swapChain = DirectXSwapChain::GetInstance()->GetSwapChain();
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
	handles.resize(numRenderTargets + 1);
	handles.front().ptr = heapHandles_[backBufferIndex].first.ptr;
	for (size_t i = 0; i < handles.size() - 1; i++) {
		handles[i + 1].ptr = heapHandles[i].ptr;
	}

	ID3D12GraphicsCommandList* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	commandList->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), handles.data(), false, depthHandle);
}

void RtvHeap::ClearRenderTargetView(uint32_t handle, const Vector4& clearColor) {
	ID3D12GraphicsCommandList* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	commandList->ClearRenderTargetView(heapHandles_[handle].first, clearColor.m.data(), 0, nullptr);
}

uint32_t RtvHeap::CreateView(class RenderTarget& peraRender) {
	if (currentHandleIndex_ >= heapSize_) {
		throw Lamb::Error::Code<RtvHeap>("Over HeapSize", ErrorPlace);
	}

	if (bookingHandle_.empty()) {
		useHandle_.push_back(currentHandleIndex_);
		peraRender.CreateRTV(heapHandles_[currentHandleIndex_].first, currentHandleIndex_);
		currentHandleIndex_++;
		return currentHandleIndex_ - 1u;
	}
	else {
		uint32_t nowCreateViewHandle = bookingHandle_.front();
		useHandle_.push_back(nowCreateViewHandle);
		peraRender.CreateRTV(heapHandles_[nowCreateViewHandle].first, nowCreateViewHandle);
		bookingHandle_.pop_front();
		return nowCreateViewHandle;
	}
}
