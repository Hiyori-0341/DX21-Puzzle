#include "Game.h"
#include "Block.h"
#include "Sound/Sound.h"
#include "Field.h"


Field* g_pField;

bool InitGame()
{
	g_pField = new Field();
	return true;
}
void UninitGame()
{
	if (g_pField)
	{
		delete g_pField;
		g_pField = nullptr;
	}

}

void UpdateGame()
{
	g_pField->Update();
}

void DrawGame()
{
	g_pField->Draw();
}