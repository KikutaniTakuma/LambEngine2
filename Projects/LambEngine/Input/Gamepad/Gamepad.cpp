/// =============================
/// ==  Frameworkクラスの定義  ==
/// =============================


#include "GamePad.h"
#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE
#include <limits>
#include <algorithm>
#include <format>
#include "Math/Vector2.h"

Gamepad::Gamepad() :
	preState_(0),
	state_({0}),
	vibration_({0}),
	isPreStickNeutral({ true, true }),
	isPadConnecting_(false)
{}

Gamepad* const Gamepad::GetInstance() {
	static Gamepad pInstance;
	return &pInstance;
}

void Gamepad::Input() {
	isPreStickNeutral = { 
		GetStick(Stick::LEFT).LengthSQ() == 0.0f,
		GetStick(Stick::RIGHT).LengthSQ() == 0.0f 
	};

	preState_ = state_;
    DWORD dresult = XInputGetState(0, &state_);
	// 接続状況の確認
	dresult == ERROR_SUCCESS ? isPadConnecting_ = true : isPadConnecting_ = false;
}

void Gamepad::InputReset() {
	preState_ = {  };
	state_ = {  };
	vibration_ = {  };
}

bool Gamepad::GetButton(Button type) {
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE
    return (state_.Gamepad.wButtons >> static_cast<short>(type)) % 2 == 1;
}

bool Gamepad::GetPreButton(Button type) {
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE
	return (preState_.Gamepad.wButtons >> static_cast<short>(type)) % 2 == 1;
}

bool Gamepad::Pushed(Button type) {
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE
	return GetButton(type) && !GetPreButton(type);
}

bool Gamepad::LongPush(Button type) {
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE
	return GetButton(type) && GetPreButton(type);
}

bool Gamepad::Released(Button type) {
	return !GetButton(type) && GetPreButton(type);
}

bool Gamepad::PushAnyKey() {
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE
	Gamepad* instance = Gamepad::GetInstance();
	Vector2 leftStick = GetStick(Stick::LEFT);
	Vector2 rightStick = GetStick(Stick::RIGHT);

	if (instance->state_.Gamepad.bLeftTrigger
		|| instance->state_.Gamepad.bRightTrigger
		|| leftStick.x < -0.3f || 0.3f <leftStick.x
		|| leftStick.y < -0.3f || 0.3f <leftStick.y
		|| rightStick.x < -0.3f || 0.3f < rightStick.x
		|| rightStick.y < -0.3f || 0.3f < rightStick.y
		) {
		return true;
	}

	return instance->state_.Gamepad.wButtons != instance->preState_.Gamepad.wButtons;
}

float Gamepad::GetTriger(Triger type, float deadZone) {
	static constexpr float kNormal = 1.0f / static_cast<float>(UCHAR_MAX);
	float moveTriger = 0.0f;
	switch (type)
	{
	case Gamepad::Triger::LEFT:
		moveTriger = static_cast<float>(state_.Gamepad.bLeftTrigger) * kNormal;
		break;

	case Gamepad::Triger::RIGHT:
		moveTriger = static_cast<float>(state_.Gamepad.bRightTrigger) * kNormal;
		break;

	default:
		return 0.0f;
	}

	return moveTriger <= deadZone ? 0.0f : moveTriger;
}

Vector2 Gamepad::GetStick(Stick type, float deadZone) {
	static constexpr float kNormal = 1.0f / static_cast<float>(SHRT_MAX);
	Vector2 moveStick = Vector2::kZero;
	deadZone = std::clamp(deadZone, 0.0f, 1.0f);

	switch (type)
	{
	case Gamepad::Stick::LEFT:
		moveStick.x = static_cast<float>(state_.Gamepad.sThumbLX) * kNormal;
		moveStick.y = static_cast<float>(state_.Gamepad.sThumbLY) * kNormal;
		break;
	case Gamepad::Stick::RIGHT:
		moveStick.x = static_cast<float>(state_.Gamepad.sThumbRX) * kNormal;
		moveStick.y = static_cast<float>(state_.Gamepad.sThumbRY) * kNormal;
		break;
	default:
		return Vector2::kZero;
	}

	float length = moveStick.Length();

	// もしデッドゾーン内だった場合は0.0fを返す
	if (length <= deadZone) {
		return Vector2::kZero;
	}

	return moveStick;
}

