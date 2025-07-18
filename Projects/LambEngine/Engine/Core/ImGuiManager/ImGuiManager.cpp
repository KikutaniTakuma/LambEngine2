/// ================================
/// ==  ImGuiManagerクラスの定義  ==
/// ================================


#include "ImGuiManager.h"

#include <cassert>

#include "Engine/Core/WindowFactory/WindowFactory.h"
#include "Engine/Core/DirectXDevice/DirectXDevice.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Core/DirectXSwapChain/DirectXSwapChain.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "ImGuizmo.h"
#include "implot.h"

#include "Utils/SafeDelete.h"

Lamb::SafePtr<ImGuiManager> ImGuiManager::pInstance_ = nullptr;

ImGuiManager* const ImGuiManager::GetInstance() {
	return pInstance_.get();
}
void ImGuiManager::Initialize() {
	pInstance_.reset(new ImGuiManager());
}
void ImGuiManager::Finalize() {
	pInstance_.reset();
}

ImGuiManager::ImGuiManager() {
#ifdef USE_DEBUG_CODE
	Lamb::SafePtr device = DirectXDevice::GetInstance()->GetDevice();
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// SRV用のヒープ
	auto descriptorHeap = CbvSrvUavHeap::GetInstance();
	uint32_t useHandle = descriptorHeap->BookingHeapPos(1u);

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	// ImGuiIO
	auto& imguiIO = ImGui::GetIO();
	// Docking有効化
	imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// 日本語フォント追加
	imguiIO.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msgothic.ttc", 13.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(WindowFactory::GetInstance()->GetHwnd());
	ImGui_ImplDX12_Init(
		device.get(),
		DirectXSwapChain::kBackBufferNumber,
		rtvDesc.Format,
		descriptorHeap->Get(),
		descriptorHeap->GetCpuHeapHandle(useHandle),
		descriptorHeap->GetGpuHeapHandle(useHandle)
	);

	ImGuizmo::SetOrthographic(false);
	Vector2 size = WindowFactory::GetInstance()->GetClientSize();
	ImGuizmo::SetRect(
		0, 0, size.x, size.y
	);
	ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

	descriptorHeap->UseThisPosition(useHandle);

	Lamb::AddLog("Initialize ImGuiManager succeeded");
#endif // DEBUG
}
ImGuiManager::~ImGuiManager() {
#ifdef USE_DEBUG_CODE
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	Lamb::AddLog("Finalize ImGuiManager succeeded");
#endif // DEBUG
}

void ImGuiManager::Start() {
#ifdef USE_DEBUG_CODE
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
#endif // USE_DEBUG_CODE
}

void ImGuiManager::End() {
#ifdef USE_DEBUG_CODE
	ID3D12GraphicsCommandList* const commandList = DirectXCommand::GetMainCommandlist()->GetCommandList();
	// ImGui描画
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // DEBUG
}