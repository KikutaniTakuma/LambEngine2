/// ==============================
/// ==  Descripterクラスの定義  ==
/// ==============================

#pragma once
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")

/// <summary>
/// ディスクリプター
/// </summary>
class Descriptor {
public:
	Descriptor() = default;
	virtual ~Descriptor() = default;

public:
	/// <summary>
	/// view作成の純粋仮想関数
	/// </summary>
	/// <param name="heapHandleCPU"></param>
	/// <param name="heapHandleGPU"></param>
	/// <param name="heapHandle"></param>
	virtual void CreateView(
		D3D12_CPU_DESCRIPTOR_HANDLE heapHandleCPU, 
		D3D12_GPU_DESCRIPTOR_HANDLE heapHandleGPU, 
		UINT heapHandle) = 0;

public:
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetHandleCPU() const {
		return heapHandleCPU_;
	}
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetHandleGPU() const {
		return heapHandleGPU_;
	}
	UINT GetHandleUINT() const {
		return heapHandle_;
	}


protected:
	D3D12_CPU_DESCRIPTOR_HANDLE heapHandleCPU_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE heapHandleGPU_ = {};
	UINT heapHandle_ = 0u;

	bool isCreateView_ = false;
};