Vector2 Gamepad::GetPreStick(Stick type, float deadZone)
{
	static constexpr float kNormal = 1.0f / static_cast<float>(SHRT_MAX);
	Vector2 moveStick = Vector2::kZero;
	deadZone = std::clamp(deadZone, 0.0f, 1.0f);

	switch (type)
	{
	case Gamepad::Stick::LEFT:
		moveStick.x = static_cast<float>(preState_.Gamepad.sThumbLX) * kNormal;
		moveStick.y = static_cast<float>(preState_.Gamepad.sThumbLY) * kNormal;
		break;
	case Gamepad::Stick::RIGHT:
		moveStick.x = static_cast<float>(preState_.Gamepad.sThumbRX) * kNormal;
		moveStick.y = static_cast<float>(preState_.Gamepad.sThumbRY) * kNormal;
		break;
	default:
		return Vector2::kZero;
	}

	float length = moveStick.Length();

	// もしデッドゾーン内だった場合は0.0fを返す
	if (length <= deadZone) {
		return Vector2::kZero;
	}

	return moveStick;
}

bool Gamepad::GetButtonStick(Button buttonType, Stick stickType, float deadZone) {
	bool result = false;
	switch (buttonType)
	{
	case Gamepad::Button::UP:
	case Gamepad::Button::Y:
		result = GetButton(buttonType) or (GetStick(stickType, deadZone).y > deadZone);
		break;
	case Gamepad::Button::DOWN:
	case Gamepad::Button::A:
		result = GetButton(buttonType) or (GetStick(stickType, deadZone).y < -deadZone);
		break;
	case Gamepad::Button::LEFT:
	case Gamepad::Button::X:
		result = GetButton(buttonType) or (GetStick(stickType, deadZone).x < -deadZone);
		break;
	case Gamepad::Button::RIGHT:
	case Gamepad::Button::B:
		result = GetButton(buttonType) or (GetStick(stickType, deadZone).x > deadZone);
		break;
	case Gamepad::Button::START:
	case Gamepad::Button::BACK:
	case Gamepad::Button::LEFT_THUMB:
	case Gamepad::Button::RIGHT_THUMB:
	case Gamepad::Button::LEFT_SHOULDER:
	case Gamepad::Button::RIGHT_SHOULDER:
	default:
		break;
	}


	return result;
}

bool Gamepad::PushedButtonStick(Button buttonType, Stick stickType, float deadZone)
{
	bool result = false;
	bool isNeutral = isPreStickNeutral[static_cast<int>(stickType)];
	switch (buttonType)
	{
	case Gamepad::Button::UP:
	case Gamepad::Button::Y:
		result = Pushed(buttonType) or (GetStick(stickType, deadZone).y > deadZone and isNeutral);
		break;
	case Gamepad::Button::DOWN:
	case Gamepad::Button::A:
		result = Pushed(buttonType) or (GetStick(stickType, deadZone).y < -deadZone and isNeutral);
		break;
	case Gamepad::Button::LEFT:
	case Gamepad::Button::X:
		result = Pushed(buttonType) or (GetStick(stickType, deadZone).x < -deadZone and isNeutral);
		break;
	case Gamepad::Button::RIGHT:
	case Gamepad::Button::B:
		result = Pushed(buttonType) or (GetStick(stickType, deadZone).x > deadZone and isNeutral);
		break;
	case Gamepad::Button::START:
	case Gamepad::Button::BACK:
	case Gamepad::Button::LEFT_THUMB:
	case Gamepad::Button::RIGHT_THUMB:
	case Gamepad::Button::LEFT_SHOULDER:
	case Gamepad::Button::RIGHT_SHOULDER:
	default:
		break;
	}


	return result;
}

bool Gamepad::ReleasedButtonStick(Button buttonType, Stick stickType, float deadZone)
{
	bool result = false;
	bool isNeutral = GetStick(stickType).LengthSQ() == 0.0f;
	switch (buttonType)
	{
	case Gamepad::Button::UP:
	case Gamepad::Button::Y:
		result = Released(buttonType) or (GetPreStick(stickType, deadZone).y > deadZone and isNeutral);
		break;
	case Gamepad::Button::DOWN:
	case Gamepad::Button::A:
		result = Released(buttonType) or (GetPreStick(stickType, deadZone).y < -deadZone and isNeutral);
		break;
	case Gamepad::Button::LEFT:
	case Gamepad::Button::X:
		result = Released(buttonType) or (GetPreStick(stickType, deadZone).x < -deadZone and isNeutral);
		break;
	case Gamepad::Button::RIGHT:
	case Gamepad::Button::B:
		result = Released(buttonType) or (GetPreStick(stickType, deadZone).x > deadZone and isNeutral);
		break;
	case Gamepad::Button::START:
	case Gamepad::Button::BACK:
	case Gamepad::Button::LEFT_THUMB:
	case Gamepad::Button::RIGHT_THUMB:
	case Gamepad::Button::LEFT_SHOULDER:
	case Gamepad::Button::RIGHT_SHOULDER:
	default:
		break;
	}


	return result;
}

