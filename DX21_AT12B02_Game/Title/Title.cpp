#include "Title.h"
#include "TitleButton.h"
#include "TitleLogo.h"
#include "TitleBackGround.h"
#include "TitleModeSelect.h"
#include "../Sound/Sound.h"

TitleBackGround* g_pTitleBackGround;
TitleButton* g_pTitleButton;
TitleLogo* g_pTitleLogo;
TitleModeSelect* g_pTitleModeSelect;
GameMode g_selectMode = MODE_MARATHON;

bool InitTitle()
{
	g_pTitleBackGround = new TitleBackGround();
	g_pTitleButton = new TitleButton();
	g_pTitleLogo = new TitleLogo();
	g_pTitleModeSelect = new TitleModeSelect();
	return true;
}

void DrawTitle()
{
	g_pTitleBackGround->Draw();
	g_pTitleButton->Draw();
	g_pTitleLogo->Draw();

	if (g_pTitleButton->GetState() == TitleButton::AFTER)
	{
		g_pTitleModeSelect->Draw();
	}
	else
	{
		g_pTitleModeSelect->Show();
	}
}

void UpdateTitle()
{
	g_pTitleBackGround->Update();
	g_pTitleButton->Update();
	g_pTitleLogo->Update();

	if (g_pTitleButton->GetState() != TitleButton::AFTER)
	{
		TitleButton::State prevState = g_pTitleButton->GetState();
		g_pTitleButton->Update();

		//ボタンの消滅演出が終了した瞬間にモード選択を表示
		if (prevState != TitleButton::AFTER && g_pTitleButton->GetState() == TitleButton::AFTER)
		{
			g_pTitleModeSelect->Show();
		}
	}
	else
	{
		g_pTitleModeSelect->Update();

		//モードが決定したら選択されたモードを保存
		if (g_pTitleModeSelect->IsDecided())
		{
			g_selectMode = g_pTitleModeSelect->GetSelectedMode();
		}
	}
}

void UninitTitle()
{
	if(g_pTitleBackGround)	{	delete g_pTitleBackGround;	g_pTitleBackGround = nullptr; }
	if(g_pTitleButton)		{	delete g_pTitleButton;		g_pTitleButton = nullptr;	  }
	if (g_pTitleLogo)		{	delete g_pTitleLogo;		g_pTitleLogo = nullptr;		  }
	if(g_pTitleModeSelect)	{	delete g_pTitleModeSelect;	g_pTitleModeSelect = nullptr; }
}

bool ChangeTitle()
{
	return g_pTitleModeSelect->IsDecided();
}

GameMode GetSelectMode()
{
	return g_selectMode;
}
 