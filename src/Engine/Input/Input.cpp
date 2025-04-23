#include "Input.h"
#include <cassert>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(WinApp* winApp)
{
	winApp_ = winApp;
	HRESULT hr;

	//DirectInputのインスタンスを生成
	hr = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイス生成
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	//マウスデバイス生成
	hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	assert(SUCCEEDED(hr));

	//入力データ形式のセット
	hr = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));

	//排他制御レベルのセット
	hr = mouse->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));
}

void Input::Update()
{
	//前回のキー入力を保存
	memcpy(preKey, key, sizeof(key));
	//キーボード情報の取得
	keyboard->Acquire();
	//全キーボード入力情報を取得
	keyboard->GetDeviceState(sizeof(key), key);

	//マウス情報の取得
	mouse->Acquire();
}

void Input::Finalize()
{
	// マウスカーソルを必ず表示に戻す
	SetMouseCursor(true);

	// デバイスの解放
	if (mouse) {
		mouse->Unacquire();
		mouse->Release();
		mouse = nullptr;
	}

	if (keyboard) {
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}

	if (directInput) {
		directInput->Release();
		directInput = nullptr;
	}
}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (key[keyNumber] && !preKey[keyNumber]) {
		return true;
	}
	return false;
}

HRESULT Input::GetMouseState(DIMOUSESTATE* mouseState)
{
	return mouse->GetDeviceState(sizeof(DIMOUSESTATE), mouseState);
}

void Input::SetMouseCursor(bool visible)
{
	ShowCursor(visible);
}