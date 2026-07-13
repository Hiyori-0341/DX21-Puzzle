#pragma once

#include "DirectX.h"
#include "SpriteDrawer.h"

constexpr float BLOCK_WIDTH  = 40.0f;			 //ブロックの横幅
constexpr float BLOCK_HEIGHT = 40.0f;			 //ブロックの縦幅
constexpr   int BLOCK_MOVE_WAIT_TIME = 60;		 //ブロックの落下移動までの待ち時間

class Field;

class Block
{
public:
	//ブロックの状態を表す列挙子
	enum State
	{
		IDLE,									 //待機中(ブロックが積まれている状態)
		MOVE,									 //移動中(キー入力で左右移動)
		FALL,									 //落下中(下キー入力と下のブロックが消えたとき)
		DESTROY,								 //ブロックが消えるとき
		ERASE,									 //ブロックが消えた後の処理
	};

private:
	//表示に使用する変数
	ID3D11Buffer*			  m_pVtx;			 //頂点バッファ
	ID3D11ShaderResourceView* m_pTexture;		 //テクスチャ

	//ブロックの処理に必要な変数				    
	State  m_state;								 //現在のブロックの状態
	int    m_color;								 //ブロックの色
	float2 m_pos;								 //座標

	Field* m_pField;					//ブロックが配置されているフィールドのポインタ

public:
	//基本の処理
	Block(int color, Field *pField);
	~Block();
	void Update();
	void Draw();

	void SetState(State state);
	State GetState();

	void SetPos(float x, float y);
	void SetPos(float2 pos);
	float2 GetPos();

	//ブロックの色を取得
	int GetColor();

private:
	//ステート別に実行する更新処理
	void UpdateIdle();
	void UpdateMove();
	void UpdateFall();
	void UpdateDestroy();

	//移動に関係する処理
	int m_moveTimer;	//落下までの時間
	float2 m_move;		//移動スピード


	int m_destroyTimer; //消えるアニメーション用のタイマー
};