#include "GameOver.h"
#include "Game.h"				//GetFinalScore()
#include "Number.h"
#include "Input/Keyboard.h"
#include "DirectX.h"
#include "SpriteDrawer.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Defines.h"

namespace
{
	ID3D11Buffer* g_pLabelBuf = nullptr;
	ID3D11ShaderResourceView* g_pLabelTex = nullptr;

	ID3D11Buffer* g_pPromptBuf = nullptr;
	ID3D11ShaderResourceView* g_pPromptTex = nullptr;	//「Enter:リトライ / Esc:タイトルへ」等のボタン案内画像

	ID3D11Buffer* g_pBackGroundBuf = nullptr;
	ID3D11ShaderResourceView* g_pBackGroundTex = nullptr;

	Number* g_pNumber = nullptr;

	bool g_wantsRetry = false;
	bool g_isFinish = false;
}

bool InitGameOver()
{
	//GAME OVERラベル
	float w = 320.0f * 0.5f;
	float h = 230.0f * 0.5f;
	Vertex vtxLabel[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, 
		{ {-w, h, 0.0f}, {0.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} },
		{ { w, h, 0.0f}, {1.0f, 1.0f} }
	};
	g_pLabelBuf = CreateVertexBuffer(GetDevice(), vtxLabel, _countof(vtxLabel));
	LoadTextureFromFile(GetDevice(), "Image/GameOver/GameOverLabel.png", &g_pLabelTex);

	//ボタン案内
	w = 640.0f * 0.5f;
	h = 460.0f * 0.5f;
	Vertex vtxPrompt[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} }, 
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} },
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }
	};
	g_pPromptBuf = CreateVertexBuffer(GetDevice(), vtxPrompt, _countof(vtxPrompt));
	LoadTextureFromFile(GetDevice(), "Image/GameOver/GameOverLabel.png", &g_pPromptTex);

	//背景
	w = SCREEN_WIDTH * 0.5f;
	h = SCREEN_HEIGHT * 0.5f;
	Vertex vtxPrompt[] = {
	{ {-w, -h, 0.0f}, {0.0f, 0.0f} },
	{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
	{ { w, -h, 0.0f}, {1.0f, 0.0f} },
	{ { w,  h, 0.0f}, {1.0f, 1.0f} }
	};
	g_pBackGroundBuf = CreateVertexBuffer(GetDevice(), vtxPrompt, _countof(vtxPrompt));
	LoadTextureFromFile(GetDevice(), "Image/GameOver/GameOverBackGround.png", &g_pBackGroundTex);


	g_pNumber = new Number();
	g_wantsRetry = false;
	g_isFinish = false;
	return true;
}

void UninitGameOver()
{
	if (g_pNumber)			{ delete g_pNumber;			g_pNumber = nullptr;}
	if (g_pPromptTex)		{ g_pPromptTex->Release();  g_pPromptTex = nullptr; }
	if (g_pPromptBuf)		{ g_pPromptBuf->Release();  g_pPromptBuf = nullptr; }
	if (g_pLabelTex)		{ g_pLabelTex->Release();   g_pLabelTex = nullptr; }
	if (g_pLabelBuf)		{ g_pLabelBuf->Release();   g_pLabelBuf = nullptr; }
	if (g_pBackGroundBuf)	{ g_pBackGroundBuf->Release(); g_pBackGroundBuf = nullptr; }
	if (g_pBackGroundTex)	{ g_pBackGroundTex->Release(); g_pBackGroundTex = nullptr; }
}

void UpdateGameOver()
{
	if (isKeyTrigger(VK_RETURN))
	{
		g_wantsRetry = true;
		g_isFinish = true;
	}
	else if (isKeyTrigger(VK_ESCAPE))
	{
		g_wantsRetry = false;
		g_isFinish = true;
	}
}

void DrawGameOver()
{
	SetSpriteScale(1.0f, 1.0f);

	//背景表示
	SetSpritePos(0.0f,0.0f);
	SetSpriteTexture(g_pBackGroundTex);
	DrawSprite(g_pBackGroundBuf);

	//ゲームオーバーラベル
	SetSpritePos(0.0f, -150.0f);
	SetSpriteTexture(g_pLabelTex);
	DrawSprite(g_pLabelBuf);

	//ボタン表示
	SetSpritePos(0.0f, 150.0f);
	SetSpriteTexture(g_pPromptTex);
	DrawSprite(g_pPromptBuf);

	//最終スコア(GetFinalScoreはField破棄後もGame.cpp側に残った値を返す)
	g_pNumber->Draw(GetFinalScore(), 50.0f, 0.0f, 24.0f, 32.0f, 6);


}

bool ChangeGameOver()
{
	return g_isFinish;
}

bool WantsRetry()
{
	return g_wantsRetry;
}