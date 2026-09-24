#pragma once
#include "DirectX.h"

class BackGround
{
public:
	BackGround();
	~BackGround();
	void Update(int chainCount);
	void Draw();

private:
	//1枚のレイヤーの情報
	struct Layer
	{
		ID3D11ShaderResourceView* pTexture;
		float scrollSpeed;	//1フレームあたりのUV移動量(0なら静止したまま)
		float scrollX;		//現在のUVオフセット(0.0~1.0でループ)
	};

	static constexpr int LAYER_NUM = 3;	//レイヤー数

	ID3D11Buffer* m_pBuffer;	//全レイヤー共通の頂点バッファ
	Layer m_layers[LAYER_NUM];	//奥から手前の順に並べる

	//連鎖フラッシュ演出用
	float m_flashAlpha;
	ID3D11ShaderResourceView* m_pWhiteTexture;
};