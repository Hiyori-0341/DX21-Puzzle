#include "Score.h"
#include "Number.h"
#include "Defines.h"

Score::Score()
	: m_pNumber(nullptr)
	, m_score(0)
	, m_displayScore(0)
{
	m_pNumber = new Number();
}

Score::~Score()
{
	delete m_pNumber;
	m_pNumber = nullptr;
}

void Score::Add(int amount)
{
	m_score += amount;
}

int Score::GetScore() const
{
	return m_score;
}

void Score::Draw() const
{
	//常にSCORE_DIGIT_NUM桁を0埋めで表示する(桁数が変わってガタつくのを防ぐ)
	m_pNumber->Draw(m_displayScore, SCORE_POS_X, SCORE_POS_Y, DIGIT_WIDTH, DIGIT_HEIGHT, SCORE_DIGIT_NUM);
}

void Score::Update()
{
	int diff = m_score - m_displayScore;
	if (diff <= 0)	return;
	//表示中のスコアを徐々に増やす
	int step = diff / 10 + 1;	//差の1/10を増やす(最低でも1は増やす)
	if (step < 1)	step = 1;

	m_displayScore += step;

	if(m_displayScore > m_score)
	{
		m_displayScore = m_score;
	}
}
