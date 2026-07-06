#pragma once

#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

bool InitDirectX(HWND hWnd, UINT width, UINT height, BOOL fullscreen);
void UninitDirectX();
void BeginDraw();
void EndDraw();

ID3D11Device* GetDevice();
ID3D11DeviceContext* GetContext();