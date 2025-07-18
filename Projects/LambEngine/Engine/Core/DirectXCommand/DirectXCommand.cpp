/// ==================================
/// ==  DirectXCommandクラスの定義  ==
/// ==================================

#include "DirectXCommand.h"
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Utils/ExecutionLog.h"
#include <cassert>
#include <algorithm>
#include "Error/Error.h"
#include "Engine/Engine.h"
#include "Utils/SafePtr.h"
#include "Utils/EngineInfo.h"


DirectXCommand::DirectXCommand():
	commandQueue_{},
	commandAllocators_{},
	commandLists_{},
	isCommandListClose_{false},
	fence_{},
	fenceVal_{0llu},
	fenceEvent_{nullptr}
{
	CreateCommandQueue_();

	CreateCommandAllocator_();

	CreateGraphicsCommandList_();

	CrateFence_();

	Lamb::AddLog("Initialize DirectXCommand succeeded");
}

DirectXCommand::~DirectXCommand() {
	CloseHandle(fenceEvent_);

	Lamb::AddLog("Finalize DirectXCommand succeeded");
}

DirectXCommand* const DirectXCommand::GetMainCommandlist()
{
	return Engine::GetInstance()->GetMainCommandlist();
}

void DirectXCommand::CloseCommandlist() {
	// コマンドリストを確定させる
	HRESULT hr = commandLists_[bufferIndex_]->Close();
	isCommandListClose_ = true;
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXCommand>("commandList_->Close() failed", ErrorPlace);
	}
}

void DirectXCommand::ExecuteCommandList() {
	ID3D12CommandList* commandLists[] = { commandLists_[bufferIndex_].Get() };
	commandQueue_->ExecuteCommandLists(_countof(commandLists), commandLists);
}

void DirectXCommand::ResetCommandlist() {
	// 次フレーム用のコマンドリストを準備
	HRESULT hr = commandAllocators_[bufferIndex_]->Reset();
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXCommand>("commandAllocator_->Reset() faield", ErrorPlace);
	}
	hr = commandLists_[bufferIndex_]->Reset(commandAllocators_[bufferIndex_].Get(), nullptr);
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXCommand>("commandList_->Reset() faield", ErrorPlace);
	}
	isCommandListClose_ = false;
}

void DirectXCommand::WaitForFinishCommnadlist() {
	// Fenceの値を更新
	fenceVal_++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue_->Signal(fence_.Get(), fenceVal_);

	// Fenceの値が指定したSigna値にたどり着いているか確認する
	// GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceVal_) {
		// 指定したSignal値にたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
		// イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

void DirectXCommand::CreateCommandQueue_() {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	
	// コマンドキューを作成
	commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue_.GetAddressOf()));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXCommand>("device somethig error", ErrorPlace);
	}
	else {
		Lamb::AddLog(std::string{__func__} + " succeeded");
	}

	commandQueue_.SetName<DirectXCommand>();
}

void DirectXCommand::CreateCommandAllocator_() {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();

	commandAllocators_ = { nullptr };

	// コマンドアロケータを生成する
	std::for_each(
		commandAllocators_.begin(),
		commandAllocators_.end(),
		[&device](auto& n) {
			HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(n.GetAddressOf()));
			if (!SUCCEEDED(hr)) {
				throw Lamb::Error::Code<DirectXCommand>("device somethig error", ErrorPlace);
			}
			else {
				Lamb::AddLog(std::string{ __func__ } + " succeeded");
			}
			n.SetName<DirectXCommand>();
		}
	);
}

void DirectXCommand::CreateGraphicsCommandList_() {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	
	for (size_t i = 0; i < commandLists_.size(); ++i) {
		// コマンドリストを作成する
		commandLists_[i] = nullptr;
		HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators_[i].Get(), nullptr, IID_PPV_ARGS(commandLists_[i].GetAddressOf()));
		if (!SUCCEEDED(hr)) {
			throw Lamb::Error::Code<DirectXCommand>("device somethig error", ErrorPlace);
		}
		else {
			Lamb::AddLog(std::string{ __func__ } + " succeeded");
		}
		commandLists_[i].SetName<DirectXCommand>();
	}
	
}

void DirectXCommand::CrateFence_() {
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	
	// 初期値0でFenceを作る
	fence_ = nullptr;
	fenceVal_ = 0llu;
	HRESULT hr = device->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.GetAddressOf()));
	fence_.SetName<DirectXCommand>();
	
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXCommand>("device somethig error", ErrorPlace);
	}

	// FenceのSignalを持つためのイベントを作成する
	fenceEvent_ = nullptr;
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!(fenceEvent_ != nullptr)) {
		throw Lamb::Error::Code<DirectXCommand>("device somethig error", ErrorPlace);
	}

	Lamb::AddLog(std::string{ __func__ } + " succeeded");
}

ID3D12GraphicsCommandList6* const DirectXCommand::GetCommandList() const
{
	return commandLists_[bufferIndex_].Get();
}

void DirectXCommand::SetBufferIndex(uint32_t bufferIndex)
{
	bufferIndex_ = std::clamp(bufferIndex, 0u, DirectXSwapChain::kBackBufferNumber - 1u);
}

void DirectXCommand::Barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subResource) {
	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース
	barrier.Transition.pResource = resource;
	// subResourceの設定
	barrier.Transition.Subresource = subResource;
	// 遷移前(現在)のResouceState
	barrier.Transition.StateBefore = before;
	// 遷移後のResouceState
	barrier.Transition.StateAfter = after;
	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
}

void DirectXCommand::BarrierUAV(ID3D12Resource* resource)
{
	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース
	barrier.Transition.pResource = resource;
	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
}

void DirectXCommand::Barrier(std::initializer_list<ID3D12Resource*> resources, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subResource) {
	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// subResourceの設定
	barrier.Transition.Subresource = subResource;
	// 遷移前(現在)のResouceState
	barrier.Transition.StateBefore = before;
	// 遷移後のResouceState
	barrier.Transition.StateAfter = after;

	std::vector<D3D12_RESOURCE_BARRIER> barriers = {};

	barriers.reserve(resources.size());
	for (auto& resource : resources) {
		barriers.push_back(barrier);
		barriers.back().Transition.pResource = resource;
	}

	// TransitionBarrierを張る
	commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
}

void DirectXCommand::Barrier(ID3D12Resource** resources, uint32_t numResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subResource) {
	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// subResourceの設定
	barrier.Transition.Subresource = subResource;
	// 遷移前(現在)のResouceState
	barrier.Transition.StateBefore = before;
	// 遷移後のResouceState
	barrier.Transition.StateAfter = after;

	std::vector<D3D12_RESOURCE_BARRIER> barriers = {};

	barriers.resize(numResource);
	for (size_t count = 0; auto & i : barriers) {
		i = barrier;
		i.Transition.pResource = resources[count];
		count++;
	}

	// TransitionBarrierを張る
	commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
}

void DirectXCommand::Barrier(std::vector<ID3D12Resource*> resources, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, UINT subResource)
{
	Lamb::SafePtr commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// subResourceの設定
	barrier.Transition.Subresource = subResource;
	// 遷移前(現在)のResouceState
	barrier.Transition.StateBefore = before;
	// 遷移後のResouceState
	barrier.Transition.StateAfter = after;

	std::vector<D3D12_RESOURCE_BARRIER> barriers = {};

	barriers.resize(resources.size());
	for (size_t count = 0; auto & i : barriers) {
		i = barrier;
		i.Transition.pResource = resources[count];
		count++;
	}

	// TransitionBarrierを張る
	commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
}
