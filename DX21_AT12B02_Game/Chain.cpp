#include "Chain.h"
#include "Number.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Defines.h"

Chain::Chain()
	: m_pNumber(nullptr)
	, m_pLabelVtx(nullptr)
	, m_pLabelTex(nullptr)
	, m_count(0)
	, m_bacePos{}
	, m_animeFrame(0)
{
	m_pNumber = new Number();

	float w = CHAIN_LABEL_WIDTH / 2;
	float h = CHAIN_LABEL_HEIGHT / 2;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} },
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} },
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }
	};
	m_pLabelVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	const char* texture = "Image/RensaLabel.png";	//用意したファイル名・パスに合わせて変更してください
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pLabelTex);
	if (FAILED(hr))
	{
		MessageBox(NULL, texture, "れんさテクスチャの読み込みに失敗", MB_OK);
	}
}

Chain::~Chain()
{
	if(m_pNumber){	delete m_pNumber;	m_pNumber = nullptr; }
	if (m_pLabelTex){	m_pLabelTex->Release();	m_pLabelTex = nullptr; }
	if (m_pLabelVtx){	m_pLabelVtx->Release(); m_pLabelVtx = nullptr; }
}

void Chain::Reset()
{
	m_count = 0;
}

void Chain::Add(float2 pos)
{
	++m_count;
	m_bacePos = pos;
	m_animeFrame = 0;
}

void Chain::Update()
{
	if (m_animeFrame < FPS)
	{
				++m_animeFrame;
	}

}

int Chain::GetCount() const
{
	return m_count;
}

void Chain::Draw() const
{
	//連鎖数が0のときは描画しない
	if (m_count <= 0 || m_animeFrame >= FPS)
		return;

	//点滅アニメーションのため、フレーム数が奇数のときは描画しない
	if((m_animeFrame / CHAIN_FLASH_INTERVAL) %2 != 0)
		return;	

	//上に移動
	float2 pos = m_bacePos;
	pos.y -= m_animeFrame * CHAIN_MOVE_SPEED;

	//「数字 + れんさ」全体がm_posを中心にくるよう、数字の一の位の位置を逆算する
	float numWidth = m_pNumber->GetWidth(m_count);
	float totalWidth = numWidth + CHAIN_LABEL_WIDTH;

	float numberRightX = pos.x - totalWidth * 0.5f + numWidth;
	float labelCenterX = numberRightX + CHAIN_LABEL_WIDTH * 0.5f;

	m_pNumber->Draw(m_count, numberRightX, pos.y, DIGIT_WIDTH, DIGIT_HEIGHT);

	SetSpritePos(labelCenterX + 10.0f, pos.y);
	SetSpriteScale(1.0f, 1.0f);
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetSpriteTexture(m_pLabelTex);
	DrawSprite(m_pLabelVtx);
}