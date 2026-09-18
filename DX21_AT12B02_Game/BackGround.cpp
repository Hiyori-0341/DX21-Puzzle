#include "BackGround.h"
#include "Defines.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"

BackGround::BackGround()
	: m_pBuffer(nullptr)
	, m_pTexture(nullptr)
{
	//頂点バッファ作成
	float w = SCREEN_WIDTH * 0.5f;
	float h = SCREEN_HEIGHT * 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 1.0f} }, // 左下
		{ {-w,  h, 0.0f}, {0.0f, 0.0f} }, // 左上
		{ { w, -h, 0.0f}, {1.0f, 1.0f} }, // 右下
		{ { w,  h, 0.0f}, {1.0f, 0.0f} }  // 右上
	};
	m_pBuffer = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));
	//テクスチャの読み込み
	const char* texture = "Image/Title/TitleLogo.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pTexture);
	if (FAILED(hr))
	{
		MessageBox(NULL, texture, "背景テクスチャ読み込みに失敗", MB_OK);
	}
}

BackGround::~BackGround()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
	if (m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
}

void BackGround::Update()
{
	//背景は特に更新することはない
}

void BackGround::Draw()
{
	SetSpritePos(0.0f, 0.0f);
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pBuffer);
}