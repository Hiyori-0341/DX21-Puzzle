#pragma once

enum SceneState
{
	SCENE_NONE,
	SCENE_TITLE,
	SCENE_GAME,
};

bool InitSceneManager();
void UpdateSceneManager();
void DrawSceneManager();
void UnInitSceneManager();
void ChangeScene(SceneState scene);