void Gamepad::Vibration(float leftVibIntensity, float rightVibIntensity) {
	leftVibIntensity = std::clamp(leftVibIntensity, 0.0f, 1.0f);
	rightVibIntensity = std::clamp(rightVibIntensity, 0.0f, 1.0f);

	vibration_.wLeftMotorSpeed = static_cast<WORD>(static_cast<float>(USHRT_MAX) * leftVibIntensity);
	vibration_.wRightMotorSpeed = static_cast<WORD>(static_cast<float>(USHRT_MAX) * rightVibIntensity);
	XInputSetState(0, &vibration_);
}

void Gamepad::Debug() {
#ifdef USE_DEBUG_CODE
	ImGui::SetNextWindowSizeConstraints({}, { 210.0f, 400.0f });
	ImGui::Begin("Gamepad Debug");
	ImGui::Text(std::format("isConecting : {}", isPadConnecting_).c_str());
	if (ImGui::TreeNode("stick")) {
		ImGui::Text("LeftX          = %.2f%%\n", GetStick(Stick::LEFT, 0.0f).x * 100.0f);
		ImGui::Text("LeftY          = %.2f%%\n", GetStick(Stick::LEFT, 0.0f).y * 100.0f);
		ImGui::Text("RightX         = %.2f%%\n", GetStick(Stick::RIGHT, 0.0f).x * 100.0f);
		ImGui::Text("RightY         = %.2f%%\n", GetStick(Stick::RIGHT, 0.0f).y * 100.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("triger")) {
		ImGui::Text("LEFT_TRIGER    = %.2f%%\n", GetTriger(Triger::LEFT, 0.0f) * 100.0f);
		ImGui::Text("RIGHT_TRIGER   = %.2f%%\n", GetTriger(Triger::RIGHT, 0.0f) * 100.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("ABXY")) {
		ImGui::Text("A              = %d\n", GetButton(Button::A));
		ImGui::Text("B              = %d\n", GetButton(Button::B));
		ImGui::Text("X              = %d\n", GetButton(Button::X));
		ImGui::Text("Y              = %d\n", GetButton(Button::Y));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("cross button")) {
		ImGui::Text("UP             = %d\n", GetButton(Button::UP));
		ImGui::Text("DOWN           = %d\n", GetButton(Button::DOWN));
		ImGui::Text("LEFT           = %d\n", GetButton(Button::LEFT));
		ImGui::Text("RIGHT          = %d\n", GetButton(Button::RIGHT));
		ImGui::TreePop();
	}	
	if (ImGui::TreeNode("other")) {
		ImGui::Text("START          = %d\n", GetButton(Button::START));
		ImGui::Text("BACK           = %d\n", GetButton(Button::BACK));
		ImGui::Text("LEFT_THUMB     = %d\n", GetButton(Button::LEFT_THUMB));
		ImGui::Text("RIGHT_THUMB    = %d\n", GetButton(Button::RIGHT_THUMB));
		ImGui::Text("LEFT_SHOULDER  = %d\n", GetButton(Button::LEFT_SHOULDER));
		ImGui::Text("RIGHT_SHOULDER = %d\n", GetButton(Button::RIGHT_SHOULDER));

		ImGui::NewLine();
		ImGui::Text("UpAndStick     = %d\n", GetButtonStick(Button::UP, Stick::LEFT));
		ImGui::Text("DownAndStick   = %d\n", GetButtonStick(Button::DOWN, Stick::LEFT));
		ImGui::Text("LeftAndStick   = %d\n", GetButtonStick(Button::LEFT, Stick::LEFT));
		ImGui::Text("RightAndStick  = %d\n", GetButtonStick(Button::RIGHT, Stick::LEFT));

		if (ImGui::TreeNode("ボタンスティックの押した瞬間")) {
			ImGui::Text("UpAndStick     = %d\n", PushedButtonStick(Button::UP, Stick::LEFT));
			ImGui::Text("DownAndStick   = %d\n", PushedButtonStick(Button::DOWN, Stick::LEFT));
			ImGui::Text("LeftAndStick   = %d\n", PushedButtonStick(Button::LEFT, Stick::LEFT));
			ImGui::Text("RightAndStick  = %d\n", PushedButtonStick(Button::RIGHT, Stick::LEFT));
			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("ボタンスティックの離した瞬間")) {
			ImGui::Text("UpAndStick     = %d\n", ReleasedButtonStick(Button::UP, Stick::LEFT));
			ImGui::Text("DownAndStick   = %d\n", ReleasedButtonStick(Button::DOWN, Stick::LEFT));
			ImGui::Text("LeftAndStick   = %d\n", ReleasedButtonStick(Button::LEFT, Stick::LEFT));
			ImGui::Text("RightAndStick  = %d\n", ReleasedButtonStick(Button::RIGHT, Stick::LEFT));
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("vibration")) {
		static Vector2 vibration;
		ImGui::DragFloat2("Vib", vibration.data(), 0.01f, 0.0f, 1.0f);
		Gamepad::Vibration(vibration.x, vibration.y);
		ImGui::TreePop();
	}

	ImGui::End();
#endif // USE_DEBUG_CODE
}