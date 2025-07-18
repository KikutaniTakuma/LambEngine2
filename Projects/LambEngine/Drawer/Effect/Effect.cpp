#include "Effect.h"
#include <Engine/Graphics/EffectDataManager/EffectDataManager.h>

#include <Engine/Core/EffekseerControler/EffekseerControler.h>

#include <numeric>

#ifdef USE_DEBUG_CODE
#include <imgui.h>
#endif // USE_DEBUG_CODE


Effect::Effect() :
	transform(),
	effect_(),
	efkManager_(),
	playHandle_(-1),
	isLoop_(false),
	color_({ .uint = std::numeric_limits<uint32_t>::max() })
{
	efkManager_ = EffekseerControler::GetInstance()->GetEfkManager();
}

void Effect::Load(const std::string& fileName) {
	EffectDataManager::GetInstance()->Load(fileName);
	effect_ = EffectDataManager::GetInstance()->Get(fileName);
}

void Effect::Update() {
	bool isPlay = IsPlay();

	if (isLoop_ and isStart_ and not isPlay) {
		Play();
		isPlay = true;
	}
	if (isPlay) {
		Vector3 axis;
		float angle;
		transform.rotate.DecomposeToAxisAngle(axis, angle);

		efkManager_->SetLocation(
			playHandle_, 
			transform.translate.x,
			transform.translate.y,
			transform.translate.z
		);
		efkManager_->SetRotation(
			playHandle_,
			{axis.x,axis.y, axis.z},
			angle
		);
		efkManager_->SetScale(
			playHandle_,
			transform.scale.x,
			transform.scale.y,
			transform.scale.z
		);

		efkManager_->SetAllColor(
			playHandle_,
			InvertEfkColor_(color_.efk)
		);
	}
	else {
		isStart_ = false;
	}

}

void Effect::Play() {
	if (not IsPlay()) {
		isStart_ = true;

		playHandle_ = efkManager_->Play(
			effect_, 
			transform.translate.x,
			transform.translate.y,
			transform.translate.z
		);

		Vector3 axis;
		float angle;
		transform.rotate.DecomposeToAxisAngle(axis, angle);

		efkManager_->SetRotation(
			playHandle_,
			{ axis.x,axis.y, axis.z },
			angle
		);
		efkManager_->SetScale(
			playHandle_,
			transform.scale.x,
			transform.scale.y,
			transform.scale.z
		);
	}
}

void Effect::Stop() {
	isStart_ = false;
	efkManager_->StopEffect(playHandle_);
	playHandle_ = -1;
}

void Effect::Pause() {
	efkManager_->SetPaused(playHandle_, true);
}

void Effect::Restart() {
	efkManager_->SetPaused(playHandle_, false);
}

void Effect::SetIsLoop(bool isLoop) {
	isLoop_ = isLoop;
}

void Effect::SetColor(uint32_t color) {
	color_.uint = color;
}

void Effect::SetColor(Vector4 color)
{
	color_.uint = color.GetColorRGBA();
}

bool Effect::IsPlay()
{
	return efkManager_->Exists(playHandle_);
}

void Effect::Debug([[maybe_unused]]const std::string& guiName) {
#ifdef USE_DEBUG_CODE
	if (ImGui::TreeNode(guiName.c_str())) {
		transform.Debug("transform");
		
		if (ImGui::TreeNode("param")) {
			debugColor_ = color_.uint;
			ImGui::ColorEdit4("色", debugColor_.data());
			color_.uint = debugColor_.GetColorRGBA();

			ImGui::Checkbox("ループ", &isLoop_);
			ImGui::TreePop();
		}

		if (ImGui::Button("Play")) {
			Play();
		}
		if (ImGui::Button("Stop")) {
			Stop();
		}
		if (ImGui::Button("Pause")) {
			Pause();
		}
		if (ImGui::Button("Restart")) {
			Restart();
		}

		ImGui::TreePop();
	}
#endif // USE_DEBUG_CODE
}

Effekseer::Color Effect::InvertEfkColor_(Effekseer::Color color)
{
	return Effekseer::Color(color.A, color.B, color.G, color.R);
}
