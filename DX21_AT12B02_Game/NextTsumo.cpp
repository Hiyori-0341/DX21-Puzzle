#include "NextTsumo.h"
#include "Block.h"
#include "Frame.h"

NextTsumo::NextTsumo()
{
	Block::LoadResources();

	m_pBlock[0] = new Block(0);
	m_pBlock[1] = new Block(0);

	m_pBlock[0]->SetPos({ NEXT_POS_X, NEXT_POS_Y + NEXT_SPACING });
	m_pBlock[1]->SetPos({ NEXT_POS_X, NEXT_POS_Y });

	float frameWidth = BLOCK_WIDTH + 10.0f;
	float frameHeight = BLOCK_HEIGHT * 2 + 10.0f;
	m_pFrame = new Frame("Image/UI/NextFrame.png", frameWidth, frameHeight, { NEXT_POS_X, NEXT_POS_Y + BLOCK_HEIGHT * 0.5f });
}

NextTsumo::~NextTsumo()
{
	delete m_pBlock[0];
	delete m_pBlock[1];

	Block::ReleaseResources();
}

void NextTsumo::SetColor(int color1, int color2)
{
	m_pBlock[0]->SetColor(color1);
	m_pBlock[1]->SetColor(color2);
}

void NextTsumo::Draw()
{
	m_pBlock[0]->Draw();
	m_pBlock[1]->Draw();
	m_pFrame->Draw();
}
