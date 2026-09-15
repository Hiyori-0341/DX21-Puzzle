#include <Windows.h>
#include "Defines.h"
#include "DirectX.h"
#include "SpriteDrawer.h"
#include "Game.h"
#include "Input/Keyboard.h"
#include "SceneManager.h"
#include "Sound/Sound.h"

#pragma comment(lib, "winmm.lib")

// プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// ウィンドウクラス情報の登録
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex)); // 変数の中身を0で初期化
	wcex.hInstance = hInstance;
	wcex.lpszClassName = "Class Name";
	wcex.lpfnWndProc = WndProc;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm = wcex.hIcon;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, "Failed RegisterClassEx", "Error", MB_OK);
		return 0;
	}

	// ウィンドウの作成
	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		wcex.lpszClassName,
		APP_TITLE,
		WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		HWND_DESKTOP, NULL, hInstance, NULL
	);
	if (hWnd == NULL)
	{
		MessageBox(NULL, "Failed CreateWindow", "Error", MB_OK);
		return 0;
	}

	// ウィンドウの表示
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// DirectXの初期化
	if (!InitDirectX(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT, FALSE))
	{
		MessageBox(NULL, "Failed InitDirectX", "Error", MB_OK);
		return 0;
	}

	InitSound();

	// SpriteDrawerの初期化
	InitSpriteDrawer(GetDevice(), GetContext(), SCREEN_WIDTH, SCREEN_HEIGHT);

	// シーンの初期化
	if (!InitSceneManager()) {
		MessageBox(NULL, "Failed InitScene", "Error", MB_OK);
		return 0;
	}

	// FPS制御
	timeBeginPeriod(1);
	DWORD oldTime = timeGetTime();
	DWORD nowTime = oldTime;

	// メッセージループ
	MSG message;
	while (1)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&message, NULL, 0, 0))
			{
				break;
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		else
		{
			nowTime = timeGetTime();
			if (nowTime - oldTime >= 1000 / FPS)
			{
				UpdateKeyboard();

				UpdateSceneManager();
				BeginDraw();
				DrawSceneManager();
				EndDraw();

				oldTime = nowTime;
			}
		}
	}

	// 終了処理
	timeEndPeriod(1);
	UninitSound();
	UnInitSceneManager();
	UninitSpriteDrawer();
	UninitDirectX();
	UnregisterClass(wcex.lpszClassName, hInstance);


	return 0;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CLOSE: // 「✖」ボタンを押した時に通知されるメッセージ
		int btn;
		btn = MessageBox(NULL, "終了しますか？", "確認", MB_YESNO);
		if (btn == IDNO) // 「いいえ」ボタンが押されたかB
		{
			return 0; // 画面を閉じるのをキャンセル
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}