#include "Keyboard.h"
#include "../DirectXTex/TextureLoad.h"
#include <windows.h>

//--- グローバル変数
BYTE g_keyTable[256];		//キー情報を保存しておくための配列
BYTE g_oldKeyTable[256];	//ひとつ前のキー情報を保存しておくための配列
float g_count[256] = {};	//キー入力ごとに独立したカウンター

//更新
void UpdateKeyboard()
{
	memcpy(g_oldKeyTable,g_keyTable,sizeof(g_oldKeyTable));		//ひとつ前の情報をコピー
	GetKeyboardState(g_keyTable);								//キー情報をテーブルに格納
}

//入力されたかどうか
bool isKeyPress(int nVirtKey)
{
	return g_keyTable[nVirtKey] & 0x80;
}

//押されたかどうか
bool isKeyTrigger(int nVirtKey)
{
	return (g_keyTable[nVirtKey] ^ g_oldKeyTable[nVirtKey]) & g_keyTable[nVirtKey] & 0x80;
}

//離されたかどうか
bool isKeyRelease(int nVirtKey)
{
	return (g_keyTable[nVirtKey] ^ g_oldKeyTable[nVirtKey]) & g_oldKeyTable[nVirtKey] & 0x80;
}

//入力され続けているかどうか
bool isKeyRepeat(int nVirtKey)
{
	//最初のキー入力を取得
	if (isKeyPress(nVirtKey))
	{
		g_count[nVirtKey]++;					//押している間カウントを進める
		if (g_count[nVirtKey] > REPEAT_COUNT)	//特定のフレーム数押し続けたらtrue
			return true;
	}
	else
	{
		g_count[nVirtKey] = 0.0f;				//離したらカウントをリセット
	}

	return false;
}
