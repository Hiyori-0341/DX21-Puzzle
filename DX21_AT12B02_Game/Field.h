#pragma once
#include "Block.h"
#include "Defines.h"
#include "DirectX.h"
#include "SpriteDrawer.h"

class Field
{
public:
	//処理の状態の定義
	enum State
	{
		CREATE,		   //ブロックを生成
		IDLE,		   //落下待機
		CHECK,		   //ブロックが４つ並んでいるか
		DESTROY,	   //ブロックの削除
		GAMEOVER	   //ゲームオーバー
	};
private:
	//背景フレームの情報
	ID3D11Buffer* m_pFrameBuf;					  //頂点バッファ
	ID3D11ShaderResourceView* m_pFrameTex;		  //テクスチャ

	//フィールドに配置されているブロックの情報
	Block* m_grid[FIELD_ROW][FIELD_COLUMN];

	//フィールドを区切る枠
	float2 m_offset;	//枠の表示位置

	//現在の状態を定義
	State m_state;

	//再起処理の確認済みフラグ
	bool m_check[FIELD_ROW][FIELD_COLUMN];

	bool m_isMoveRight;	//右に移動しているか
public:
	Field();
	~Field();
	void Update();
	void Draw();

	bool IsMoveRight();
	void SetMoveRight(bool isMoveRight);

private:
	// ステート別の更新処理 
	void UpdateCreate();
	void UpdateIdle();
	void UpdateCheck();
	void UpdateDestroy();
	void UpdateGameOver();

	//二次元配列の添え字を示す構造体
	struct Index
	{
		int x;
		int y;
	};

	float2 IndexToPos(Index index);

	// ゲーム上の座標から配列の添え字に変換 
	Index PosToIndex(float2 pos);

	//再起処理で同じ色のブロックを数える
	int RecursiveBlockCount(Index index);

	//再起処理で同じブロックを削除する
	void RecursiveBlockDestroy(Index index);

	//同期処理
	void syncBlock(int x, int y);
	
	//ブロックを回転させる
	void RotateBlock(int direction);
};