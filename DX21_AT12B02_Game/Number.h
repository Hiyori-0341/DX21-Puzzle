#pragma once
#include "DirectX.h"

class Number
{
public:
	Number();
	~Number();

	void Draw(int val, float x, float y, float w,float h,int Digits = 0);

	void SetDigitSize(float w, float h);
	float GetWidth(int val, int digits = 0)const;

private:
	static constexpr int DIGIT_NUM = 10;	// 0`9‚Ì”š‚Ìí—Ş

	ID3D11Buffer* m_pVtx;
	ID3D11ShaderResourceView* m_pTex;

	float m_digitWidth;
	float m_digitHeight;
};