#pragma once
#include "DirectX.h"

class Fade
{
private:
	ID3D11Buffer* m_pBuffer;
	ID3D11ShaderResourceView* m_pTexture;

	//アニメーション用データ
	int m_animeFrame;		 //現在の経過時間
	int m_maxFrame;			 //最大アニメーション時間
	bool m_isFadeOut;		 //フェードアウトフラグ

public:
	Fade();
	~Fade();
	void Update();
	void Draw();

	//フェードアウト開始
	void Start(float time, bool isOut);

	//終了したかどうか
	bool IsFinish();

	//フェードアウトしたかどうか
	bool IsFadeOut();
};