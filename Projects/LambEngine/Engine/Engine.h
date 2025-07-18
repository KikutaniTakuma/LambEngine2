/// ==========================
/// ==  Engineクラスの宣言  ==
/// ==========================


#pragma once
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")

#include "EngineUtils/LambPtr/LambPtr.h"
#include <Utils/SafePtr.h>

#include "Core/DirectXCommand/DirectXCommand.h"
#include "Core/DirectXSwapChain/DirectXSwapChain.h"
#include <string>
#include <memory>
#include <unordered_map>

/// <summary>
/// 各種マネージャーやDirectX12関連、ウィンドウ関連の初期化と解放を担う
/// </summary>
class Engine {
private:
	Engine() = default;
	Engine(const Engine&) = delete;
	Engine(Engine&&) = default;
	~Engine() = default;

	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;

public:
	/// <summary>
	/// 失敗した場合内部でassertで止められる
	/// </summary>
	/// <param name="windowName">Windowの名前</param>
	/// <param name="windowSize">ウィンドウの大きさ(バックバッファの大きさも同じになる)</param>
	/// <param name="fpsLimit">最大fps設定デフォルトで60</param>
	/// <param name="isFullscreen">フルスクリーンモードか否か</param>
	static void Initialize(
		const std::string& windowName, 
		const class Vector2& windowSize, 
		float fpsLimit = 60.0f, 
		bool isFullscreen = false
	);

	static void Finalize();

	static bool IsFinalize();

private:
	/// <summary>
	/// シングルトンインスタンス
	/// </summary>
	static Engine* pInstance_;

private:
	bool isFinalize_ = false;

public:
	static inline Engine* const GetInstance() {
		return pInstance_;
	}

private:
	std::string GetCpuName() const;

	void HardwareLog() const;

#ifdef USE_DEBUG_CODE
	///
	/// Debug用
	/// 
private:
	class Debug {
	public:
		Debug();
		~Debug();

	public:
		void InitializeDebugLayer();

	private:
		Lamb::LambPtr<ID3D12Debug1> debugController_;
	};

	static Debug debugLayer_;
#endif




	/// 
	/// DirctXDevice
	/// 
private:
	void InitializeDirectXDevice();

private:
	Lamb::SafePtr<class DirectXDevice> directXDevice_ = nullptr;

	/// 
	/// DirectXCommand
	/// 
private:
	void InitializeDirectXCommand();
	void FinalizeDirectXCommand();

public:
	DirectXCommand* const GetMainCommandlist() const;

private:
	std::unique_ptr<DirectXCommand> directXCommand_;

/// 
/// DirectXCommand
/// 
private:
	void InitializeDirectXSwapChain();

private:
	Lamb::SafePtr<class DirectXSwapChain> directXSwapChain_ = nullptr;

	
/// <summary>
/// DirectXTK
/// </summary>
private:
	void InitializeDirectXTK();

private:
	Lamb::SafePtr<class StringOutPutManager> stringOutPutManager_ = nullptr;


	///
	/// MainLoop
	/// 
public:
	static void FrameStart();

	static void FrameEnd();
};