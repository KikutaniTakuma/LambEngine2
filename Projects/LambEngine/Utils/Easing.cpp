#include "Easing.h"
#include "Utils/EngineInfo.h"
#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE

#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include <cmath>
#include <numbers>

const std::array<std::string, size_t(Easing::Type::kNum)> Easing::kEaseingTypeString_ = {
		"None",

		"InSine",
		"OutSine",
		"InOutSine",

		"InQuad",
		"OutQuad",
		"InOutQuad",

		"InCubic",
		"OutCubic",
		"InOutCubic",

		"InQuart",
		"OutQuart",
		"InOutQuart",

		"InQuint",
		"OutQuint",
		"InOutQuint",

		"InExpo",
		"OutExpo",
		"InOutExpo",

		"InCirc",
		"OutCirc",
		"InOutCirc",

		"InBack",
		"OutBack",
		"InOutBack",

		"InElastic",
		"OutElastic",
		"InOutElastic",

		"InBounce",
		"OutBounce",
		"InOutBounce"
};

Easing::Easing():
	isActive_(false),
	isLoop_(false),
	t_(0.0f),
	spdT_(1.0f),
	type_(Type::kNone)
{}

void Easing::Update() {
	if (isActive_) {
		t_ += spdT_ * Lamb::DeltaTime();
		t_ = std::clamp(t_, 0.0f, 1.0f);

		if (t_ <= 0.0f || 1.0f <= t_) {
			if (isLoop_) {
				spdT_ *= -1.0f;
			}
			else {
				Stop();
			}
		}
	}
}

void Easing::Start(
	bool isLoop, 
	float easeTime, 
	Type type
) {
	isActive_ = true;
	isLoop_ = isLoop;
	t_ = 0.0f;
	spdT_ = 1.0f / easeTime;

	type_ = type;
}
void Easing::Pause() {
	isActive_ = false;
}

void Easing::Restart() {
	isActive_ = true;
}
void Easing::Stop() {
	isActive_ = false;
	isLoop_ = false;
}

void Easing::Reset() {
	t_ = 0.0f;
}


