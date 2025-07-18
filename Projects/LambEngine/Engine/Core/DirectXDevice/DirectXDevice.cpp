/// =================================
/// ==  DirectXDeviceクラスの定義  ==
/// =================================

#include "DirectXDevice.h"
#include "Utils/ExecutionLog.h"
#include "Utils/ConvertString.h"
#include "Math/Vector2.h"

#include <cassert>
#include <format>
#include <array>

#include "Error/Error.h"
#include "Utils/SafeDelete.h"

Lamb::SafePtr<DirectXDevice> DirectXDevice::pInstance_ = nullptr;

DirectXDevice::~DirectXDevice() {
	Lamb::AddLog("Finalize DirectXDevice succeeded");
}

DirectXDevice* const DirectXDevice::GetInstance() {
	return pInstance_.get();
}

void DirectXDevice::Initialize() {
	pInstance_.reset(new DirectXDevice());
}
void DirectXDevice::Finalize() {
	pInstance_.reset();
}


DirectXDevice::DirectXDevice():
	incrementSRVCBVUAVHeap_(0u),
	incrementRTVHeap_(0u),
	incrementDSVHeap_(0u),
	incrementSAMPLER_(0u),
	device_{},
	dxgiFactory_{},
	useAdapter_{}
{
	// IDXGIFactory生成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory_.GetAddressOf()));
	assert(SUCCEEDED(hr));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXDevice>("somthing error", "CreateDXGIFactory", __FILE__, __LINE__);
	}

	// 使用するグラボの設定
	SettingAdapter_();
	if (useAdapter_ == nullptr) {
		throw Lamb::Error::Code<DirectXDevice>("GPU not Found", "SettingAdapter", __FILE__, __LINE__);
	}

	// Deviceの初期化
	// 使用しているデバイスによってD3D_FEATURE_LEVELの対応バージョンが違うので成功するまでバージョンを変えて繰り返す
	CreateDevice_();

#ifdef USE_DEBUG_CODE
	InfoQueue_();
#endif

	CreateHeapIncrements_();

	Lamb::AddLog("Initialize DirectXDevice succeeded");
}

void DirectXDevice::SettingAdapter_() {
	useAdapter_ = nullptr;
	for (UINT i = 0;
		dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(useAdapter_.GetAddressOf())) != DXGI_ERROR_NOT_FOUND;
		++i
		) 
	{

		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = useAdapter_->GetDesc3(&adapterDesc);
		if (hr != S_OK) {
			throw Lamb::Error::Code<DirectXDevice>("GetDesc3() Failed", ErrorPlace);
		}

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Lamb::AddLog(ConvertString(std::format(L"Use Adapter : {}", adapterDesc.Description)));
			break;
		}
		useAdapter_.Reset();
	}
}

void DirectXDevice::CreateDevice_() {
	// Deviceの初期化
	// 使用しているデバイスによってD3D_FEATURE_LEVELの対応バージョンが違うので成功するまでバージョンを変えて繰り返す
	std::array featureLevels = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};
	std::array featureLevelString = {
		"12.2", "12.1", "12.0"
	};

	for (size_t i = 0; i < featureLevels.size(); ++i) {
		HRESULT hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(device_.GetAddressOf()));

		if (SUCCEEDED(hr)) {
			Lamb::AddLog(std::format("FeatureLevel : {}", featureLevelString[i]));
			break;
		}
	}

	if (device_ == nullptr) {
		throw Lamb::Error::Code<DirectXDevice>("device not found", ErrorPlace);
	}

	// shader modelの確認
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
	HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
	if (FAILED(hr) or (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5)) {
		Lamb::AddLog("This device does not support Shader Model 6.5");
		isCanUseMeshShader_ = false;
	}
	else {
		Lamb::AddLog("This device support Shader Model 6.5");
		isCanUseMeshShader_ = true;
	}

	// mesh shaderが使えるか
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
	hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features));
	if (FAILED(hr) or (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)) {
		Lamb::AddLog("This device does not support Mesh Shader");
		isCanUseMeshShader_ = false;
	}
	else {
		Lamb::AddLog("This device support Mesh Shader");
		isCanUseMeshShader_ = true;
	}

	if (not isCanUseMeshShader_) {
		Lamb::AddLog("Notice!! This device cannot use ""Mesh Shader""");
	}
	Lamb::AddLog("Complete create D3D12Device");
}

#ifdef USE_DEBUG_CODE
void DirectXDevice::InfoQueue_() const {
	Lamb::LambPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(infoQueue.GetAddressOf())))) {
		// やばいエラーの予期に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		// 解放
		infoQueue.Reset();
	}
}
#endif // USE_DEBUG_CODE

void DirectXDevice::CreateHeapIncrements_() {
	incrementSRVCBVUAVHeap_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	incrementRTVHeap_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	incrementDSVHeap_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	incrementSAMPLER_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

[[nodiscard]]
ID3D12DescriptorHeap* DirectXDevice::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderrVisible
) {
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderrVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (!SUCCEEDED(device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)))) {
		throw Lamb::Error::Code<DirectXDevice>("somthing error", ErrorPlace);
	}

	return descriptorHeap;
}

[[nodiscard]]
ID3D12Resource* DirectXDevice::CreateBufferResuorce(size_t sizeInBytes) {
	if (!device_) {
		OutputDebugStringA("device is nullptr!!");
		throw Lamb::Error::Code<DirectXDevice>("device is nullptr", ErrorPlace);
	}

	// Resourceを生成する
	// リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProp{};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	// リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定にする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	// バッファの場合はこれにする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際にリソースを作る
	ID3D12Resource* resource = nullptr;
	resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(&uploadHeapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<DirectXDevice>("somthing error", ErrorPlace);
	}

	return resource;
}

[[nodiscard]]
ID3D12Resource* DirectXDevice::CreateDepthStencilTextureResource(int32_t width, int32_t height) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ID3D12Resource* resource = nullptr;
	if (!SUCCEEDED(
		device_->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthClearValue,
			IID_PPV_ARGS(&resource))
	)) {
		throw Lamb::Error::Code<DirectXDevice>("somthing error", ErrorPlace);
	}

	return resource;
}

[[nodiscard]]
ID3D12Resource* DirectXDevice::CreateDepthStencilTextureResource(const Vector2& size) {
	return CreateDepthStencilTextureResource(
		static_cast<int32_t>(size.x),
		static_cast<int32_t>(size.y)
	);
}
