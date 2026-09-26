#include "Game.h"
#include "GameStartUI.h"
#include "GameMode.h"
#include "BackGround.h"
#include "Block.h"
#include "Sound/Sound.h"
#include "Field.h"
#include "GameOverLabel.h"
#include "Title/Title.h"

Field* g_pField;
BackGround* g_pBackGround;
GameStartUI* g_pGameStartUI;
GameOverLabel* g_pGameOverLabel;
bool g_isGameOverLabelStarted;
int  g_finalScore;

bool InitGame()
{
	GameMode mode = GetSelectMode();

	g_pGameStartUI = new GameStartUI();
	g_pBackGround = new BackGround();
	
	if(mode == MODE_MARATHON)
	{
		g_pField = new Field(mode);
	}
	else if(mode == MODE_VERSUS)
	{
		g_pField = new Field(mode);
	}

	g_pGameOverLabel = new GameOverLabel();
	g_isGameOverLabelStarted = false;
	g_finalScore = 0;

	return true;
}

void UninitGame()
{
	if(g_pBackGround)
	{
		delete g_pBackGround;
		g_pBackGround = nullptr;
	}
	if (g_pField)
	{
		delete g_pField;
		g_pField = nullptr;
	}
	if(g_pGameStartUI)
	{
		delete g_pGameStartUI;
		g_pGameStartUI = nullptr;
	}
	if(g_pGameOverLabel)
	{
		delete g_pGameOverLabel;
		g_pGameOverLabel = nullptr;
	}
}

void UpdateGame()
{

	if(!g_pGameStartUI->isFinish())
	{
		g_pGameStartUI->Update();
	}
	else
	{
		g_pBackGround->Update(g_pField->GetChainCount());
		g_pField->Update();

		if(g_pField->GetState() == Field::GAMEOVER)
		{
			if (!g_isGameOverLabelStarted)
			{
				g_finalScore = g_pField->GetScore();
				g_pGameOverLabel->Start();
				g_isGameOverLabelStarted = true;
			}
			g_pGameOverLabel->Update();
		}
	}
}

void DrawGame()
{
	g_pBackGround->Draw();
	g_pField->Draw();
	if (!g_pGameStartUI->isFinish())
	{
		g_pGameStartUI->Draw();
	}
	if(g_isGameOverLabelStarted)
	{
		g_pGameOverLabel->Draw();
	}
}

bool ChangeGame()
{
	return g_isGameOverLabelStarted && g_pGameOverLabel->isFinish();
}

int GetFinalScore()
{
	return g_finalScore;
}
