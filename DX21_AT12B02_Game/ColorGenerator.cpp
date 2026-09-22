#include "ColorGenerator.h"
#include <cstdlib>
#include <utility>

ColorGenerator::ColorGenerator()
	: m_bagIndex(BAG_SIZE)	//最初はNext()呼び出し時に必ず袋を補充させる
	, m_lastColor(-1)
	, m_streakCount(0)
{
}

void ColorGenerator::RefillBag()
{
	//各色をBAG_CYCLE個ずつ用意する
	int idx = 0;
	for (int color = 0; color < BLOCK_COLOR_NUM; ++color)
	{
		for (int i = 0; i < BAG_CYCLE; ++i)
		{
			m_bag[idx++] = color;
		}
	}

	//シャッフル(Fisher-Yates)
	for (int i = BAG_SIZE - 1; i > 0; --i)
	{
		int j = rand() % (i + 1);
		std::swap(m_bag[i], m_bag[j]);
	}

	m_bagIndex = 0;
}

int ColorGenerator::Next()
{
	if (m_bagIndex >= BAG_SIZE)
	{
		RefillBag();
	}

	int color = m_bag[m_bagIndex];

	//同じ色がMAX_STREAKを超えて連続してしまう場合、
	//袋の残りから違う色を探して今引く位置と入れ替える
	if (color == m_lastColor && m_streakCount >= MAX_STREAK)
	{
		for (int i = m_bagIndex + 1; i < BAG_SIZE; ++i)
		{
			if (m_bag[i] != m_lastColor)
			{
				std::swap(m_bag[m_bagIndex], m_bag[i]);
				color = m_bag[m_bagIndex];
				break;
			}
		}
		//袋の残りが全部同じ色の場合はそのまま(BAG_CYCLE, MAX_STREAKの値次第では起こり得る)
	}

	++m_bagIndex;

	if (color == m_lastColor)
	{
		++m_streakCount;
	}
	else
	{
		m_lastColor = color;
		m_streakCount = 1;
	}

	return color;
}