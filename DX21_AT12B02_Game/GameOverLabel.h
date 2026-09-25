#pragma once
#include "DirectX.h"
#include "SpriteDrawer.h"

class GameOverLabel
{
public:
	GameOverLabel();
	~GameOverLabel();

	void Start();
	void Update();
	void Draw()const;

	bool isFinish() const;

private:
	ID3D11Buffer* m_pVtx;
	ID3D11ShaderResourceView* m_pTexture;
	float2 m_pos;

	int m_animeFrame;
	int m_animeStep;
};