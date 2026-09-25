#pragma once
#include "../DirectX.h"
#include "../SpriteDrawer.h"
#include "../GameMode.h"

class TitleModeSelect
{
public:
	enum State
	{
		HIDDEN,
		APPEARING,
		SELECTING,
		DECIDED
	};
public:
	TitleModeSelect();
	~TitleModeSelect();

	void Show();
	void Update();
	void Draw()const;

	bool IsDecided()const;
	GameMode GetSelectedMode()const;

private:
	ID3D11Buffer* m_pVtx;

	ID3D11ShaderResourceView* m_pMarathonTex;
	ID3D11ShaderResourceView* m_pVersusTex;

	State m_state;
	int m_animeFrame;
	int m_cursorIndex;
};
