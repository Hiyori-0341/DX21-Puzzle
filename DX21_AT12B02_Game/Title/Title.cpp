#include "Title.h"
#include "TitleButton.h"
#include "TitleLogo.h"
#include "../Sound/Sound.h"

TitleButton* g_pTitleButton;
TitleLogo* g_pTitleLogo;

bool InitTitle()
{
	InitSound();

	g_pTitleButton = new TitleButton();
	g_pTitleLogo = new TitleLogo();

	return true;
}

void DrawTitle()
{
	g_pTitleButton->Draw();
	g_pTitleLogo->Draw();
}

void UpdateTitle()
{
	g_pTitleButton->Update();
	g_pTitleLogo->Update();
}

void UninitTitle()
{
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
