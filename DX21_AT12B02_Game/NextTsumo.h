#pragma once
#include "DirectX.h"
#include "Defines.h"

class Block;
class Frame;

class NextTsumo
{
public:
	NextTsumo();
	~NextTsumo();
	void SetColor(int color1, int color2);

	void Draw();
private:
	constexpr static float NEXT_POS_X = 150.0f;
	constexpr static float NEXT_POS_Y = -200.0f;
	constexpr static float NEXT_SPACING = BLOCK_HEIGHT;

	Block* m_pBlock[2];
	Frame* m_pFrame;
};