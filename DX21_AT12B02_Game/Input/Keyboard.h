#pragma once

constexpr float REPEAT_COUNT = 30;	//長押しする時間

void UpdateKeyboard();				//キーボード入力の更新
bool isKeyPress(int nVirtKey);		//キーボード入力されているかどうか
bool isKeyTrigger(int nVirtKey);	//キーが押されたか
bool isKeyRelease(int nVirtKey);	//キーが離されたか
bool isKeyRepeat(int nVirtKey);		//一度入力した後も入力され続けているかどうか
