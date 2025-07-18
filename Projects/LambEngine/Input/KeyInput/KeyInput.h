/// ============================
/// ==  KeyInputクラスの宣言  ==
/// ============================


#pragma once
#include <array>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "Engine/EngineUtils/LambPtr/LambPtr.h"

/// <summary>
/// キーボードの入力
/// </summary>
class KeyInput {
private:
	KeyInput() = delete;
	KeyInput(const KeyInput&) = delete;
	KeyInput(KeyInput&&) = delete;
	KeyInput(IDirectInput8* input);
	~KeyInput();
	KeyInput& operator=(const KeyInput&) = delete;
	KeyInput& operator=(KeyInput&&) = delete;
	

public:
	void Input();

	void InputReset();

	bool GetKey(uint8_t keyType) {
		return (pInstance_->key_[keyType] & 0x80);
	}

	bool GetPreKey(uint8_t keyType) {
		return (pInstance_->preKey_[keyType] & 0x80);
	}

	/// <summary>
	/// 押した瞬間
	/// </summary>
	bool Pushed(uint8_t keyType);

	/// <summary>
	/// 押している間
	/// </summary>
	bool LongPush(uint8_t keyType);
	
	/// <summary>
	/// 話した瞬間
	/// </summary>
	bool Released(uint8_t keyType);

	/// <summary>
	/// 何かしらのキーが押された
	/// </summary>
	/// <returns>押されたらtrue</returns>
	bool PushAnyKey();

public:
	static void Initialize(IDirectInput8* input);
	static void Finalize();

private:
	static KeyInput* pInstance_;

public:
	static KeyInput* const GetInstance() {
		return pInstance_;
	}


private:
	Lamb::LambPtr<IDirectInputDevice8> keyBoard_;

	// キー入力バッファー
	std::array<BYTE, 0x100> key_;
	std::array<BYTE, 0x100> preKey_;

	bool initalizeSucceeded_;
};