#include "Title.h"
#include "TitleButton.h"
#include "TitleLogo.h"
#include "TitleBackGround.h"
#include "../Sound/Sound.h"

TitleBackGround* g_pTitleBackGround;
TitleButton* g_pTitleButton;
TitleLogo* g_pTitleLogo;

bool InitTitle()
{
	g_pTitleBackGround = new TitleBackGround();
	g_pTitleButton = new TitleButton();
	g_pTitleLogo = new TitleLogo();
	return true;
}

void DrawTitle()
{
	g_pTitleBackGround->Draw();
	g_pTitleButton->Draw();
	g_pTitleLogo->Draw();
}

void UpdateTitle()
{
	g_pTitleBackGround->Update();
	g_pTitleButton->Update();
	g_pTitleLogo->Update();
}

void UninitTitle()
{
	if(g_pTitleBackGround)
	{
		delete g_pTitleBackGround;
		g_pTitleBackGround = nullptr;
	}

	if(g_pTitleButton)
	{
		delete g_pTitleButton;
		g_pTitleButton = nullptr;
	}

	if (g_pTitleLogo)
	{
		delete g_pTitleLogo;
		g_pTitleLogo = nullptr;
	}

}

bool ChangeTitle()
{
	return g_pTitleButton->GetState() == TitleButton::AFTER;
}
 