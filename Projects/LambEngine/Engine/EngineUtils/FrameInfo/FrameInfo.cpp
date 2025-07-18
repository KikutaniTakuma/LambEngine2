/// ==============================
/// ==  FrameInfoクラスの定義  ==
/// ==============================


#include "FrameInfo.h"

#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE

#include "Engine/Core/WindowFactory/WindowFactory.h"
#include "Utils/ExecutionLog.h"
#include "Input/KeyInput/KeyInput.h"


#include <cmath>
#include <thread>
#include <Windows.h>
#undef max
#undef min
#include <fstream>
#include <string>
#include <format>
#include <algorithm>

FrameInfo::FrameInfo() :
	kMaxMonitorFps_(GetMainMonitorFramerate()),
#ifdef USE_DEBUG_CODE
	isDebugStopGame_(false),
	isOneFrameActive_(false),
	isFixedDeltaTime_(false),
#endif // USE_DEBUG_CODE
	frameStartTime_(),
	deltaTime_(0.0f),
	fps_(0.0f),
	maxFps_(0.0f),
	minFps_(0.0f),
	frameCount_(0),
	fpsLimit_(0.0f),
	minTime_(),
	minCheckTime_(),
	gameSpeedSccale_(1.0f),
	frameDatas_(),
	frameDataDuration_(1),
	frameDataDurationStartTime_{},
	avgProcDuration_{600llu},
	fpsStringOutPut_{},
	isDrawFps_(false),
	isStartFrameInfo_(false)
{
	// リフレッシュレート取得
	fps_ = kMaxMonitorFps_;
	minFps_ = fps_;
	deltaTime_ = 1.0f / fps_;

	auto nowTime = std::chrono::steady_clock::now();
	gameStartTime_ = nowTime;
	reference_ = nowTime;
	frameDataDurationStartTime_ = nowTime;

	maxFpsLimit_ = fps_;

	SetFpsLimit(fps_);

	fpsStringOutPut_.SetFormat("./Resources/Font/default.spritefont");
	fpsStringOutPut_.scale *= 0.3f;
}

FrameInfo::~FrameInfo() {
	auto end = std::chrono::steady_clock::now();

	//画面情報構造体
	DEVMODE mode{};

	//現在の画面情報を取得
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &mode);


	maxFps_ = std::clamp(maxFps_, 0.0f, fpsLimit_);
	minFps_ = std::clamp(minFps_, 0.0f, fpsLimit_);

	float avgFps = 0.0;
	float size = static_cast<float>(frameDatas_.size());
	while (!frameDatas_.empty()) {
		avgFps += frameDatas_.front();

		frameDatas_.pop();
	}

	avgFps /= size;

	Lamb::AddLog(std::format("Frame count : {}", frameCount_));

	auto playtime =
		std::chrono::duration_cast<std::chrono::seconds>(end - gameStartTime_);

	auto h = std::chrono::duration_cast<std::chrono::hours>(playtime);
	playtime -= h;
	auto m = std::chrono::duration_cast<std::chrono::minutes>(playtime);
	playtime -= m;
	auto s = playtime;

	Lamb::AddLog(std::format("Play time : {} {} {}", h,m,s));

	Lamb::AddLog(std::format("Average Fps : {:.2f}", avgFps));
	if (std::chrono::duration_cast<std::chrono::seconds>(end - gameStartTime_) > frameDataDuration_) {
		Lamb::AddLog(std::format("Max Fps : {:.2f}", maxFps_));
		Lamb::AddLog(std::format("Min Fps : {:.2f}", minFps_));
	}
}

FrameInfo* const FrameInfo::GetInstance() {
	static FrameInfo instance;

	return &instance;
}

void FrameInfo::Start() {
	frameStartTime_ = std::chrono::steady_clock::now();
}

