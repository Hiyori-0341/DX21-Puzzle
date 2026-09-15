#include "TitleLogo.h"
#include "../VertexBuffer.h"
#include "../DirectXTex/TextureLoad.h"

TitleLogo::TitleLogo()
	: m_pBuffer(nullptr)
	, m_pTexture(nullptr)
{
	//頂点バッファの作成
	float w = TITLE_LOGO_WIDTH * 0.5f;
	float h = TITLE_LOGO_HEIGHT * 0.5f;

	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, // 左下
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} }, // 左上
		{ { w, -h, 0.0f}, {1.0f, 0.0f} }, // 右下
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }  // 右上
	};
	m_pBuffer = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	//テクスチャの読み込み
	const char* texture = "Image/Title/TitleLogo.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pTexture);
	if(FAILED(hr))
	{
		MessageBox(NULL, texture, "タイトルロゴテクスチャ読み込みに失敗", MB_OK);
	}
}

TitleLogo::~TitleLogo()
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

void TitleLogo::Update()
{

}

void TitleLogo::Draw()
{
	SetSpritePos(0.0f, -150.0f);
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pBuffer);
}
