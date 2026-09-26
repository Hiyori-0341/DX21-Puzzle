#pragma once
#include "GameMode.h"

bool InitGame();
void UninitGame();
void UpdateGame();
void DrawGame();

bool ChangeGame();
int GetFinalScore();
int GetFinalLevel();
GameMode GetFinalMode();