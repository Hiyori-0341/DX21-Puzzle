#pragma once
#include "../DirectX.h"

//定数
constexpr float TITLE_BUTTON_WIDTH = 200.0f;
constexpr float TITLE_BUTTON_HEIGHT = 50.0f;



class TitleButton
{
public:
	TitleButton();
	~TitleButton();
	bool Init();
	void Update();
	void Draw();

private:
	//点滅アニメーションの描画
	void DrawFlash(float cycleTime);

	//ステートに応じたボタンの表示処理
	void DrawBefore();
	void DrawEnter();
	void DrawAfter();

public:
	enum State
	{
		BEFORE,		//ボタンを押す前
		ENTER,		//押した瞬間
		AFTER,		//押した後
	};

private:
	ID3D11Buffer* m_pBuffer;
	ID3D11ShaderResourceView* m_pTexture;

	State	 m_state;
	int		 m_animeFrame;

public:
	State GetState() const;
};