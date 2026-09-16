#include "TitleBackGround.h"
#include "../VertexBuffer.h"
#include "../DirectXTex/TextureLoad.h"
#include "../Defines.h"

TitleBackGround::TitleBackGround()
{
	float w = SCREEN_WIDTH * 0.5f;
	float h = SCREEN_HEIGHT * 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, // 左下
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} }, // 左上
		{ { w, -h, 0.0f}, {1.0f, 0.0f} }, // 右下
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }  // 右上
	};
	m_pBuffer = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));
	const char* texture = "Image/Title/TitleBackGround.png";

	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pTexture);
	if(FAILED(hr))
	{
		MessageBox(NULL, texture, "タイトル背景テクスチャ読み込みに失敗", MB_OK);
	}
}

TitleBackGround::~TitleBackGround()
{
	if(m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
	if(m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
}

void TitleBackGround::Update()
{
}

void TitleBackGround::Draw()
{
	SetSpritePos(0.0f, 0.0f);
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pBuffer);
}