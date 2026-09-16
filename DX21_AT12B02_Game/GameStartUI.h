#pragma once 
#include "DirectX.h"
#include "SpriteDrawer.h"

class GameStartUI
{
private:
	//‰æ‘œ•\Ž¦
	ID3D11Buffer* m_pFrameBuf;
	ID3D11ShaderResourceView* m_pFrameTex;
	float2 m_pos;

	int m_animeFrame;
	int m_animeStep;

public:
	GameStartUI();
	~GameStartUI();
	void Update();
	void Draw();

	bool isFinish();
};