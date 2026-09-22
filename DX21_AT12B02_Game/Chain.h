#pragma once
#include "DirectX.h"
#include "SpriteDrawer.h"

class Number;

//============================================================
// Chain
//
// 連鎖数の管理と、「数字+れんさラベル」の表示を担当する。
// Fieldは消去が起きるたびAdd()を、ペアが着地するたびReset()を呼ぶだけでよい。
//============================================================
class Chain
{
public:
	Chain();
	~Chain();

	//ペアが着地したとき呼ぶ(連鎖数を0に戻す)
	void Reset();

	//消去が確定したとき呼ぶ。連鎖数を1増やし、表示位置を更新する
	//pos: 消したブロックの中心座標(表示位置に使う)
	void Add(float2 pos);
	void Update();
	void Draw() const;

	int GetCount() const;

private:
	Number* m_pNumber;

	//「れんさ」ラベルの表示に使う頂点バッファ・テクスチャ
	ID3D11Buffer* m_pLabelVtx;
	ID3D11ShaderResourceView* m_pLabelTex;

	int    m_count;	//現在の連鎖数
	float2 m_bacePos;	//表示位置の基準座標(消去が起きたときに更新する)
	int m_animeFrame;	//表示アニメーションの進行フレーム
};