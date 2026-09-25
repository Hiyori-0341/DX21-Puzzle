#include "TitleModeSelect.h"
#include "../Defines.h"
#include "../VertexBuffer.h"
#include "../DirectXTex/TextureLoad.h"
#include "../Input/Keyboard.h"

namespace
{
	constexpr float BUTTON_WIDTH  = 216.0f;
	constexpr float BUTTON_HEIGHT =	 36.0f;
	constexpr float BUTTON_GAP	  =  60.0f;
	constexpr int   APPEAR_FRAME  =		20;		//出現アニメーションのフレーム数
}

TitleModeSelect::TitleModeSelect()
	: m_pVtx(nullptr)
	, m_pMarathonTex(nullptr)
	, m_pVersusTex(nullptr)
	, m_state(HIDDEN)
	, m_animeFrame(0)
	, m_cursorIndex(0)
{
	float w = BUTTON_WIDTH * 0.5f;
	float h = BUTTON_HEIGHT * 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} },
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} },
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }
	};
	m_pVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	const char* texturePaths[] = {
		"Image/Title/MarathonButton.png",
		"Image/Title/VersusButton.png"
	};

	for(int i = 0; i < _countof(texturePaths); ++i)
	{
		HRESULT hr = LoadTextureFromFile(GetDevice(), texturePaths[i], (i == 0) ? &m_pMarathonTex : &m_pVersusTex);
		if (FAILED(hr))
		{
			MessageBox(NULL, texturePaths[i], "ボタンテクスチャの読み込みに失敗", MB_OK);
		}
	}
}

TitleModeSelect::~TitleModeSelect()
{
	if (m_pVtx)			{	m_pVtx->Release();		  m_pVtx = nullptr;	}
	if (m_pMarathonTex) {  m_pMarathonTex->Release(); m_pMarathonTex = nullptr; }
	if (m_pVersusTex)	{  m_pVersusTex->Release();   m_pVersusTex = nullptr; }
}

void TitleModeSelect::Show()
{
	m_state = TitleModeSelect::APPEARING;
	m_animeFrame = 0;
	m_cursorIndex = 0;
}

void TitleModeSelect::Update()
{
	switch (m_state)
	{
		case TitleModeSelect::HIDDEN:	
			break;

		case TitleModeSelect::APPEARING:
			++m_animeFrame;
			if (m_animeFrame >= APPEAR_FRAME)
			{
				m_state = TitleModeSelect::SELECTING;
			}
			break;

		case TitleModeSelect::SELECTING:
			//カーソル移動
			if (isKeyTrigger(VK_UP) || isKeyTrigger(VK_DOWN))
			{
				m_cursorIndex = 1 - m_cursorIndex;	//0と1を切り替える
			}

			if (isKeyTrigger(VK_SPACE))
			{
				m_state = TitleModeSelect::DECIDED;
			}
			break;
		case TitleModeSelect::DECIDED:
			break;
	}
}	

void TitleModeSelect::Draw() const
{
	if (m_state == TitleModeSelect::HIDDEN)
		return;

	//フェードインの透明度(APPEARING中は徐々に不透明になる。SELECTING以降は1.0)
	float alpha = 1.0f;
	if (m_state == TitleModeSelect::APPEARING)
	{
		alpha = (float)m_animeFrame / APPEAR_FRAME;
	}

	//マラソン(上)
	bool isMarathonSelected = (m_cursorIndex == 0);
	float marathonAlpha = isMarathonSelected ? alpha : alpha * 0.5f;	//選ばれていない方は少し薄くする
	SetSpriteColor(1.0f, 1.0f, 1.0f, marathonAlpha);
	SetSpriteScale(isMarathonSelected ? 1.1f : 1.0f, isMarathonSelected ? 1.1f : 1.0f);
	SetSpritePos(0.0f, 150.0f - BUTTON_GAP * 0.5f);
	SetSpriteTexture(m_pMarathonTex);
	DrawSprite(m_pVtx);

	//VS(下)
	bool isVersusSelected = (m_cursorIndex == 1);
	float versusAlpha = isVersusSelected ? alpha : alpha * 0.5f;
	SetSpriteColor(1.0f, 1.0f, 1.0f, versusAlpha);
	SetSpriteScale(isVersusSelected ? 1.1f : 1.0f, isVersusSelected ? 1.1f : 1.0f);
	SetSpritePos(0.0f, 150.0f + BUTTON_GAP * 0.5f);
	SetSpriteTexture(m_pVersusTex);
	DrawSprite(m_pVtx);

	//他の描画に影響しないよう戻す
	SetSpriteScale(1.0f, 1.0f);
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
}

bool TitleModeSelect::IsDecided() const
{
	return m_state == TitleModeSelect::DECIDED;
}

GameMode TitleModeSelect::GetSelectedMode() const
{
	return (m_cursorIndex == 0) ? MODE_MARATHON : MODE_MARATHON;
}