void FrameInfo::End() {
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	// 10^-6
	static constexpr float kUnitAdjustment = 0.000001f;
	
	auto elapsed =
		std::chrono::duration_cast<std::chrono::microseconds>(end - reference_);

	if (elapsed < minCheckTime_) {
		while (std::chrono::steady_clock::now() - reference_ < minTime_) {
			std::this_thread::sleep_for(std::chrono::nanoseconds(250));
		}
	}

	end = std::chrono::steady_clock::now();
	auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(end - frameStartTime_);

	deltaTime_ = frameTime.count() * kUnitAdjustment;
	fps_ = 1.0f / deltaTime_;
	fps_ = std::min(fps_, fpsLimit_);

	if (isStartFrameInfo_) {
		maxFps_ = std::max(fps_, maxFps_);
		minFps_ = std::min(fps_, minFps_);
	}

	frameCount_++;
	auto nowTime = std::chrono::steady_clock::now();
	reference_ = nowTime;

#ifdef USE_DEBUG_CODE
	if (frameCount_ < frameRateData_.size()) {
		frameRateData_[frameCount_] = fps_;
	}
	else {
		for (size_t i = 0; (i + 1) < frameRateData_.size(); i++) {
			frameRateData_[i] = frameRateData_[i + 1];
		}
		frameRateData_.back() = fps_;
	}
#endif // USE_DEBUG_CODE

	if (frameDataDuration_ < std::chrono::duration_cast<std::chrono::seconds>(nowTime - frameDataDurationStartTime_)) {
		frameDataDurationStartTime_ = nowTime;
		frameDatas_.push(fps_);

		if (avgProcDuration_ < frameDatas_.size()) {
			float avgFps = 0.0f;
			float size = static_cast<float>(frameDatas_.size());
			while (!frameDatas_.empty()) {
				avgFps += frameDatas_.front();

				frameDatas_.pop();
			}

			avgFps /= size;
			frameDatas_.push(avgFps);
		}
	}
}

void FrameInfo::StartFrameInfo() {
	if (not isStartFrameInfo_) {
		isStartFrameInfo_ = true;
		auto nowTime = std::chrono::steady_clock::now();
		gameStartTime_ = nowTime;
		frameDataDurationStartTime_ = nowTime;
	}
}

void FrameInfo::DrawFps() {
	if (isDrawFps_) {
		fpsStringOutPut_.Clear();
		fpsStringOutPut_ << "fps : " << static_cast<int32_t>(fps_);
		fpsStringOutPut_.Draw();
	}
}

void FrameInfo::SwitchDarwFlg() {
	if (KeyInput::GetInstance()->Pushed(DIK_F3)) {
		isDrawFps_ = !isDrawFps_;
	}
}

void FrameInfo::SetFpsLimit(float fpsLimit) {
	fpsLimit_ = std::clamp(fpsLimit, 10.0f, maxFpsLimit_);
	           
	minTime_ = std::chrono::microseconds(uint64_t(10e5f / fpsLimit_));
	minCheckTime_ = std::chrono::microseconds(uint64_t(10e5f / (fpsLimit_ + (5.0f * (fpsLimit_ / 60.0f)))));
}

void FrameInfo::Debug() {
	this->SwitchDarwFlg();

#ifdef USE_DEBUG_CODE
	static float fpsLimit = fpsLimit_;
	fpsLimit = fpsLimit_;

	if (KeyInput::GetInstance()->Pushed(DIK_F9)) {
		isDebugStopGame_ = !isDebugStopGame_;
	}
	if (KeyInput::GetInstance()->Pushed(DIK_F11)) {
		if (isDebugStopGame_) {
			isOneFrameActive_ = true;
		}
	}

	ImGui::Begin("Frame Information");
	bool isThisWindowActive = WindowFactory::GetInstance()->IsThisWindowaActive();
	ImGui::Text(std::format("This Window Active : {}", isThisWindowActive).c_str());
	ImGui::Text("Frame rate : %3.0lf fps", fps_);
	ImGui::PlotLines(
		"Frame", 
		frameRateData_.data(),
		static_cast<int32_t>(frameRateData_.size()), 
		0, NULL, 0.0f, fpsLimit_
	);
	ImGui::Text("Delta Time : %.4lf", deltaTime_);
	ImGui::Text("Frame Count : %llu", frameCount_);
	ImGui::DragFloat("fps limit", &fpsLimit, 1.0f, 1.0f, kMaxMonitorFps_);
	
	fpsLimit_ = fpsLimit;
	SetFpsLimit(fpsLimit_);
	if (ImGui::TreeNode("DEBUG")) {
		ImGui::Checkbox("is Debug Stop", &isDebugStopGame_);
		if (isDebugStopGame_) {
			if (ImGui::Button("DEBUG frame next")) {
				isOneFrameActive_ = true;
			}
		}
		else {
			ImGui::Checkbox("is fixed deltaTime", &isFixedDeltaTime_);
		}
		ImGui::TreePop();
	}
	ImGui::End();
#endif // USE_DEBUG_CODE
}

void FrameInfo::SetGameSpeedScale(float gameSpeedSccale) {
	gameSpeedSccale_ = std::clamp(gameSpeedSccale, 0.0f, 100.0f);
}

float FrameInfo::GetMainMonitorFramerate() const {
	//画面情報構造体
	DEVMODE mode{};

	//現在の画面情報を取得
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &mode);

	// リフレッシュレート取得
	return static_cast<float>(mode.dmDisplayFrequency);
}

float FrameInfo::GetMaxFpsLimit() const
{
	return maxFpsLimit_;
}
