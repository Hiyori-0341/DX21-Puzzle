#include "SceneManager.h"
#include "Title/Title.h"
#include "Game.h"
#include "Fade.h"

#include <windows.h>

Fade* g_pFade;
SceneState g_SceneState;
SceneState g_NextSceneState;

bool InitSceneManager()
{
	g_pFade = new Fade();
	ChangeScene(SCENE_TITLE);


	return true;
}

void UpdateSceneManager()
{
	g_pFade->Update();

	//シーン切替
	//フェードが終わったら切替を行う
	if (g_pFade->IsFinish())
	{
		//フェードアウトで終了していれば次のシーンへ
		if (g_pFade->IsFadeOut())
		{
			ChangeScene(g_NextSceneState);

			g_NextSceneState = SCENE_NONE;

		}
		else
		{
			switch (g_SceneState)
			{
			case SCENE_NONE:
				break;
			case SCENE_TITLE:
				if (ChangeTitle())
					g_NextSceneState = SCENE_GAME;
				break;
			case SCENE_GAME:
				if(ChangeGame())
					g_NextSceneState = SCENE_TITLE;
				break;
			default:
				break;
			}

			if (g_NextSceneState != SCENE_NONE)
			{
				g_pFade->Start(2.0f, true);	//フェードアウト開始
			}
		}
	}

	switch (g_SceneState)
	{
		case SCENE_NONE:
			break;
		case SCENE_TITLE:
			UpdateTitle();
			break;
		case SCENE_GAME:
			UpdateGame();
			break;
	}


}

void DrawSceneManager()
{
	switch (g_SceneState)
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		DrawTitle();
		break;
	case SCENE_GAME:
		DrawGame();
		break;
	}

	g_pFade->Draw();

}

void UnInitSceneManager()
{
	if(g_pFade)
	{
		delete g_pFade;
		g_pFade = nullptr;
	}

	switch (g_SceneState)
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		UninitTitle();
		break;
	case SCENE_GAME:
		UninitGame();
		break;
	default:
		break;
	}
}

void ChangeScene(SceneState scene)
{
	//フェードインの開始
	g_pFade->Start(2.0f, false);

	switch (g_SceneState)
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		UninitTitle();
		break;
	case SCENE_GAME:
		UninitGame();
		break;
	default:
		break;
	}

	g_SceneState = scene;

	bool result = false;
	switch (g_SceneState)
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		InitTitle();	result = true;
		break;
	case SCENE_GAME:
		InitGame();		result = true;
		break;
	default:
		break;
	}
	if (!result)
	{
		MessageBox(NULL, "切替先シーンの初期化に失敗", "Error", MB_OK);
	}
}
