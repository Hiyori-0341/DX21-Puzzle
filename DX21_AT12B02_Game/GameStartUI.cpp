#include "GameStartUI.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Defines.h"


GameStartUI::GameStartUI()
	: m_pFrameBuf(nullptr)
	, m_pFrameTex(nullptr)
	, m_pos{ 0.0f, 0.0f }
	, m_animeFrame(0)
	, m_animeStep(0)
{
	float w = 400.0f * 0.5f;
	float h = 100.0f * 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, // 左下
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} }, // 左上
		{ { w, -h, 0.0f}, {1.0f, 0.0f} }, // 右下
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }  // 右上
	};

	m_pFrameBuf = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	const char* texture = "Image/Title/TitleLogo.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pFrameTex);
	if (FAILED(hr))
	{
		MessageBox(NULL, texture, "ゲームスタートUIテクスチャ読み込みに失敗", MB_OK);
	}

	m_pos.y = -(SCREEN_HEIGHT * 0.5f);
}

GameStartUI::~GameStartUI()
{
	if (m_pFrameTex)
	{
		m_pFrameTex->Release();
		m_pFrameTex = nullptr;
	}
	if (m_pFrameBuf)
	{
		m_pFrameBuf->Release();
		m_pFrameBuf = nullptr;
	}
}

void GameStartUI::Update()
{
	++m_animeFrame;
	bool isNext = false;
	switch (m_animeStep)
	{
		case 0:
			if(m_animeFrame > 60)
				isNext = true;
			break;
		case 1:
			m_pos.y += (SCREEN_HEIGHT * 0.5f) / 30;
			if(m_animeFrame >= 30)
				isNext = true;
			break;
		case 2:
			if(m_animeFrame >= 70)
				isNext = true;
			break;
		case 3:
			m_pos.y += (SCREEN_HEIGHT * 0.5f) / 30;
			if (m_animeFrame >= 30)
				isNext = true;
			break;
		case 4:
			if(m_animeFrame >= 20)
				isNext = true;
			break;
	}

	if (isNext)
	{
		++m_animeStep;
		m_animeFrame = 0;
	}
}

void GameStartUI::Draw()
{
	SetSpritePos(m_pos.x, m_pos.y);
	SetSpriteTexture(m_pFrameTex);
	DrawSprite(m_pFrameBuf);
}

bool GameStartUI::isFinish()
{
	return m_animeStep >= 5;
}
