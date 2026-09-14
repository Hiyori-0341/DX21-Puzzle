#pragma once

#define APP_TITLE		("DX21 Game")
#define SCREEN_WIDTH	(960)
#define SCREEN_HEIGHT	(540)
#define FPS				(60)

#define FIELD_COLUMN (6)
#define FIELD_ROW (12)

#define BLOCK_COLOR_NUM (4)		//ブロックの色の種類
#define BLOCK_ERACE_NUM (4)		//指定個数つながったらブロックを消す

//ブロック削除アニメーションの設定
#define BLOCK_DESTROY_FLASH_CNT		 (4)	//点滅回数
#define BLOCK_DESTROY_FLASH_INTERVAL (5)	//点滅間隔
#define BLOCK_DESTROY_FLASH_FRAME    (BLOCK_DESTROY_FLASH_CNT * BLOCK_DESTROY_FLASH_INTERVAL * 2)
#define BLOCK_DESTROY_BOMB_FRAME     (7)	//爆発フレーム
#define BLOCK_DESTROY_TOTAL_FRAME    (BLOCK_DESTROY_FLASH_FRAME + BLOCK_DESTROY_BOMB_FRAME)	//合計フレーム数
#define BLOCK_DESTROY_BOMB_SCALE     (1.2f)	//爆発の拡大率

constexpr float PI = 3.14159265f;