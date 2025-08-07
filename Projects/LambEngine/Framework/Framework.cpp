#include "Framework.h"
#include "Core/Window/WindowFactory.h"
#include "Data/WindowStartUpData.h"
#include "Utils/FileUtils.h"

#include "Input/Input.h"

namespace Lamb {
	void Framework::Execution() {
		this->InitializeLibrary_();

		SafePtr window = WindowFactory::GetInstance();

		// メインループ
		while (window->WindowMassage()) {
			// フルスクリーン/windowの変更
			window->Fullscreen();

			// windowの位置の更新
			window->UpdateCurrentPos();

		}

		this->FinalizeLibrary_();
	}

	void Framework::InitializeLibrary_() {
		// Window初期化
		WindowFactory::Initialize();
		SafePtr window = WindowFactory::GetInstance();

		// jsonをロード
		auto json = LoadJson("EngineData/Window.json");

		WindowStartUpData windata;

		// jsonからデータ出力
		JsonToData(json, windata);

		window->Create(
			windata.windowTitle,
			windata.width,
			windata.height,
			windata.isFullscreen
		);

		// 入力クラス初期化
		Input::Initialize();
	}

	void Framework::FinalizeLibrary_() {
		// 入力クラス開放
		Input::Finalize();

		// Window終了
		WindowFactory::Finalize();
	}
}