// Gamepad.h
#pragma once
#include <dinput.h>
#include <XInput.h>
#include "Vector2.h"
#include<wrl.h>
#include <cstdint>



class Gamepad {
public:

	enum Button : int32_t {
		DPadUp = XINPUT_GAMEPAD_DPAD_UP,
		DPadDown = XINPUT_GAMEPAD_DPAD_DOWN,
		DPadLeft = XINPUT_GAMEPAD_DPAD_LEFT,
		DPadRight = XINPUT_GAMEPAD_DPAD_RIGHT,
		Start = XINPUT_GAMEPAD_START,
		Back = XINPUT_GAMEPAD_BACK,
		LeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,
		RightThumb = XINPUT_GAMEPAD_RIGHT_THUMB,
		LeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
		RightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
		A = XINPUT_GAMEPAD_A,
		B = XINPUT_GAMEPAD_B,
		X = XINPUT_GAMEPAD_X,
		Y = XINPUT_GAMEPAD_Y
	};

private:
	enum class PadType {
		DirectInput,
		XInput,
		None,
	};

	union State {
		DIJOYSTATE2 di;
		XINPUT_STATE xi;
	};

	PadType type_ = PadType::None;
	int32_t deadZoneL_ = 0;
	int32_t deadZoneR_ = 0;
	State state_;
	State statePre_;
	DWORD padNo_ = 0;

	Microsoft::WRL::ComPtr<IDirectInputDevice8> device_ = nullptr;

public:
	Gamepad() = default;
	~Gamepad() = default;


	bool Init(Microsoft::WRL::ComPtr<IDirectInput8> directInput, DWORD padNo);
	void Update();

	// ボタンが押されているか
	bool IsPressButton(int32_t buttonNumber) const;
	// ボタンがトリガーされたか
	bool IsTriggerButton(int32_t buttonNumber) const;
	// スティックの入力を取得
	Vector2 GetStick(int32_t stickNumber) const;

	template<typename T> bool GetState(T& out) const;
	template<typename T> bool GetStatePrevious(T& out) const;

	/// <summary>
	/// デッドゾーン設定
	/// </summary>
	/// <param name="deadZoneL"></param>
	/// <param name="deadZoneR"></param>
	void SetDeadZone(int32_t deadZoneL, int32_t deadZoneR);

	/// <summary>
	/// 振動
	/// </summary>
	/// <param name="leftVelocity"></param>
	/// <param name="rightVelocity"></param>
	void SetVibration(float leftVelocity, float rightVelocity);
};
