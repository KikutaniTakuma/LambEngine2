/// ================================
/// ==  RenderTargerクラスの定義  ==
/// ================================



#include "RenderTarget.h"
#include "Engine/Engine.h"
#include "Engine/Core/WindowFactory/WindowFactory.h"
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Core/DirectXSwapChain/DirectXSwapChain.h"
#include "Utils/ConvertString.h"
#include "Utils/ExecutionLog.h"
#include <cassert>
#include "Math/Vector4.h"
#include "Engine/Core/DescriptorHeap/RtvHeap.h"
#include "Error/Error.h"

RenderTarget::RenderTarget() :
	RenderTarget(
		static_cast<uint32_t>(WindowFactory::GetInstance()->GetClientSize().x),
		static_cast<uint32_t>(WindowFactory::GetInstance()->GetClientSize().y)
	)
{
	
}

RenderTarget::RenderTarget(uint32_t width, uint32_t height) :
	resource_(),
	isWrightResouceState_(false),
	width_(width),
	height_(height),
	srvDesc_{},
	rtvHeapHandle_{},
	rtvHeapHandleUint_(0u)
{
	auto resDesc = DirectXSwapChain::GetInstance()->GetSwapchainBufferDesc();
	resDesc.Width = width_;
	resDesc.Height = height_;
	resDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;


	// Resourceを生成する
	// リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapPropaerties{};
	heapPropaerties.Type = D3D12_HEAP_TYPE_DEFAULT;
	Vector4 clsValue = { 0.0f, 0.0f, 0.0f, 0.0f };
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	for (size_t i = 0; i < clsValue.m.size(); i++) {
		clearValue.Color[i] = clsValue[i];
	} 

	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();

	// 実際にリソースを作る
	HRESULT hr = device->
		CreateCommittedResource(
			&heapPropaerties,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(resource_.GetAddressOf())
		);
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<RenderTarget>("CreateCommittedResource Function Failed", ErrorPlace);
	}

	Lamb::SafePtr rtvHeap = RtvHeap::GetInstance();
	rtvHeap->BookingHeapPos(1u);
	rtvHeap->CreateView(*this);

	srvDesc_ = {};
	srvDesc_.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc_.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc_.Texture2D.MipLevels = 1;
	srvDesc_.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
}

RenderTarget::~RenderTarget() {
	Lamb::SafePtr rtvHeap = RtvHeap::GetInstance();
	rtvHeap->ReleaseView(rtvHeapHandleUint_);
}

void RenderTarget::SetThisRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle) {
	ChangeResourceState();

	Lamb::SafePtr rtvHeap = RtvHeap::GetInstance();
	rtvHeap->SetRtv(rtvHeapHandleUint_, depthHandle);

	Vector4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	rtvHeap->ClearRenderTargetView(rtvHeapHandleUint_, clearColor);
}

void RenderTarget::ChangeResourceState() {
	DirectXCommand::Barrier(
		resource_.Get(),
		isWrightResouceState_ ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		isWrightResouceState_ ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	isWrightResouceState_ = not isWrightResouceState_;
}

void RenderTarget::SetMainRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle) {
	ChangeResourceState();
	
	RtvHeap::GetInstance()->SetMainRtv(depthHandle);
}

void RenderTarget::UseThisRenderTargetShaderResource() {
	auto mainComList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	mainComList->SetGraphicsRootDescriptorTable(0, heapHandleGPU_);
}

void RenderTarget::CreateView(
	D3D12_CPU_DESCRIPTOR_HANDLE heapHandleCPU,
	D3D12_GPU_DESCRIPTOR_HANDLE heapHandleGPU,
	UINT heapHandle
) {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	
	device->CreateShaderResourceView(
		resource_.Get(),
		&srvDesc_,
		heapHandleCPU
	);

	heapHandleCPU_ = heapHandleCPU;
	heapHandleGPU_ = heapHandleGPU;
	heapHandle_ = heapHandle;

	tex_.reset();
	tex_ = std::make_unique<Texture>();
	tex_->Set(
		resource_,
		srvDesc_,
		heapHandleGPU_,
		heapHandle_
	);
}

