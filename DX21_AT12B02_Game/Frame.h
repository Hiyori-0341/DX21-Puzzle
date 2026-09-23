#pragma once
#include "DirectX.h"
#include "SpriteDrawer.h"

//============================================================
// Frame
//
// 指定した幅・高さ・中心位置に、1枚のテクスチャをそのまま貼り付けるだけの汎用クラス。
// フィールドの外枠・ネクスト表示の外枠など、サイズの違う複数の枠に使い回す。
//============================================================
class Frame
{
public:
	//texturePath: 枠画像のパス
	//width, height: 表示サイズ(ピクセル)
	//pos: 表示の中心座標
	Frame(const char* texturePath, float width, float height, float2 pos);
	~Frame();

	void Draw() const;

private:
	ID3D11Buffer* m_pVtx;
	ID3D11ShaderResourceView* m_pTex;
	float2 m_pos;
};