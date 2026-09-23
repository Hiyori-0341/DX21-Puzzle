#include "Frame.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "SpriteDrawer.h"

Frame::Frame(const char* texturePath, float width, float height, float2 pos)
	: m_pVtx(nullptr)
	, m_pTex(nullptr)
	, m_pos(pos)
{
	float w = width / 2;
	float h = height / 2;
	Vertex vtx[] = {
		{ {-w, -h, 0.0f}, {0.0f, 0.0f} },
		{ {-w,  h, 0.0f}, {0.0f, 1.0f} },
		{ { w, -h, 0.0f}, {1.0f, 0.0f} },
		{ { w,  h, 0.0f}, {1.0f, 1.0f} }
	};
	m_pVtx = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	HRESULT hr = LoadTextureFromFile(GetDevice(), texturePath, &m_pTex);
	if (FAILED(hr))
	{
		MessageBox(NULL, texturePath, "˜gƒeƒNƒXƒ`ƒƒ‚Ì“Ç‚Ýž‚Ý‚ÉŽ¸”s", MB_OK);
	}
}

Frame::~Frame()
{
	if (m_pTex) { m_pTex->Release(); m_pTex = nullptr; }
	if (m_pVtx) { m_pVtx->Release(); m_pVtx = nullptr; }
}

void Frame::Draw() const
{
	SetSpritePos(m_pos.x, m_pos.y);
	SetSpriteScale(1.0f, 1.0f);
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetSpriteTexture(m_pTex);
	DrawSprite(m_pVtx);
}