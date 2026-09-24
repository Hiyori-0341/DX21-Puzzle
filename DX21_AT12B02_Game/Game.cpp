#include "Game.h"
#include "GameStartUI.h"
#include "BackGround.h"
#include "Block.h"
#include "Sound/Sound.h"
#include "Field.h"


Field* g_pField;
BackGround* g_pBackGround;
GameStartUI* g_pGameStartUI;

bool InitGame()
{
	g_pGameStartUI = new GameStartUI();
	g_pBackGround = new BackGround();
	g_pField = new Field();
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

}

bool ChangeGame()
{
	return g_pField->GetState() == Field::GAMEOVER;
}
