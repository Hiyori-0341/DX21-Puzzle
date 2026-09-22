#pragma once
#include "Defines.h"

//============================================================
// ColorGenerator
//
// ぷよぷよ本家と同じ考え方で、色の出現に規則性を持たせて生成する。
// ・BLOCK_COLOR_NUM色 × BAG_CYCLE個 を1つの袋に入れてシャッフルし、
//   その袋を使い切るまで順番に引く(一定周期で必ず色が均等になる)
// ・同じ色がMAX_STREAKを超えて連続しないよう、袋の中から入れ替える
//============================================================
class ColorGenerator
{
public:
	ColorGenerator();

	//次の色を1つ取り出す
	int Next();

private:
	//新しい袋を作ってシャッフルする
	void RefillBag();

	static constexpr int BAG_CYCLE = 2;	//1色あたり袋に入れる個数
	static constexpr int BAG_SIZE = BLOCK_COLOR_NUM * BAG_CYCLE;
	static constexpr int MAX_STREAK = 3;	//同じ色が連続してよい最大数

	int m_bag[BAG_SIZE];
	int m_bagIndex;		//袋から何個取り出したか

	int m_lastColor;	//直前に出した色
	int m_streakCount;	//直前の色が何個連続しているか
};