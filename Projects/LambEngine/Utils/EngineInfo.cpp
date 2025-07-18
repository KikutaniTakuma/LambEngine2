#include "EngineInfo.h"
#include "Engine/EngineUtils/FrameInfo/FrameInfo.h"
#include "Engine/Core/WindowFactory/WindowFactory.h"
#include "Engine/Engine.h"

#include "Engine/Core/DirectXDevice/DirectXDevice.h"

#include "Engine/Graphics/RenderingManager/RenderingManager.h"



namespace Lamb {
	float DeltaTime() {
		static FrameInfo* const frameInfo = FrameInfo::GetInstance();
#ifdef USE_DEBUG_CODE
		float deltatime = frameInfo->GetDelta();
		if (frameInfo->GetIsDebugStop()) {
			const float kStopFrameDeltaTime = 1.0f / frameInfo->GetMaxFpsLimit();
			deltatime = frameInfo->GetIsOneFrameActive() ? kStopFrameDeltaTime : 0.0f;
		}

		return deltatime * frameInfo->GetGameSpeedScale();
#else
		return frameInfo->GetDelta() * frameInfo->GetGameSpeedScale();
#endif // USE_DEBUG_CODE

	}

	float MaxFPS()
	{
		static FrameInfo* const frameInfo = FrameInfo::GetInstance();
		return frameInfo->GetMaxFpsLimit();
	}

	Vector2 ClientSize() {
		static WindowFactory* const windowFactory = WindowFactory::GetInstance();
		return windowFactory->GetClientSize();
	}

	Vector2 WindowSize() {
		static WindowFactory* const windowFactory = WindowFactory::GetInstance();
		return windowFactory->GetWindowSize();
	}

	std::chrono::steady_clock::time_point ThisFrameTime() {
		static FrameInfo* const frameInfo = FrameInfo::GetInstance();
		return frameInfo->GetThisFrameTime();
	}

	bool IsEngineFianlize() {
		return Engine::IsFinalize();
	}
	bool IsCanUseMeshShader() {
		return DirectXDevice::GetInstance()->GetIsCanUseMeshShader();
	}

	uint32_t GetGraphicBufferIndex()
	{
		const Lamb::SafePtr renderingManager = RenderingManager::GetInstance();
		if (renderingManager.empty()) {
			return 0;
		}
		else {
			return renderingManager->GetBufferIndex();
		}
	}
	
}

