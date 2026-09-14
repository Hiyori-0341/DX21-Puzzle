
#include "../Defines.h"
#include "TitleButton.h"
#include "../VertexBuffer.h"
#include "../DirectXTex/TextureLoad.h"
#include "../Input/Keyboard.h"
#include <math.h>

TitleButton::TitleButton()
	: m_pBuffer(nullptr)
	, m_pTexture(nullptr)
	, m_state(TitleButton::BEFORE)
	, m_animeFrame(0)
{
	//頂点バッファ作成
	float w = TITLE_BUTTON_WIDTH * 0.5f;
	float h = TITLE_BUTTON_HEIGHT * 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, // 左下
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} }, // 左上
		{ { w, -h, 0.0f}, {1.0f, 0.0f} }, // 右下
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }  // 右上
	};
	m_pBuffer = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	//テクスチャの読み込み
	const char* texture = "Image/Title/PushStart.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pTexture);
	if (FAILED(hr))
	{
		MessageBox(NULL, texture, "タイトルボタンテクスチャ読み込みに失敗", MB_OK);
	}
}

TitleButton::~TitleButton()
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

bool TitleButton::Init()
{
	return true;
}

void TitleButton::Update()
{
	//アニメーションの更新
	++m_animeFrame;

	//ステートの更新
	switch (m_state)
	{
	case TitleButton::BEFORE:
		if (isKeyTrigger(VK_SPACE))
		{
			m_state = TitleButton::ENTER;
			m_animeFrame = 0;
		}
		break;
	case TitleButton::ENTER:
		if (m_animeFrame > 60.0f)
		{
			m_state = TitleButton::AFTER;
		}
		break;
	case TitleButton::AFTER:	break;
	}
}

void TitleButton::Draw()
{
	switch (m_state)
	{
	case TitleButton::BEFORE:	DrawBefore();	break;
	case TitleButton::ENTER:	DrawEnter();	break;
	case TitleButton::AFTER:	DrawAfter();	break;
	}

	//透明度拡大度を戻す
	SetSpriteScale(1.0f, 1.0f);
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void TitleButton::DrawFlash(float cycleTime)
{
	//周期の計算
	int		 cycleFrame		= cycleTime * FPS;				//秒数からフレームに変換
	float	 cycleOneStep	= 360.0f / cycleFrame;			//1フレームで進む周期の量
	float	 cycleValue		= cycleOneStep * m_animeFrame;		//残材の周期量

	//透明度の計算
	float alpha = sinf(cycleValue / 180 * PI);
	alpha		= alpha * 0.5f + 0.5f;	//0.5～1.0の値に変換
	SetSpriteColor(1.0f, 1.0f, 1.0f, alpha);

	SetSpritePos(0.0f, 150.0f);
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pBuffer);
}

void TitleButton::DrawBefore()
{
	DrawFlash(2.0f);
}

void TitleButton::DrawEnter()
{
	//点滅表示
	DrawFlash(1.0f);

	//パラメータ計算
	float enterMaxScale = 1.5f;							//最大拡大率
	float enterScaleTime = 0.3f;						//拡大までの時間
	int	  enterScalingFrame = enterScaleTime * FPS;		//現在の経過フレームと指定されたフレーム数から変化の割合を計算

	float enterScalingRate = (float)m_animeFrame / enterScalingFrame;

	float scale = 1.0f + enterScalingRate * enterMaxScale;
	SetSpriteScale(scale, scale);

	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f - enterScalingRate);

	SetSpritePos(0.0f, 150.0f);
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pBuffer);
}

void TitleButton::DrawAfter()
{
	DrawFlash(1.0f);
}

TitleButton::State TitleButton::GetState() const
{
	return m_state;
}