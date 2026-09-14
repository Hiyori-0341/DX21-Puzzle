#include "Title.h"
#include "TitleButton.h"
#include "../Sound/Sound.h"

TitleButton* g_pTitleButton;

bool InitTitle()
{
	InitSound();

	g_pTitleButton = new TitleButton();

	return true;
}

void DrawTitle()
{
	g_pTitleButton->Draw();
}

void UpdateTitle()
{
	g_pTitleButton->Update();
}

void UninitTitle()
{
	if(g_pTitleButton)
	{
		delete g_pTitleButton;
		g_pTitleButton = nullptr;
	}

}

bool ChangeTitle()
{
	return g_pTitleButton->GetState() == TitleButton::AFTER;
}
