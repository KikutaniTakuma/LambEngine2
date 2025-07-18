/// ==========================
/// ==  Engineクラスの定義  ==
/// ==========================


#include "Engine.h"
#include <cassert>
#include <format>
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

#include <bit>

#include "Core/WindowFactory/WindowFactory.h"
#include "Core/DirectXDevice/DirectXDevice.h"
#include "Core/DirectXCommand/DirectXCommand.h"
#include "Core/DirectXSwapChain/DirectXSwapChain.h"
#include "Core/StringOutPutManager/StringOutPutManager.h"
#include "Core/ImGuiManager/ImGuiManager.h"
#include "Core/EffekseerControler/EffekseerControler.h"

#include "Core/DescriptorHeap/RtvHeap.h"
#include "Core/DescriptorHeap/CbvSrvUavHeap.h"
#include "Core/DescriptorHeap/DsvHeap.h"

#include "Engine/Graphics/RenderingManager/RenderingManager.h"
#include "Engine/Graphics/TextureManager/TextureManager.h"
#include "AudioManager/AudioManager.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"
#include "Engine/Graphics/AnimationManager/AnimationManager.h"
#include "Engine/Graphics/VertexIndexDataManager/VertexIndexDataManager.h"
#include "Engine/Graphics/Meshlet/Meshlet.h"
#include "Engine/Graphics/PipelineManager/PipelineManager.h"

#include "EngineUtils/FrameInfo/FrameInfo.h"
#include "EngineUtils/FlgManager/FlgManager.h"
#include "Utils/ConvertString.h"
#include "Utils/ExecutionLog.h"
#include "Engine/EngineUtils/ErrorCheck/ErrorCheck.h"

#include "Math/Vector2.h"

#include "Utils/ScreenOut.h"

#include "Error/Error.h"
#include "Utils/SafeDelete.h"

#include "Drawer/DrawerManager.h"

#include "Utils/EngineInfo.h"



#ifdef USE_DEBUG_CODE
Engine::Debug Engine::debugLayer_;

Engine::Debug::Debug() :
	debugController_(nullptr)
{}

Engine::Debug::~Debug() {
	debugController_.Reset();

	// リソースリークチェック
	Lamb::LambPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.GetAddressOf())))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
	}
}

///
/// デバッグレイヤー初期化
/// 

void Engine::Debug::InitializeDebugLayer() {
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController_.GetAddressOf())))) {
		// デバッグレイヤーを有効化する
		debugController_->EnableDebugLayer();
		// これは重いのでFALSE
		debugController_->SetEnableGPUBasedValidation(FALSE);

		Lamb::AddLog("InitializeDebugLayer succeeded");
	}
	else {
		throw Lamb::Error::Code<Engine::Debug>("D3D12GetDebugInterface failed", ErrorPlace);
	}
}

#endif // USE_DEBUG_CODE

/// 
/// 各種初期化処理
/// 

Engine* Engine::pInstance_ = nullptr;

void Engine::Initialize(const std::string& windowName, const Vector2& windowSize, float fpsLimit, bool isFullscreen) {
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<Engine>("CoInitializeEx failed", ErrorPlace);
	}
	else {
		Lamb::AddLog("CoInitializeEx succeeded");
	}
	pInstance_ = new Engine();
	assert(pInstance_);


	const auto&& windowTitle = ConvertString(windowName);

	FrameInfo::GetInstance()->SetFpsLimit(fpsLimit);
	Lamb::AddLog("Set fps limit : " + std::to_string(fpsLimit));

	// Window生成
	WindowFactory::GetInstance()->Create(windowTitle, static_cast<int32_t>(windowSize.x), static_cast<int32_t>(windowSize.y), isFullscreen);

#ifdef USE_DEBUG_CODE
	// DebugLayer有効化
	debugLayer_.InitializeDebugLayer();
#endif

	pInstance_->HardwareLog();

	// デバイス生成
	pInstance_->InitializeDirectXDevice();

	// ディスクリプタヒープ初期化
	RtvHeap::Initialize(128u);
	DsvHeap::Initialize(128u);
	CbvSrvUavHeap::Initialize(1024u);

	// コマンドリスト生成
	pInstance_->InitializeDirectXCommand();

	// スワップチェーン生成
	pInstance_->InitializeDirectXSwapChain();

	ImGuiManager::Initialize();

	pInstance_->InitializeDirectXTK();

	// 各種マネージャー初期化
	ShaderManager::Initialize();
	TextureManager::Initialize();
	AudioManager::Inititalize();
	PipelineManager::Initialize();
	VertexIndexDataManager::Initialize();
	MeshletManager::Initialize();
	RenderContextManager::Initialize();
	AnimationManager::Initialize();
	EffekseerControler::Initialize();


	DrawerManager::Initialize();
}

