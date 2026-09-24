
#pragma once
#include "Block.h"
#include "Defines.h"
#include "DirectX.h"
#include "SpriteDrawer.h"
#include "Sound/Sound.h"
#include "ColorGenerator.h"

class Chain;
class NextTsumo;
class Frame;
class Score;

//============================================================
// Field
//
// 盤面(m_grid)の管理、操作中の2個組ブロックの移動・回転・落下、
// 消去判定を担当する。
//
// 操作中の2個組(ペア)は、着地するまで盤面(m_grid)には入れず、
// m_pairBlock / m_pairIndex で管理する。
// 生成直後は上側のブロックが盤面の1マス上(y = -1)にいる。
//============================================================
class Field
{
public:
	//処理の状態の定義
	enum State
	{
		CREATE,		//ブロックを生成
		IDLE,		//操作・落下待機
		CHECK,		//ブロックが4つ以上並んでいるか
		DESTROY,	//ブロックの削除
		GAMEOVER	//ゲームオーバー
	};

public:
	Field();
	~Field();

	void Update();
	void Draw();

	State GetState() const;
	int GetChainCount() const;
	int GetScore() const;

private:
	//二次元配列の添え字を示す構造体
	struct Index
	{
		int x;
		int y;
	};

	//ペアの要素番号
	enum PairPart
	{
		PIVOT,		//回転の中心(生成時は下側)
		SUB,		//回転する側(生成時は上側)
		PAIR_NUM
	};

private:
	//ステート別の更新処理
	void UpdateCreate();
	void UpdateIdle();
	void UpdateCheck();
	void UpdateDestroy();

	//盤面上のブロックの更新
	void UpdateBlocks();
	void UpdateFallBlock(int x, int y);
	bool StartFalling();

	//着地の衝撃を列の下方向に伝播させる
	void PropagateSquash(int x, int startY);

	//操作中のペア
	bool HasPair() const;
	void UpdatePair();
	bool CanPlacePair(int dx, int dy) const;
	void MovePair(int dx, int dy);
	void RotatePair(int direction);
	void HardDrop();
	void StepDown();
	void LockPair();
	void ApplyPairPos();

	//消去判定
	int CollectSameColor(int startX, int startY,
	bool visited[FIELD_ROW][FIELD_COLUMN], Index* pOut) const;

	//マス目の判定・座標変換
	bool IsCellFree(int x, int y) const;
	float2 IndexToPos(Index index) const;

private:
	//ブロック生成アルゴリズム
	ColorGenerator m_colorGen;

	//連鎖数の表示
	Chain* m_pChain;

	//ツモ表示
	NextTsumo* m_pNextTsumo;
	int m_nextColor[PAIR_NUM];

	//背景フレームの情報
	ID3D11Buffer* m_pFrameBuf;	//頂点バッファ
	ID3D11ShaderResourceView* m_pFrameTex;	//テクスチャ
	float2 m_offset;						//フレームの表示位置

	//フレーム描画
	Frame* m_pFrame;

	//スコア表示
	Score* m_pScore;

	//フィールドに配置されているブロック
	Block* m_grid[FIELD_ROW][FIELD_COLUMN];

	//現在の状態
	State m_state;

	//操作中のペア(着地するまでm_gridには入れない)
	Block* m_pairBlock[PAIR_NUM];
	Index  m_pairIndex[PAIR_NUM];		//ペアのマス目(生成直後はyが-1になる)
	int    m_fallTimer;					//ペアの自動落下タイマー
	int    m_quickTurnDirection;		//回転できなかった方向(1:右回転 -1:左回転 0:なし)

	//サウンド
	XAUDIO2_BUFFER* m_pBlockDestroySE;
};

