#pragma once

#define APP_TITLE		("DX21 Game")
#define SCREEN_WIDTH	(960)
#define SCREEN_HEIGHT	(540)
#define FPS				(60)

#define BLOCK_WIDTH		(40.0f)
#define BLOCK_HEIGHT	(40.0f)

#define DIGIT_WIDTH		(20.0f)
#define DIGIT_HEIGHT	(30.0f)

// スコア表示
#define SCORE_DIGIT_NUM	(6)	//スコア表示の桁数
#define SCORE_POS_X  ( 50.0f)
#define	SCORE_POS_Y  (250.0f)

//「れんさ」ラベルのサイズ
#define CHAIN_LABEL_WIDTH	(60.0f)
#define CHAIN_LABEL_HEIGHT	(40.0f)
#define CHAIN_FLASH_INTERVAL	(5)	//点滅間隔
#define CHAIN_MOVE_SPEED		(1.5f)	//ブロックの移動速度

// フィールドサイズ
#define FIELD_COLUMN	(6)
#define FIELD_ROW		(12)

#define BLOCK_COLOR_NUM (4)		//ブロックの色の種類
#define BLOCK_ERACE_NUM (4)		//指定個数つながったらブロックを消す

//ブロック削除アニメーションの設定
#define BLOCK_DESTROY_FLASH_CNT		 (4)	//点滅回数
#define BLOCK_DESTROY_FLASH_INTERVAL (5)	//点滅間隔
#define BLOCK_DESTROY_FLASH_FRAME    (BLOCK_DESTROY_FLASH_CNT * BLOCK_DESTROY_FLASH_INTERVAL * 2)
#define BLOCK_DESTROY_BOMB_FRAME     (7)	//爆発フレーム
#define BLOCK_DESTROY_TOTAL_FRAME    (BLOCK_DESTROY_FLASH_FRAME + BLOCK_DESTROY_BOMB_FRAME)	//合計フレーム数
#define BLOCK_DESTROY_BOMB_SCALE     (1.2f)	//爆発の拡大率

//ブロックの着地時のつぶれ演出の設定
#define SQUASH_OFFSET	(8.0f)	//つぶれの最大縮小率
#define SQUASH_DECAY	(0.085)	//つぶれの1フレームあたりの減衰率

//ゴーストピース
#define GHOST_ALPHA	(0.5f)	//ゴーストピースの透明度

//レベルシステム
#define  LEVEL_UP_ERASE_COUNT (10)	//消したブロックの個数がこの値に達するとレベルアップする
#define  LEVEL_MAX (99)	//最大レベル
#define  FALL_WAIT_TIME_BASE (60)	//ブロックが落下するまでの待機時間の基本値(フレーム)
#define  FALL_WAIT_TIME_MIN  (10)	//ブロックが落下するまでの待機時間の最小値(フレーム)
#define  FALL_WAIT_TIME_STEP (2)	//ブロックが落下するまでの待機時間の減少量(フレーム)


constexpr float PI = 3.14159265f;