void Engine::Finalize() {
	pInstance_->isFinalize_ = true;

	// 各種マネージャー解放
	DrawerManager::Finalize();

	EffekseerControler::Finalize();
	AnimationManager::Finalize();
	RenderContextManager::Finalize();
	MeshletManager::Finalize();
	VertexIndexDataManager::Finalize();
	PipelineManager::Finalize();
	AudioManager::Finalize();
	TextureManager::Finalize();
	ShaderManager::Finalize();

	StringOutPutManager::Finalize();

	CbvSrvUavHeap::Finalize();
	DsvHeap::Finalize();
	RtvHeap::Finalize();

	ImGuiManager::Finalize();

	DirectXSwapChain::Finalize();
	pInstance_->FinalizeDirectXCommand();
	DirectXDevice::Finalize();

	delete pInstance_;
	pInstance_ = nullptr;

	// COM 終了
	CoUninitialize();
	Lamb::AddLog("CoUninitialize succeeded");

	ErrorCheck::GetInstance()->CrashProgram();
}

bool Engine::IsFinalize() {
	return pInstance_->isFinalize_;
}

std::string Engine::GetCpuName() const {
	// COMセキュリティの設定
	HRESULT hres = CoInitializeSecurity(
		nullptr,
		-1,
		nullptr,
		nullptr,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr,
		EOAC_NONE,
		nullptr
	);

	if (FAILED(hres)) {
		return std::string{};
	}

	// WMIセットアップ
	IWbemLocator* pLoc = nullptr;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		std::bit_cast<LPVOID*>(&pLoc)
	);

	if (FAILED(hres)) {
		return std::string{};
	}

	IWbemServices* pSvc = nullptr;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMv2"),
		nullptr,
		nullptr,
		0,
		LONG{},
		0,
		0,
		&pSvc
	);

	if (FAILED(hres)) {
		pLoc->Release();
		return std::string{};
	}

	// WMIクエリのセットアップ
	IEnumWbemClassObject* pEnumerator = nullptr;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_Processor"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&pEnumerator
	);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		return std::string{};
	}

	// WMIクエリの実行
	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;

	while (pEnumerator) {
		hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (uReturn == 0) {
			break;
		}

		VARIANT vtProp = {};

		// "Name"プロパティを取得
		hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		if (SUCCEEDED(hres)) {
			// CPU情報を取得
			std::wstring cpuInfo = vtProp.bstrVal;
			VariantClear(&vtProp);

			// WMIオブジェクトの解放
			pclsObj->Release();
			pSvc->Release();
			pLoc->Release();

			// std::wstringをstd::stringに変換して返す
			return ConvertString(cpuInfo);
		}
	}

	// WMIオブジェクトの解放
	pSvc->Release();
	pLoc->Release();

	return std::string{};
}

void Engine::HardwareLog() const {
	Lamb::AddLog("cpu : " + pInstance_->GetCpuName());

	MEMORYSTATUSEX memoryStatus;
	memoryStatus.dwLength = sizeof(memoryStatus);

	if (GlobalMemoryStatusEx(&memoryStatus)) {
		Lamb::AddLog(std::format("main memory : {} MB", memoryStatus.ullTotalPhys / (1024 * 1024)));
		Lamb::AddLog(std::format("main memory Use rate : {} %", memoryStatus.dwMemoryLoad));
	}
}



///
/// DirectXDevice
/// 

void Engine::InitializeDirectXDevice() {
	DirectXDevice::Initialize();
	directXDevice_ = DirectXDevice::GetInstance();
}


/// 
/// DirectXCommand
/// 

void Engine::InitializeDirectXCommand() {
	directXCommand_ = std::make_unique<DirectXCommand>();
}

void Engine::FinalizeDirectXCommand()
{
	directXCommand_.reset();
}

DirectXCommand* const Engine::GetMainCommandlist() const
{
	return directXCommand_.get();
}


/// 
/// DirectXSwapChain
///

void Engine::InitializeDirectXSwapChain() {
	DirectXSwapChain::Initialize();
	directXSwapChain_ = DirectXSwapChain::GetInstance();
}


///
///  DirectXTK
///

void Engine::InitializeDirectXTK() {
	StringOutPutManager::Initialize();
	stringOutPutManager_ = StringOutPutManager::GetInstance();
}




/// 
/// MianLoop用
/// 

void Engine::FrameStart() {
	const Lamb::SafePtr frameInfo = FrameInfo::GetInstance();
	frameInfo->Start();

	RenderingManager::GetInstance()->FrameStart();


	Lamb::screenout.Clear();
	Lamb::screenout << Lamb::endline;
}

void Engine::FrameEnd() {
	RenderingManager::GetInstance()->FrameEnd();

	// エラーチェック
	static auto err = ErrorCheck::GetInstance();
	if (err->GetError()) {
		return;
	}
	FlgManager::GetInstance()->AllFlgUpdate();

	const Lamb::SafePtr frameInfo = FrameInfo::GetInstance();
	frameInfo->DrawFps();
	Lamb::screenout.Draw();


	frameInfo->End();
}