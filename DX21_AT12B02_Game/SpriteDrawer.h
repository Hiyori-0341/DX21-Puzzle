#ifndef __SPRITE_DRAWER_H__
#define __SPRITE_DRAWER_H__

#include <d3d11.h>

struct float3 {
	float x;
	float y;
	float z;
};
struct float2 {
	float x;
	float y;
};

// 頂点の情報
struct Vertex {
	float3 pos;
	float2 uv;
};

/* @brief 初期化
 * @param[in] width 画面横幅
 * @param[in] height 画面縦幅
 * @param[in] isYup true-画面上方向に+/false-画面下方向に+
 */
void InitSpriteDrawer(
	ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
	UINT width, UINT height, bool isYup = false);
void UninitSpriteDrawer();

/*
 * @描画処理
 */
void DrawSprite(ID3D11Buffer* pSprite);

//--- 各種スプライトの設定関数 ---
void SetSpritePos(float x, float y);
void SetSpriteAngle(float rad);
void SetSpriteScale(float x, float y);
void SetSpriteUVPos(float u, float v);
void SetSpriteUVScale(float u, float v);
void SetSpriteColor(float r, float g, float b, float a);
void SetSpriteTexture(ID3D11ShaderResourceView* pTexture);


#endif // __SPRITE_DRAWER_H__