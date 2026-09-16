#pragma once
#include "../DirectX.h"

class TitleBackGround
{
public:
	TitleBackGround();
	~TitleBackGround();
	void Update();
	void Draw();

private:
	//テクスチャ
	ID3D11Buffer* m_pBuffer;
	ID3D11ShaderResourceView* m_pTexture;

};