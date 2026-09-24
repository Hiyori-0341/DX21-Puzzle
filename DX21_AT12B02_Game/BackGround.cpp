#define NOMINMAX

#include "BackGround.h"
#include "Defines.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include <algorithm>

namespace
{
	//レイヤーごとの設定。配列の並び=奥から手前の描画順
	struct LayerSetting
	{
		const char* texturePath;
		float scrollSpeed;
	};

	constexpr LayerSetting LAYER_SETTINGS[] =
	{
		{ "Image/Field/BackGround.png",		0.0003f },	//一番奥:ゆっくり流れる空
		{ "Image/Title/PushStart.",			0.0008f },	//中間:少し速く流れる雲
		{ "Image/Title/PushStart.",			0.0f    },	//手前:静止した近景(スクロール速度0)
	};
}

BackGround::BackGround()
	: m_pBuffer(nullptr)
	, m_layers{}
	, m_flashAlpha(0.0f)
	, m_pWhiteTexture(nullptr)
{
	//全レイヤー共通、画面いっぱいの矩形
	float w = SCREEN_WIDTH * 0.5f;
	float h = SCREEN_HEIGHT * 0.5f;
	Vertex vtx[] = {
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} },
		{ { w,  h, 0.0f}, {1.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} }
	};
	m_pBuffer = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	//配列を回して、レイヤーごとにテクスチャを読み込む
	for (int i = 0; i < LAYER_NUM; ++i)
	{
		m_layers[i].scrollSpeed = LAYER_SETTINGS[i].scrollSpeed;
		m_layers[i].scrollX = 0.0f;

		HRESULT hr = LoadTextureFromFile(GetDevice(), LAYER_SETTINGS[i].texturePath, &m_layers[i].pTexture);
		if (FAILED(hr))
		{
			MessageBox(NULL, LAYER_SETTINGS[i].texturePath, "背景テクスチャ読み込みに失敗", MB_OK);
		}
	}

	LoadTextureFromFile(GetDevice(), "Image/White.png", &m_pWhiteTexture);
}

BackGround::~BackGround()
{
	for (int i = 0; i < LAYER_NUM; ++i)
	{
		if (m_layers[i].pTexture)
		{
			m_layers[i].pTexture->Release();
			m_layers[i].pTexture = nullptr;
		}
	}

	if (m_pWhiteTexture)
	{
		m_pWhiteTexture->Release();
		m_pWhiteTexture = nullptr;
	}

	if (m_pBuffer)
	{
		m_pBuffer->Release();
		m_pBuffer = nullptr;
	}
}

void BackGround::Update(int chainCount)
{
	//レイヤーごとに、設定された速度でUVを流す(速度0のレイヤーは動かない)
	for (int i = 0; i < LAYER_NUM; ++i)
	{
		m_layers[i].scrollX += m_layers[i].scrollSpeed;
		if (m_layers[i].scrollX > 1.0f)
		{
			m_layers[i].scrollX -= 1.0f;
		}
	}

	//連鎖数に応じたフラッシュ
	if (chainCount > 0)
		m_flashAlpha = std::min(chainCount * 0.1f, 1.0f);
	else if (m_flashAlpha > 0.0f)
		m_flashAlpha -= 0.05f;
}

void BackGround::Draw()
{
	SetSpritePos(0.0f, 0.0f);
	SetSpriteScale(1.0f, 1.0f);
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);

	//配列を奥から順(0番目→最後)に重ねて描く。同じ頂点バッファを使い回す
	for (int i = 0; i < LAYER_NUM; ++i)
	{
		SetSpriteTexture(m_layers[i].pTexture);
		SetSpriteUVPos(m_layers[i].scrollX,0.0f);
		DrawSprite(m_pBuffer);
	}
	SetSpriteUVPos(0.0f, 0.0f);	//他の描画に影響しないよう戻す

	if (m_flashAlpha > 0.0f)
	{
		SetSpriteColor(1.0f, 1.0f, 1.0f, m_flashAlpha);
		SetSpriteTexture(m_pWhiteTexture);
		DrawSprite(m_pBuffer);
		SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}