/// =========================
/// ==  Mouseクラスの定義  ==
/// =========================


#include "Mouse.h"
#include "Engine/Core/WindowFactory/WindowFactory.h"
#ifdef USE_DEBUG_CODE
#include "imgui.h"
#endif // USE_DEBUG_CODE
#include "Utils/ExecutionLog.h"
#include "Error/Error.h"

#include <cassert>


Mouse* Mouse::pInstance_ = nullptr;

void Mouse::Initialize(IDirectInput8* input) {
	pInstance_ = new Mouse(input);
	if (!pInstance_) {
		throw Lamb::Error::Code<Mouse>("instance failed", ErrorPlace);
	}
}


void Mouse::Finalize() {
	delete pInstance_;
	pInstance_ = nullptr;
}

Mouse::Mouse(IDirectInput8* input) :
	mouse_(),
	mosueState_(),
	preMosueState_(),
	wheel_(0),
	initalizeSucceeded_(false)
{
	assert(input);
	HRESULT hr = input->CreateDevice(GUID_SysMouse, mouse_.GetAddressOf(), NULL);
	assert(SUCCEEDED(hr));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<Mouse>("CreateDevice failed", ErrorPlace);
	}

	hr = mouse_->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(hr));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<Mouse>("SetDataFormat failed", ErrorPlace);
	}

	hr = mouse_->SetCooperativeLevel(WindowFactory::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
	if (!SUCCEEDED(hr)) {
		throw Lamb::Error::Code<Mouse>("SetCooperativeLevel failed", ErrorPlace);
	}

	initalizeSucceeded_ = true;

	Lamb::AddLog("Initialize Mouse succeeded");
}

Mouse::~Mouse() {
	if (mouse_) {
		mouse_->Unacquire();
	}
}



void Mouse::Input() {
	if (!initalizeSucceeded_) {
		return;
	}

	preMosueState_ = mosueState_;

	mouse_->Acquire();

	mosueState_ = {};
	mouse_->GetDeviceState(sizeof(DIMOUSESTATE2), &mosueState_);

	wheel_ += static_cast<size_t>(mosueState_.lZ);
}

void Mouse::InputReset() {
	mosueState_ = {};
	preMosueState_ = {};
}

bool Mouse::Pushed(Mouse::Button button) {
	if (!initalizeSucceeded_) {
		return false;
	}
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE


	if ((mosueState_.rgbButtons[uint8_t(button)] & 0x80) &&
		!(preMosueState_.rgbButtons[uint8_t(button)] & 0x80))
	{
		return true;
	}

	return false;
}

bool Mouse::LongPush(Mouse::Button button) {
	if (!initalizeSucceeded_) {
		return false;
	}

#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE

	if ((mosueState_.rgbButtons[uint8_t(button)] & 0x80) &&
		(preMosueState_.rgbButtons[uint8_t(button)] & 0x80))
	{
		return true;
	}

	return false;
}

bool Mouse::Released(Mouse::Button button) {
	if (!initalizeSucceeded_) {
		return false;
	}

#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE

	if (!(mosueState_.rgbButtons[uint8_t(button)] & 0x80) &&
		(preMosueState_.rgbButtons[uint8_t(button)] & 0x80))
	{
		return true;
	}

	return false;
}

bool Mouse::PushAnyKey() {
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return false;
	}
#endif // USE_DEBUG_CODE

	for (size_t i = 0; i < 8; i++) {
		if (mosueState_.rgbButtons[i] != preMosueState_.rgbButtons[i]) {
			return true;
		}
	}

	if (
		mosueState_.lX != preMosueState_.lX
		|| mosueState_.lY != preMosueState_.lY
		|| mosueState_.lZ != preMosueState_.lZ
		) {
		return true;
	}

	return false;
}

Vector2 Mouse::GetVelocity() {
	if (!initalizeSucceeded_) {
		return Vector2::kZero;
	}

	return { static_cast<float>(mosueState_.lX), -static_cast<float>(mosueState_.lY) };
}

float Mouse::GetWheel() {
	if (!initalizeSucceeded_) {
		return 0.0f;
	}

#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return 0.0f;
	}
#endif // USE_DEBUG_CODE
	return static_cast<float>(wheel_);
}

float Mouse::GetWheelVelocity() {
	if (!initalizeSucceeded_) {
		return 0.0f;
	}
#ifdef USE_DEBUG_CODE
	if (ImGui::GetIO().WantCaptureMouse or ImGui::IsAnyItemHovered()) {
		return 0.0f;
	}
#endif // USE_DEBUG_CODE
	return static_cast<float>(mosueState_.lZ);
}

Vector2 Mouse::GetPos() {
	if (!initalizeSucceeded_) {
		return Vector2::kZero;
	}

	POINT p{};
	GetCursorPos(&p);
	ScreenToClient(FindWindowW(WindowFactory::GetInstance()->GetWindowClassName().c_str(), nullptr), &p);

	Vector2 pos{ static_cast<float>(p.x),static_cast<float>(p.y) };

	return pos;
}

void Mouse::SetPos(const Vector2& pos) {
	static WindowFactory* const window = WindowFactory::GetInstance();
	Vector2 windowPos = window->GetPos();
	SetCursorPos(static_cast<int>(pos.x + windowPos.x), static_cast<int>(pos.y + windowPos.y));
}

void Mouse::Show(bool flg) {
	::ShowCursor(BOOL(flg));
}

void Mouse::Debug() {
#ifdef USE_DEBUG_CODE
	ImGui::Begin("Mouse Debug");
	ImGui::Text("Left          : %d", LongPush(Mouse::Button::Left));
	ImGui::Text("Midle         : %d", LongPush(Mouse::Button::Middle));
	ImGui::Text("Right         : %d", LongPush(Mouse::Button::Right));
	ImGui::Text("EX0           : %d", LongPush(Mouse::Button::EX0));
	ImGui::Text("EX1           : %d", LongPush(Mouse::Button::EX1));
	ImGui::Text("EX2           : %d", LongPush(Mouse::Button::EX2));
	ImGui::Text("EX3           : %d", LongPush(Mouse::Button::EX3));
	ImGui::Text("EX4           : %d", LongPush(Mouse::Button::EX4));
	ImGui::Text("pos           : %.2f, %.2f", GetPos().x, GetPos().y);
	ImGui::Text("velocity      : %.2f, %.2f", GetVelocity().x, GetVelocity().y);
	ImGui::Text("Wheel         : %.0f", GetWheel());
	ImGui::Text("WheelVelocity : %.0f", GetWheelVelocity());
	ImGui::End();
#endif // USE_DEBUG_CODE
}