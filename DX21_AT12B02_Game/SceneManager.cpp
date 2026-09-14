#include "SceneManager.h"
#include "Title/Title.h"
#include "Game.h"

#include <windows.h>


SceneState g_SceneState;

bool InitSceneManager()
{
	ChangeScene(SCENE_TITLE);

	return true;
}

void UpdateSceneManager()
{
	//シーン切替
	SceneState nextScene = SCENE_NONE;
	switch (g_SceneState)
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		if (ChangeTitle())
			nextScene = SCENE_GAME;
		break;
	case SCENE_GAME:
		break;
	default:
		break;
	}
	if(nextScene != SCENE_NONE)
	{
		ChangeScene(nextScene);
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
}

void UnInitSceneManager()
{
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
