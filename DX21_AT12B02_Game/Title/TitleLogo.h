#pragma once
#include "../DirectX.h"

constexpr float TITLE_LOGO_WIDTH = 500.0f;
constexpr float TITLE_LOGO_HEIGHT = 200.0f;

class TitleLogo
{
public:
	TitleLogo();
	~TitleLogo();
	void Update();
	void Draw();

private:
	//テクスチャ
	ID3D11Buffer* m_pBuffer;
	ID3D11ShaderResourceView* m_pTexture;
};
