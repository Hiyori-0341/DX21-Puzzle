#ifndef __TEXTURE_LOAD_H__
#define __TEXTURE_LOAD_H__

#include "DirectXTex.h"

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex/DirectXTexD.lib")
#else
#pragma comment(lib, "DirectXTex/DirectXTex.lib")
#endif

inline HRESULT LoadTextureFromFile(
	ID3D11Device* pDevice,
	const char* pszFileName, ID3D11ShaderResourceView** ppTexture)
{
	HRESULT hr;
	DirectX::TexMetadata mdata;
	DirectX::ScratchImage image;
	wchar_t wPath[MAX_PATH];
	size_t wLen = 0;

	MultiByteToWideChar(0, 0, pszFileName, -1, wPath, MAX_PATH);
	if (strstr(pszFileName, ".tga"))
	{
		hr = DirectX::LoadFromTGAFile(wPath, &mdata, image);
	}
	else
	{
		hr = DirectX::LoadFromWICFile(wPath, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &mdata, image);
	}
	if (FAILED(hr)) {
		return hr;
	}

	hr = CreateShaderResourceView(
		pDevice,
		image.GetImages(), image.GetImageCount(),
		mdata, ppTexture);

	return hr;
}

#endif // __TEXTURE_LOAD_H__