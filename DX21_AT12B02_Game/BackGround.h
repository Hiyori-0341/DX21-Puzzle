#pragma once
#include "DirectX.h"

class BackGround
{
public:
	BackGround();
	~BackGround();
	void Update();
	void Draw();

private:
	ID3D11Buffer* m_pBuffer;
	ID3D11ShaderResourceView* m_pTexture;
};