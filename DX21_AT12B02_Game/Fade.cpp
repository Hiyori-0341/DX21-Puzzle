#include "Fade.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Defines.h"

Fade::Fade()
	: m_pBuffer(nullptr),
	m_pTexture(nullptr),
	m_animeFrame(0),
	m_maxFrame(0),
	m_isFadeOut(false)
{
	//頂点バッファ作成
	float w = SCREEN_WIDTH * 0.5f;
	float h = SCREEN_HEIGHT * 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, // 左下
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} }, // 左上
		{ { w, -h, 0.0f}, {1.0f, 0.0f} }, // 右下
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }  // 右上
	};
	m_pBuffer = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	//テクスチャの読み込み
	const char* texture = "Image/Fade.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pTexture);
	if(FAILED(hr))
	{
		MessageBox(NULL, texture, "フェードテクスチャ読み込みに失敗", MB_OK);
	}
}

Fade::~Fade()
{
	if(m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
	if(m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
}

void Fade::Update()
{
	if (!IsFinish())	
		m_animeFrame++;
}

void Fade::Draw()
{
	if (IsFinish())	return;
	float alpha = (float)m_animeFrame / m_maxFrame;
	if (!m_isFadeOut)
	{
		alpha = 1.0f - alpha;
	}

	SetSpritePos(0.0f,0.0f);
	SetSpriteTexture(m_pTexture);
	SetSpriteColor(1.0f, 1.0f, 1.0f, alpha);
	DrawSprite(m_pBuffer);

	//透明度を戻す
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
}


void Fade::Start(float time, bool isOut)
{
	m_animeFrame = 0;
	m_maxFrame = FPS * time;
	m_isFadeOut = isOut;
}

bool Fade::IsFinish()
{
	return m_animeFrame >= m_maxFrame;
}

bool Fade::IsFadeOut()
{
	return m_isFadeOut;
}
