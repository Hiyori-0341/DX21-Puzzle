#include "Block.h"
#include "Defines.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"

namespace
{
	//落下時の重力加速度(1フレームごとに落下速度へ加算)
	constexpr float FALL_GRAVITY = 0.8f;

	//全ブロックで共有するリソース
	ID3D11Buffer* g_pVtx = nullptr;
	ID3D11ShaderResourceView* g_pTextures[BLOCK_COLOR_NUM] = {};
	int						  g_resourceRefCount = 0;
}


//============================================================
// 共有リソース
//============================================================

void Block::LoadResources()
{
	//2回目以降は参照カウントだけ増やす
	if (g_resourceRefCount++ > 0)
		return;

	//頂点座標の作成
	float width = BLOCK_WIDTH / 2;
	float height = BLOCK_HEIGHT / 2;
	Vertex vtx[4] = {
		{ {-width, -height, 0.0f},  {0.0f, 0.0f} }, // 左下
		{ {-width,  height, 0.0f},  {0.0f, 1.0f} }, // 左上
		{ { width, -height, 0.0f},  {1.0f, 0.0f} }, // 右下
		{ { width,  height, 0.0f},  {1.0f, 1.0f} }  // 右上
	};
	g_pVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	//テクスチャの読み込み
	const char* texture[BLOCK_COLOR_NUM] = {
		"Image/Block/Block_Red.png",
		"Image/Block/Block_Green.png",
		"Image/Block/Block_Blue.png",
		"Image/Block/Block_Yellow.png"
	};
	for (int i = 0; i < BLOCK_COLOR_NUM; ++i)
	{
		HRESULT hr = LoadTextureFromFile(GetDevice(), texture[i], &g_pTextures[i]);
		if (FAILED(hr)) { MessageBox(NULL, texture[i], "Load Error", MB_OK); }
	}
}

void Block::ReleaseResources()
{
	if (g_resourceRefCount == 0)
		return;

	//最後の参照が消えたときだけ解放する
	if (--g_resourceRefCount > 0)
		return;

	for (int i = 0; i < BLOCK_COLOR_NUM; ++i)
	{
		if (g_pTextures[i])
		{
			g_pTextures[i]->Release();
			g_pTextures[i] = nullptr;
		}
	}

	if (g_pVtx)
	{
		g_pVtx->Release();
		g_pVtx = nullptr;
	}
}


//============================================================
// 基本の処理
//============================================================
Block::Block(int color)
	: m_state(Block::MOVE)
	, m_color(color% BLOCK_COLOR_NUM)
	, m_pos{ 0.0f, 0.0f }
	, m_fallSpeed(0.0f)
	, m_destroyTimer(0)
{
}

Block::~Block()
{
}

void Block::Update()
{
	switch (m_state)
	{
	case Block::FALL:		UpdateFall();		break;
	case Block::DESTROY:	UpdateDestroy();	break;
	default:									break;	//IDLE, MOVE, ERASEは何もしない
	}
}

void Block::Draw()
{
	//削除ステートなら点滅→拡大のアニメーション
	if (m_state == Block::DESTROY)
	{
		if (m_destroyTimer < BLOCK_DESTROY_FLASH_FRAME)
		{
			//点滅
			if (m_destroyTimer / BLOCK_DESTROY_FLASH_INTERVAL % 2 == 0)
				SetSpriteScale(0.0f, 0.0f);
			else
				SetSpriteScale(1.0f, 1.0f);
		}
		else
		{
			//点滅フレーム後は爆発風の拡大
			SetSpriteScale(BLOCK_DESTROY_BOMB_SCALE, BLOCK_DESTROY_BOMB_SCALE);
		}
	}
	else
	{
		SetSpriteScale(1.0f, 1.0f);
	}

	SetSpritePos(m_pos.x, m_pos.y);
	SetSpriteTexture(g_pTextures[m_color]);
	DrawSprite(g_pVtx);
}


//============================================================
// 座標・色
//============================================================

void Block::SetPos(float2 pos)
{
	m_pos = pos;
}

float2 Block::GetPos() const
{
	return m_pos;
}

int Block::GetColor() const
{
	return m_color;
}

bool Block::IsSameColor(const Block* pOther) const
{
	return pOther != nullptr && m_color == pOther->m_color;
}

void Block::SetColor(int color)
{
	m_color = color % BLOCK_COLOR_NUM;
}


//============================================================
// 状態
//============================================================

Block::State Block::GetState() const
{
	return m_state;
}

bool Block::IsFalling() const
{
	return m_state == Block::FALL;
}

bool Block::IsDestroying() const
{
	return m_state == Block::DESTROY;
}

bool Block::IsErased() const
{
	return m_state == Block::ERASE;
}

void Block::StartFall()
{
	m_state = Block::FALL;
	m_fallSpeed = 0.0f;
}

void Block::StartDestroy()
{
	m_state = Block::DESTROY;
	m_destroyTimer = 0;
}

void Block::Land(float2 pos)
{
	m_pos = pos;
	m_state = Block::IDLE;
	m_fallSpeed = 0.0f;
}

bool Block::HasReachedY(float y) const
{
	return m_pos.y >= y;
}


//============================================================
// ステート別の更新処理
//============================================================

void Block::UpdateFall()
{
	//重力によって落下速度が徐々に増加し、その分だけ移動
	m_fallSpeed += FALL_GRAVITY;
	m_pos.y += m_fallSpeed;
}

void Block::UpdateDestroy()
{
	++m_destroyTimer;

	//アニメーションが終了したら状態をERASEに変更
	if (m_destroyTimer > BLOCK_DESTROY_TOTAL_FRAME)
	{
		m_state = Block::ERASE;
	}
}