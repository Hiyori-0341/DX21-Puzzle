#pragma once
#include "DirectX.h"

class Number;

//============================================================
// Score
//
// スコアの加算・保持と、数字での表示を担当する。
// Fieldは消去が起きるたびAdd(amount)を呼ぶだけでよい。
//============================================================
class Score
{
public:
	Score();
	~Score();

	//スコアを加算する
	void Add(int amount);

	void Draw() const;
	void Update();

	int GetScore() const;

private:
	Number* m_pNumber;

	int m_score;
	int m_displayScore;	//表示中のスコア
};