void Easing::Debug([[maybe_unused]]const std::string& debugName) {
#ifdef USE_DEBUG_CODE
	ImGui::Begin(debugName.c_str());
	if (ImGui::BeginCombo("BlendType", kEaseingTypeString_[static_cast<uint32_t>(type_)].c_str()))
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(Type::kNum); ++i)
		{
			bool isSelected = (type_ == static_cast<Type>(i));
			if (ImGui::Selectable(kEaseingTypeString_[i].c_str(), isSelected))
			{
				type_ = static_cast<Type>(i);
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Checkbox("isLoop", isLoop_.data());
	if (ImGui::Button("Start")) {
		isActive_ = true;
		t_ = 0.0f;
	}
	else if (ImGui::Button("Stop")) {
		Stop();
	}
	ImGui::End();

#endif // USE_DEBUG_CODE
}

void Easing::DebugTreeNode([[maybe_unused]] const std::string& debugName) {
#ifdef USE_DEBUG_CODE
	if (ImGui::TreeNode(debugName.c_str())) {
		if (ImGui::BeginCombo("BlendType", kEaseingTypeString_[static_cast<uint32_t>(type_)].c_str()))
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(Type::kNum); ++i)
			{
				bool isSelected = (type_ == static_cast<Type>(i));
				if (ImGui::Selectable(kEaseingTypeString_[i].c_str(), isSelected))
				{
					type_ = static_cast<Type>(i);
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Checkbox("isLoop", isLoop_.data());
		if (ImGui::Button("Start")) {
			isActive_ = true;
			t_ = 0.0f;
		}
		else if (ImGui::Button("Stop")) {
			Stop();
		}

		ImGui::TreePop();
	}

#endif // USE_DEBUG_CODE
}

void Easing::SetType(Type type) {
	type_ = type;
}


float Easing::GetTypeT() const {
	switch (type_)
	{
	case Easing::Type::kNone:
		return t_;
	case Easing::Type::kInSine:
		return InSine(t_);
	case Easing::Type::kOutSine:
		return OutSine(t_);
	case Easing::Type::kInOutSine:
		return InOutSine(t_);
	case Easing::Type::kInQuad:
		return InQuad(t_);
	case Easing::Type::kOutQuad:
		return OutQuad(t_);
	case Easing::Type::kInOutQuad:
		return InOutQuad(t_);
	case Easing::Type::kInCubic:
		return InCubic(t_);
	case Easing::Type::kOutCubic:
		return OutCubic(t_);
	case Easing::Type::kInOutCubic:
		return InOutCubic(t_);
	case Easing::Type::kInQuart:
		return InQuart(t_);
	case Easing::Type::kOutQuart:
		return OutQuart(t_);
	case Easing::Type::kInOutQuart:
		return InOutQuart(t_);
	case Easing::Type::kInQuint:
		return InQuint(t_);
	case Easing::Type::kOutQuint:
		return OutQuint(t_);
	case Easing::Type::kInOutQuint:
		return InOutQuint(t_);
	case Easing::Type::kInExpo:
		return InExpo(t_);
	case Easing::Type::kOutExpo:
		return OutExpo(t_);
	case Easing::Type::kInOutExpo:
		return InOutExpo(t_);
	case Easing::Type::kInCirc:
		return InCirc(t_);
	case Easing::Type::kOutCirc:
		return OutCirc(t_);
	case Easing::Type::kInOutCirc:
		return InOutCirc(t_);
	case Easing::Type::kInBack:
		return InBack(t_);
	case Easing::Type::kOutBack:
		return OutBack(t_);
	case Easing::Type::kInOutBack:
		return InOutBack(t_);
	case Easing::Type::kInElastic:
		return InElastic(t_);
	case Easing::Type::kOutElastic:
		return OutElastic(t_);
	case Easing::Type::kInOutElastic:
		return InOutElastic(t_);
	case Easing::Type::kInBounce:
		return InBounce(t_);
	case Easing::Type::kOutBounce:
		return OutBounce(t_);
	case Easing::Type::kInOutBounce:
		return InOutBounce(t_);
	default:
		return t_;
	}
}

float Easing::InSine(float t) {
	return 1.0f - std::cos((t * std::numbers::pi_v<float>) / 2.0f);
}
float Easing::OutSine(float t) {
	return std::sin((t * std::numbers::pi_v<float>) / 2.0f);
}
float Easing::InOutSine(float t) {
	return -(std::cos(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;
}

float Easing::InQuad(float t) {
	return t * t;
}
float Easing::OutQuad(float t) {
	return 1.0f - (1.0f - t) * (1.0f - t);
}
float Easing::InOutQuad(float t) {
	return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

float Easing::InCubic(float t) {
	return t * t * t;
}
float Easing::OutCubic(float t) {
	return 1.0f - std::pow(1.0f - t, 3.0f);
}
float Easing::InOutCubic(float t) {
	return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float Easing::InQuart(float t) {
	return t * t * t * t;
}
float Easing::OutQuart(float t) {
	return 1.0f - std::pow(1.0f - t, 4.0f);
}
float Easing::InOutQuart(float t) {
	return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

float Easing::InQuint(float t) {
	return t * t * t * t * t;
}
float Easing::OutQuint(float t) {
	return 1.0f - std::pow(1.0f - t, 5.0f);
}
float Easing::InOutQuint(float t) {
	return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) / 2.0f;
}

float Easing::InExpo(float t) {
	return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * t - 10.0f);
}
float Easing::OutExpo(float t) {
	return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}
float Easing::InOutExpo(float t) {
	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: t < 0.5f ? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f
		: (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

float Easing::InCirc(float t) {
	return 1.0f - std::sqrt(1.0f - std::pow(t, 2.0f));
}
float Easing::OutCirc(float t) {
	return std::sqrt(1.0f - std::pow(t - 1.0f, 2.0f));
}
float Easing::InOutCirc(float t) {
	return t < 0.5f
		? (1.0f - std::sqrt(1 - std::pow(2.0f * t, 2.0f))) / 2.0f
		: (std::sqrt(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

float Easing::InBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	return c3 * t * t * t - c1 * t * t;
}
float Easing::OutBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}
float Easing::InOutBack(float t) {
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;

	return t < 0.5f
		? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
		: (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

float Easing::InElastic(float t) {
	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;

	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
}
float Easing::OutElastic(float t) {
	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;

	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}
float Easing::InOutElastic(float t) {
	const float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;

	return t == 0.0f
		? 0.0f
		: t == 1.0f
		? 1.0f
		: t < 0.5f
		? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f
		: (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
}

float Easing::InBounce(float t) {
	return 1.0f - OutBounce(1.0f - t);
}
float Easing::OutBounce(float t) {
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1.0f / d1) {
		return n1 * t * t;
	}
	else if (t < 2.0f / d1) {
		return n1 * (t -= 1.5f / d1) * t + 0.75f;
	}
	else if (t < 2.5f / d1) {
		return n1 * (t -= 2.25f / d1) * t + 0.9375f;
	}
	else {
		return n1 * (t -= 2.625f / d1) * t + 0.984375f;
	}
}
float Easing::InOutBounce(float t) {
	return t < 0.5
		? (1.0f - OutBounce(1.0f - 2.0f * t)) / 2.0f
		: (1.0f + OutBounce(2.0f * t - 1.0f)) / 2.0f;
}