#include "GameOverLabel.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Defines.h"

namespace
{
	constexpr float BANNER_WIDTH = 640.0f;
	constexpr float BANNER_HEIGHT = 460.0f;
	constexpr float START_Y = -(SCREEN_HEIGHT * 0.5f + BANNER_HEIGHT);
	constexpr float TARGET_Y = 0.0f;
	constexpr int   FALL_FRAME = 30;
	constexpr int   HOLD_FRAME = 60;
}

GameOverLabel::GameOverLabel()
	: m_pVtx(nullptr)
	, m_pTexture(nullptr)
	, m_pos{ 0.0f, START_Y }
	, m_animeFrame(0)
	, m_animeStep(0)
{
	float w = BANNER_WIDTH * 0.5f;
	float h = BANNER_HEIGHT * 0.5f;
	Vertex vtx[4] = {
		{ {-w, -h, 0.0f},  {0.0f, 0.0f} }, // 左下
		{ {-w,  h, 0.0f},  {0.0f, 1.0f} }, // 左上
		{ { w, -h, 0.0f},  {1.0f, 0.0f} }, // 右下
		{ { w,  h, 0.0f},  {1.0f, 1.0f} }  // 右上
	};
	m_pVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	const char* texture = "Image/GameOver/GameOverLabel.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pTexture);
	if (FAILED(hr))  MessageBox(NULL, texture, "ゲームオーバー画像の読み込みに失敗", MB_OK); 
}

GameOverLabel::~GameOverLabel()
{
	if(m_pVtx)
	{
		m_pVtx->Release();
		m_pVtx = nullptr;
	}

	if(m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
}

void GameOverLabel::Start()
{
	m_pos.y = START_Y;
	m_animeFrame = 0;
	m_animeStep = 0;
}

void GameOverLabel::Update()
{
	if(m_animeStep < 0)
		return;
	++m_animeFrame;

	switch (m_animeStep)
	{
		case 0: // 落下
		{
			m_pos.y += (TARGET_Y - START_Y) / FALL_FRAME;
			if (m_animeFrame >= FALL_FRAME)
			{
				m_pos.y = TARGET_Y;
				m_animeFrame = 0;
				m_animeStep = 1;
			}
			break;
		}
		case 1: // 停止
		{
			if (m_animeFrame >= HOLD_FRAME)
			{
				m_animeStep = 2; // アニメーション終了
			}
			break;
		}
		case 2: // アニメーション終了
		{
			{
				m_animeStep = -1; // アニメーション終了
				break;
			}
		}
	}
}

void GameOverLabel::Draw() const
{
	if (m_animeStep < 0)
		return;
	SetSpritePos(m_pos.x, m_pos.y);
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pVtx);
}

bool GameOverLabel::isFinish() const
{
	return m_animeStep >= 2;
}