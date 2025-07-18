/// =============================
/// ==  Frameworkクラスの定義  ==
/// =============================


#include "Framework.h"
#include "Engine/Engine.h"
#include "Engine/Core/WindowFactory/WindowFactory.h"
#include "Engine/EngineUtils/FrameInfo/FrameInfo.h"
#include "Utils/ExecutionLog.h"
#include "Error/Error.h"

#include "Input/Input.h"

#include "Drawer/Line/Line.h"

#include "Engine/Graphics/RenderingManager/RenderingManager.h"
#include "Engine/Graphics/RenderContextManager/RenderContextManager.h"

#include <Engine/Graphics/EffectDataManager/EffectDataManager.h>

void Framework::InitializeEngine() {
	// ライブラリ初期化
	Engine::Initialize(initDesc_.windowName, initDesc_.windowSize, initDesc_.maxFps, initDesc_.isFullesceen);

	RenderingManager::Initialize();

	// 入力処理初期化
	Input::Initialize();

	Line::Initialize();

	EffectDataManager::Initialize();
}

void Framework::FinalizeEngine() {
	EffectDataManager::Finalize();

	Line::Finalize();

	// 入力関連解放
	Input::Finalize();

	RenderingManager::Finalize();

	// ライブラリ終了
	Engine::Finalize();
}

void Framework::Execution() {
	try {
		this->InitializeEngine();
		this->Initialize();

		FrameInfo* const frameInfo = FrameInfo::GetInstance();
		Input* const input = Input::GetInstance();
		WindowFactory* const window = WindowFactory::GetInstance();

		frameInfo->StartFrameInfo();

		while (window->WindowMassage()) {
			// 描画開始処理
			Engine::FrameStart();

			// fps
			frameInfo->Debug();

			// 入力処理
			input->InputStart();

			// フルスクリーン化
			window->Fullscreen();

			// 今のウィンドウのポジションを更新
			window->UpdateCurrentPos();

			// 更新処理
			this->Update();

			// 終了リクエストが来たら終わり
			if (this->isEnd_) {
				RenderingManager::GetInstance()->FinalFrame();
				break;
			}

			// 描画処理
			this->Draw();

			// フレーム終了処理
			Engine::FrameEnd();
		}
	}
	catch (const Lamb::Error& err) {
		Lamb::ErrorLog(err);
	}
#ifndef USE_DEBUG_CODE
	catch (const std::exception& err) {
		Lamb::ErrorLog(err.what(), __func__);
	}
#endif // USE_DEBUG_CODE
	this->Finalize();
	this->FinalizeEngine();
}