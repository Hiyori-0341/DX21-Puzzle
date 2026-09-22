#include "Number.h"
#include "DirectXTex/TextureLoad.h"
#include "VertexBuffer.h"
#include "Defines.h"

Number::Number()
	: m_pVtx(nullptr)
	, m_pTex(nullptr)
	, m_digitWidth(0.0f)
	, m_digitHeight(0.0f)
{
	float w = 0.5f;
	float h = 0.5f;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} },
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} },
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }
	};
	m_pVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	HRESULT hr = LoadTextureFromFile(GetDevice(), "Image/Number.png", &m_pTex);
	if (FAILED(hr))
	{
		MessageBox(NULL, "数字テクスチャの読み込みに失敗.", "Error", MB_OK);
	}
}

Number::~Number()
{
	if(m_pTex)
	{
		m_pTex->Release();
		m_pTex = nullptr;
	}
	if(m_pVtx)
	{
		m_pVtx->Release();
		m_pVtx = nullptr;
	}
}

void Number::Draw(int val, float x, float y, float w, float h,int Digits)
{
	if (val < 0)	val = 0;

	SetDigitSize(w, h);

	SetSpriteTexture(m_pTex);
	SetSpriteScale(m_digitWidth, m_digitHeight);
	SetSpriteUVScale(1.0f / DIGIT_NUM,1.0f);

	int v = val;
	int digitIdx = 0;

	while(v > 0 || digitIdx < Digits)
	{
		int digit = v % 10;
		v /= 10;

		float posX = x - digitIdx * w;
		SetSpritePos(posX, y);
		SetSpriteUVPos((float)digit / DIGIT_NUM, 0.0f);
		DrawSprite(m_pVtx);

		++digitIdx;
	}

	SetSpriteUVPos(0.0f, 0.0f);
	SetSpriteUVScale(1.0f, 1.0f);
}

void Number::SetDigitSize(float w, float h)
{
	m_digitHeight = h;
	m_digitWidth = w;
}

float Number::GetWidth(int val, int digits) const
{
	if (val < 0)	val = 0;

	int digitCount = 1;
	int v = val / 10;
	while (v > 0)
	{
		++digitCount;
		v /= 10;
	}
	if(digitCount < digits)
	{
		digitCount = digits;
	}

	return digitCount * m_digitWidth;
}
