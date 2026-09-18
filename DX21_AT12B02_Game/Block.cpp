#include "Block.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Input/Keyboard.h"
#include "Field.h"



Block::Block(int color, Field *pField)
	: m_pVtx(nullptr), m_pTexture(nullptr)
	, m_state(Block::State::MOVE)
	, m_color(color)
	, m_pos{ 0.0f,0.0f }
	, m_moveTimer(0)
	, m_move{0.0f,0.0f}
	, m_pField(pField)
	, m_destroyTimer(0)
{
	float width = BLOCK_WIDTH / 2;
	float height = BLOCK_HEIGHT / 2;
	//頂点座標の作成
	Vertex vtx[4] = {
		{ {-width, -height, 0.0f},  {0.0f, 0.0f} }, // 左下
		{ {-width,  height, 0.0f},  {0.0f, 1.0f} }, // 左上
		{ { width, -height, 0.0f},  {1.0f, 0.0f} }, // 右下
		{ { width,  height, 0.0f},  {1.0f, 1.0f} }  // 右上
	};
	m_pVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));	//_countof()配列の個数を数える

	//テクスチャの読み込み
	const char* texture[BLOCK_COLOR_NUM] = {
		"Image/Block/Block_Red.png",
		"Image/Block/Block_Green.png",
		"Image/Block/Block_Blue.png",
		"Image/Block/Block_Yellow.png"
	};
	// 安全のためインデックス範囲チェック
	int idx = m_color % BLOCK_COLOR_NUM;
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture[idx], &m_pTexture);

	//エラー確認
	if (FAILED(hr)) { MessageBox(NULL, texture[idx], "Load Error", MB_OK); };
}

Block::~Block()
{
	if(m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}

	if (m_pVtx)
	{
		m_pVtx->Release();
		m_pVtx = nullptr;
	}
}

void Block::Update()
{
	switch (m_state)
	{
	case Block::State::IDLE:	UpdateIdle();		break;
	case Block::State::MOVE:	UpdateMove();		break;
	case Block::State::FALL:	UpdateFall();		break;
	case Block::State::DESTROY:	UpdateDestroy();	break;
	}
}

void Block::Draw()
{
	//削除ステートなら実行
	if (m_state == Block::DESTROY)
	{
		//点滅アニメーション
		if (m_destroyTimer < BLOCK_DESTROY_FLASH_FRAME)
		{
			if (m_destroyTimer / BLOCK_DESTROY_FLASH_INTERVAL % 2 == 0)
				SetSpriteScale(0.0f, 0.0f);
			else
				SetSpriteScale(1.0f, 1.0f);
		}
		//点滅フレーム後は爆発風の拡大
		else
		{
			SetSpriteScale(BLOCK_DESTROY_BOMB_SCALE, BLOCK_DESTROY_BOMB_SCALE);
		}
	}
	else
	{
		SetSpriteScale(1.0f, 1.0f);
	}

	//表示位置の変更
	SetSpritePos(m_pos.x, m_pos.y);
	//テクスチャを貼り付けて表示
	SetSpriteTexture(m_pTexture);
	DrawSprite(m_pVtx);
}

void Block::ResetFallSpeed()
{
	m_move.y = 0.0f;
}

void Block::SetState(State state)
{
	m_state = state;
}

Block :: State Block::GetState()
{
	return m_state;
}

void Block::SetPos(float x, float y)
{
	m_pos.x = x;
	m_pos.y = y;
}

void Block::SetPos(float2 pos)
{
	m_pos = pos;
}

float2 Block::GetPos()
{
	return m_pos;
}

int Block::GetColor()
{
	return m_color;
}

void Block::UpdateIdle()
{
}

void Block::UpdateMove()
{
	if (isKeyRepeat(VK_LEFT) || isKeyTrigger(VK_LEFT))
	{
		if (m_pField) m_pField->MoveHorizontal(-1);
	}
	if (isKeyRepeat(VK_RIGHT) || isKeyTrigger(VK_RIGHT))
	{
		if (m_pField) m_pField->MoveHorizontal(1);
	}
	if (isKeyTrigger(VK_UP))
	{
		if (m_pField)
		{
			m_pField->HardDrop();
		}
	}
	if (isKeyTrigger(VK_DOWN))
	{
		if(m_pField)
		{
			m_pField->SoftDrop();
		}
	}

	//落下処理
	m_moveTimer++;

	if (m_moveTimer > BLOCK_MOVE_WAIT_TIME)
	{
		m_pos.y += BLOCK_HEIGHT;
		m_moveTimer = 0;
	}
}

void Block::UpdateFall()
{
	//重力によって移動速度が徐々に増加
	m_move.y += 0.8f;

	//増加した速度分だけ移動
	m_pos.y += m_move.y;

}

void Block::UpdateDestroy()
{
	//アニメーションタイマーの更新
	++m_destroyTimer;

	//アニメーションが終了したら状態をERASEに変更
	if(m_destroyTimer > BLOCK_DESTROY_TOTAL_FRAME)
	{
		m_state = State::ERASE;
	}
}