void RenderTarget::CreateRTV(D3D12_CPU_DESCRIPTOR_HANDLE descHeapHandle, UINT descHeapHandleUINT) {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(resource_.Get(), &rtvDesc, descHeapHandle);

	rtvHeapHandle_ = descHeapHandle;
	rtvHeapHandleUint_ = descHeapHandleUINT;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& RenderTarget::GetRtvHandleCPU() const
{
	return rtvHeapHandle_;
}

UINT RenderTarget::GetRtvHandleUINT() const
{
	return rtvHeapHandleUint_;
}

ID3D12Resource* const RenderTarget::GetResource() const
{
	return resource_.Get();
}

void RenderTarget::SetRenderTargets(
	Lamb::SafePtr<RenderTarget*> renderTargetPtrs, 
	uint32_t numRenderTarget, 
	const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle
) {
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
	handles.resize(numRenderTarget);
	for (uint32_t i = 0; i < numRenderTarget; i++) {
		handles[i] = renderTargetPtrs[i]->GetRtvHandleCPU();
	}

	Lamb::SafePtr rtvHeap = RtvHeap::GetInstance();
	rtvHeap->SetRtv(handles.data(), numRenderTarget, depthHandle);
}

void RenderTarget::SetMainAndRenderTargets(
	Lamb::SafePtr<RenderTarget*> renderTargetPtrs, 
	uint32_t numRenderTarget, 
	const D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle
) {
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles;
	handles.resize(numRenderTarget);
	for (uint32_t i = 0; i < numRenderTarget; i++) {
		handles[i] = renderTargetPtrs[i]->GetRtvHandleCPU();
	}

	Lamb::SafePtr rtvHeap = RtvHeap::GetInstance();
	rtvHeap->SetRtvAndMain(handles.data(), numRenderTarget, depthHandle);
}

void RenderTarget::ChangeResourceState(
	Lamb::SafePtr<RenderTarget*> renderTargetPtrs, 
	uint32_t numRenderTarget
) {
	for (uint32_t i = 0; i < numRenderTarget; i++) {
		renderTargetPtrs[i]->ChangeResourceState();
	}
}

void RenderTarget::ChangeToWriteResources(Lamb::SafePtr<RenderTarget*> renderTargetPtrs, uint32_t numRenderTarget) {
	std::vector<ID3D12Resource*> resources;
	resources.resize(numRenderTarget);
	for (uint32_t i = 0; i < numRenderTarget; i++) {
		resources[i] = renderTargetPtrs[i]->GetResource();
		renderTargetPtrs[i]->isWrightResouceState_ = true;
	}
	
	DirectXCommand::Barrier(
		resources,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
}

void RenderTarget::ChangeToTextureResources(Lamb::SafePtr<RenderTarget*> renderTargetPtrs, uint32_t numRenderTarget) {
	std::vector<ID3D12Resource*> resources;
	resources.resize(numRenderTarget);
	for (uint32_t i = 0; i < numRenderTarget; i++) {
		resources[i] = renderTargetPtrs[i]->GetResource();
		renderTargetPtrs[i]->isWrightResouceState_ = false;
	}

	DirectXCommand::Barrier(
		resources,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
}

void RenderTarget::Clear(
	Lamb::SafePtr<RenderTarget*> renderTargetPtrs, 
	uint32_t numRenderTarget
) {
	Lamb::SafePtr rtvHeap = RtvHeap::GetInstance();
	for (uint32_t i = 0; i < numRenderTarget; i++) {
		rtvHeap->ClearRenderTargetView(renderTargetPtrs[i]->rtvHeapHandleUint_, Vector4::kZero);
	}
}


