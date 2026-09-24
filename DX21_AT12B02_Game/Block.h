#pragma once

#include "DirectX.h"
#include "SpriteDrawer.h"


constexpr   int BLOCK_MOVE_WAIT_TIME = 60;	//ブロックが1マス自動落下するまでの待ち時間(フレーム)

//============================================================
// Block
//
// ブロック1個分の「見た目」と「状態ごとの動き」だけを担当する。
// 盤面上のどのマスにいるか、キー入力、当たり判定はFieldが担当する。
//============================================================
class Block
{
public:
	//ブロックの状態を表す列挙子
	enum State
	{
		IDLE,		//待機中(ブロックが積まれている状態)
		MOVE,		//操作中(Fieldが左右移動・回転を行う)
		FALL,		//落下中(下のブロックが消えたとき)
		DESTROY,	//消えるアニメーション中
		ERASE,		//アニメーション終了(Fieldが削除する)
	};

public:
	//全ブロックで共有する頂点バッファ・テクスチャの読み込み/解放
	//(Fieldのコンストラクタ/デストラクタから呼ぶ。参照カウント式)
	static void LoadResources();
	static void ReleaseResources();

public:
	explicit Block(int color);
	~Block();

	void Update();
	void Draw();

	//座標
	void SetPos(float2 pos);
	float2 GetPos() const;

	//色
	int  GetColor() const;
	bool IsSameColor(const Block* pOther) const;
	void SetColor(int color);

	//状態
	State GetState() const;
	bool IsFalling() const;
	bool IsDestroying() const;
	bool IsErased() const;

	void StartFall();					//落下を開始する(落下速度は0から)
	void StartDestroy();				//消えるアニメーションを開始する
	void Land(float2 pos);				//指定位置に置いて待機状態にする
	void Squash(float intensity);		//着地の衝撃でつぶれる演出


	//落下中の座標がyまで到達したか(Fieldがマスを1つ進めるかの判定に使う)
	bool HasReachedY(float y) const;

private:
	//ステート別に実行する更新処理
	void UpdateFall();
	void UpdateDestroy();

private:
	State  m_state;			//現在のブロックの状態
	int    m_color;			//ブロックの色
	float2 m_pos;			//座標
	float  m_fallSpeed;		//落下速度
	int    m_destroyTimer;	//消えるアニメーション用のタイマー
	float  m_squashIntensity;	//着地の衝撃でつぶれる演出の強さ
};