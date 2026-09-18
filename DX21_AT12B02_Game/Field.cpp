#include "Field.h"
#include "SpriteDrawer.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Block.h"
#include <iostream>
#include "Input/Keyboard.h" 

Field::Field()
	:m_pFrameBuf(nullptr)
	, m_pFrameTex(nullptr)
	, m_grid{}
	, m_offset{}
	, m_state()
	, m_check{}
	, m_isMoveRight(false)
	, m_pivotBlock(nullptr)
{
	m_offset.x = 0.5f * (FIELD_COLUMN - 1.0f) * BLOCK_WIDTH;
	m_offset.y = 0.5f * (FIELD_ROW - 1.0f) * BLOCK_HEIGHT;
	m_offset.x = -m_offset.x;
	m_offset.y = -m_offset.y;

	float width = BLOCK_WIDTH / 2;
	float height = BLOCK_HEIGHT / 2;

	//頂点バッファ作成
	Vertex vtx[] = {
		{ {-width, -height, 0.0f},  {0.0f, 0.0f} }, // 左下
		{ {-width,  height, 0.0f},  {0.0f, 1.0f} }, // 左上
		{ { width, -height, 0.0f},  {1.0f, 0.0f} }, // 右下
		{ { width,  height, 0.0f},  {1.0f, 1.0f} }  // 右上
	};
	m_pFrameBuf = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	//テクスチャ読み込み
	const char* texture = "Image/Field/Frame.png";
	HRESULT hr = LoadTextureFromFile(GetDevice(), texture, &m_pFrameTex);
	if (FAILED(hr))
	{
		MessageBox(NULL, "FIELD Texture failed.", "Error", MB_OK);
	}

	//ブロック配列の中身を初期化
	for (int i = 0; i < FIELD_ROW; i++)
	{
		for (int j = 0; j < FIELD_COLUMN; j++)
		{
			m_grid[i][j] = nullptr;

		}
	}

	//サウンドデータの読み込み
	m_pBlockDestroySE = LoadSound("Sound/SE/BlockErase.wav");
}

Field :: ~Field()
{
	//念のため消えずに残っているブロックを削除
	for (int i = 0; i < FIELD_ROW; i++)
	{
		for (int j = 0; j < FIELD_COLUMN; j++)
		{
			if (m_grid[i][j] != nullptr)
				delete m_grid[i][j];
		}
	}

	//各種リソースを削除
	if (m_pFrameTex) {
		m_pFrameTex->Release();
		m_pFrameTex = nullptr;
	}
	if (m_pFrameBuf) {
		m_pFrameBuf->Release();
		m_pFrameBuf = nullptr;
	}
}

void Field::Update()
{
	m_moveInputHandled = false;	//移動入力を処理していない状態にする
	m_horizontalInputHandled = false;	//左右移動入力を処理していない状態にする

	Block* updateList[FIELD_ROW * FIELD_COLUMN];
	int updateCount = 0;

	for(int y = 0; y < FIELD_ROW; ++y)
	{
		for(int x = 0; x < FIELD_COLUMN; ++x)
		{
			if(m_grid[y][x] != nullptr)
			{
				updateList[updateCount++] = m_grid[y][x];
			}
		}
	}
	
	for(int i = 0; i < updateCount; ++i)
	{
		updateList[i]->Update();
	}

	for (int y = FIELD_ROW - 1; y >= 0; --y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			if (m_grid[y][x] != nullptr &&
				m_grid[y][x]->GetState() == Block::FALL)
			{
				UpdateFallBlock(x, y);
			}
		}
	}

	//フィールドの更新処理
	switch (m_state)
	{
	case Field::State::CREATE:		   UpdateCreate(); 		 break;
	case Field::State::IDLE:		   UpdateIdle();		 break;
	case Field::State::CHECK:		   UpdateCheck();		 break;
	case Field::State::DESTROY:		   UpdateDestroy();		 break;
	case Field::State::GAMEOVER:	   UpdateGameOver();	 break;
	}
}


void Field::Draw()
{
	SetSpriteTexture(m_pFrameTex); // 表示する枠全てに同じ画像を適用 
	SetSpriteScale(1.0f, 1.0f); // スケールを元に戻す

	for (int y = 0; y < FIELD_ROW; ++y) {
		for (int x = 0; x < FIELD_COLUMN; ++x) {
			SetSpritePos(BLOCK_WIDTH * x + m_offset.x,
				BLOCK_HEIGHT * y + m_offset.y);
			DrawSprite(m_pFrameBuf);
		}
	}

	for (int y = 0; y < FIELD_ROW; ++y) {
		for (int x = 0; x < FIELD_COLUMN; ++x) {
			if (m_grid[y][x] != nullptr) {
				m_grid[y][x]->Draw();
			}
		}
	}

	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetSpriteScale(1.0f, 1.0f);
}

bool Field::IsMoveRight()
{
	return m_isMoveRight;
}

void Field::SetMoveRight(bool isMoveRight)
{
	m_isMoveRight = isMoveRight;
}

//
void Field::HardDrop()
{
	if (m_moveInputHandled)
		return;
	m_moveInputHandled = true;

	// 下キーが押されたときに、ブロックを一気に落下させる処理
	Block* moveBlock[2] = { nullptr, nullptr };

	// 操作中のブロックの座標を記録する配列
	int blockX[2] = { -1, -1 };
	int blockY[2] = { -1, -1 };

	int blockCount = 0;


	for(int y = 0; y < FIELD_ROW; ++y)
	{
		for(int x = 0; x < FIELD_COLUMN; ++x)
		{
			//ブロックが存在しなければ処理しない
			if(m_grid[y][x] == nullptr)		continue;
			//ブロックの状態が移動中でなければ処理しない
			if (m_grid[y][x]->GetState() != Block::State::MOVE)		continue;
			//ブロックの数が2個を超えたら処理しない
			if (blockCount >= 2) break;

			moveBlock[blockCount] = m_grid[y][x];
			blockX[blockCount] = x;
			blockY[blockCount] = y;

			blockCount++;
		}
		if (blockCount >= 2) break;
	}

	//２個そろっていなければ何もしない
	if (blockCount != 2)	return;


	//何マス落下できるかを計算する
	int dropDist = FIELD_ROW;

	for(int i = 0;i < 2; ++i)
	{
		int x = blockX[i];
		int y = blockY[i];
		int dist = 0;
		while (y + dist + 1 < FIELD_ROW)
		{
			int checkY = y + dist + 1;

			//操作中の片方のブロックを無視する
			if(m_grid[checkY][x] == moveBlock[0] || 
				m_grid[checkY][x] == moveBlock[1])
			{
				dist++;
				continue;
			}

			//他のブロックにぶつかった
			if(m_grid[checkY][x] != nullptr)
			{
				break;
			}
			dist++;
		}

		//２個のうち最も落ちれない距離を採用する
		if (dist < dropDist) dropDist = dist;
	}

	//グリッド上の位置を変更する
	if (dropDist > 0)
	{
		//元の場所を空に
		for(int i = 0; i < 2; ++i)
		{
			m_grid[blockY[i]][blockX[i]] = nullptr;
		}

		//新しい場所に移動する
		for(int i = 0; i < 2; ++i)
		{
			int newY = blockY[i] + dropDist;
			m_grid[newY][blockX[i]] = moveBlock[i];

			//ブロックの座標を更新する
			Index index;
			index.x = blockX[i];
			index.y = newY;

			moveBlock[i]->SetPos(IndexToPos(index));

			//落下速度をリセット
			moveBlock[i]->ResetFallSpeed();

			//落下終了
			moveBlock[i]->SetState(Block::State::IDLE);
		}
	}
	else
	{
		//既に落下できない場合
		moveBlock[0]->SetState(Block::State::IDLE);
		moveBlock[1]->SetState(Block::State::IDLE);

		moveBlock[0]->ResetFallSpeed();
		moveBlock[1]->ResetFallSpeed();
	}

	//落下後の状態をチェックする
	for (int y = FIELD_ROW - 2; y >= 0; --y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			Block* pBlock = m_grid[y][x];

			if (pBlock == nullptr)
				continue;

			// 操作中のブロックは除外
			if (pBlock->GetState() == Block::MOVE)
				continue;

			// 下が空いているなら落下
			if (m_grid[y + 1][x] == nullptr)
			{
				pBlock->SetState(Block::FALL);
				pBlock->ResetFallSpeed();
			}
		}
	}
}

void Field::SoftDrop()
{
	if(m_moveInputHandled)
		return;	
	m_moveInputHandled = true;

	Block* moveBlock[2] = { nullptr, nullptr };

	int blockX[2] = { -1, -1 };
	int blockY[2] = { -1, -1 };

	int blockCount = 0;

	//==================================================
	// 操作中のブロックを探す
	//==================================================
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			if (m_grid[y][x] == nullptr)
				continue;

			if (m_grid[y][x]->GetState() != Block::State::MOVE)
				continue;

			moveBlock[blockCount] = m_grid[y][x];
			blockX[blockCount] = x;
			blockY[blockCount] = y;

			blockCount++;

			if (blockCount == 2)
				break;
		}

		if (blockCount == 2)
			break;
	}

	// 2個揃っていなければ終了
	if (blockCount != 2)
		return;


	//==================================================
	// まず「2個とも」下に移動できるか確認
	//==================================================
	bool canDrop = true;

	for (int i = 0; i < 2; ++i)
	{
		int x = blockX[i];
		int y = blockY[i];

		// 一番下
		if (y + 1 >= FIELD_ROW)
		{
			canDrop = false;
			break;
		}

		// 下に別のブロックがある
		Block* below = m_grid[y + 1][x];

		if (below != nullptr)
		{
			// 下のブロックが操作中の2個ならOK
			if (below != moveBlock[0] &&
				below != moveBlock[1])
			{
				canDrop = false;
				break;
			}
		}
	}

	//==================================================
	// 落下できない
	//==================================================
	if (!canDrop)
	{
		for (int i = 0; i < 2; ++i)
		{
			moveBlock[i]->SetState(Block::State::IDLE);
			moveBlock[i]->ResetFallSpeed();
		}

		return;
	}


	//==================================================
	// 元の位置をすべて空にする
	//==================================================
	for (int i = 0; i < 2; ++i)
	{
		m_grid[blockY[i]][blockX[i]] = nullptr;
	}


	//==================================================
	// 2個とも1段下げる
	//==================================================
	for (int i = 0; i < 2; ++i)
	{
		int newX = blockX[i];
		int newY = blockY[i] + 1;

		m_grid[newY][newX] = moveBlock[i];

		// 座標を更新
		Index index;
		index.x = newX;
		index.y = newY;

		moveBlock[i]->SetPos(IndexToPos(index));

		// 落下速度をリセット
		moveBlock[i]->ResetFallSpeed();

		// 落下状態
		moveBlock[i]->SetState(Block::State::MOVE);
	}
}

void Field::MoveHorizontal(int direction)
{
	if(m_horizontalInputHandled)	return;
	m_horizontalInputHandled = true;

	Block* moveBlock[2] = { nullptr, nullptr };
	int BlockX[2] = { -1, -1 };
	int BlockY[2] = { -1, -1 };
	int BlockCount = 0;

	// 操作中のブロックを探す
	for(int y = 0; y < FIELD_ROW; ++y)
	{
		for(int x = 0; x < FIELD_COLUMN; ++x)
		{
			if(m_grid[y][x] == nullptr)	continue;
			if(m_grid[y][x]->GetState() != Block::State::MOVE)	continue;
			moveBlock[BlockCount] = m_grid[y][x];
			BlockX[BlockCount] = x;
			BlockY[BlockCount] = y;
			BlockCount++;
			if(BlockCount == 2)	break;
		}
		if(BlockCount == 2)	break;
	}

	int targetX[2];
	bool canMove = true;

	for (int i = 0; i < 2; i++)
	{
		targetX[i] = BlockX[i] + direction;
		if (targetX[i] < 0 || targetX[i] >= FIELD_COLUMN)
		{
			canMove = false;
			break;
		}
		Block* targetBlock = m_grid[BlockY[i]][targetX[i]];
		if (targetBlock != nullptr && targetBlock != moveBlock[0] && targetBlock != moveBlock[1])
		{
			canMove = false;
			break;
		}
	}
	
	if (!canMove)	return;

	for (int i = 0; i < 2; i++)
	{
		m_grid[BlockY[i]][BlockX[i]] = nullptr;
	}
	for(int i = 0; i < 2; i++)
	{
		m_grid[BlockY[i]][targetX[i]] = moveBlock[i];
		Index index;
		index.x = targetX[i];
		index.y = BlockY[i];
		moveBlock[i]->SetPos(IndexToPos(index));
	}

	SetMoveRight(direction > 0);
}

void Field::UpdateCreate()
{
	//乱数の初期化
	srand((unsigned int)time(NULL));

	//ブロックの生成場所を計算
	int x = FIELD_COLUMN / 2;
	int y = 0;
	int time = timeBeginPeriod(1);

	//ブロックが生成される前に、すでにブロックが存在する場合はゲームオーバー
	if (m_grid[y][x] != nullptr)
	{
		m_state = Field::State::GAMEOVER;
		return;
	}

	//ブロックを縦に２個生成（色数に合わせてランダム）
	m_grid[y][x] = new Block(rand() % BLOCK_COLOR_NUM, this);
	m_grid[y + 1][x] = new Block(rand() % BLOCK_COLOR_NUM, this);

	// 下側のブロックを回転中心として固定する。
	// 回転するたびに「下側のブロック」を探し直さないことで、
	// 回転するたびに中心が入れ替わる問題を防ぐ。
	m_pivotBlock = m_grid[y + 1][x];

	// 生成したブロックの位置を、配列の添え字に該当する箇所へ移動 
	Index index;
	index.x = x;
	index.y = y;
	float2 pos = IndexToPos(index);
	m_grid[y][x]->SetPos(pos.x, pos.y);
	m_grid[y + 1][x]->SetPos(pos.x, pos.y + BLOCK_HEIGHT);

	//ブロックの落下待機へ処理を変更
	m_state = Field::State::IDLE;
}

void Field::UpdateIdle()
{
	// 回転処理：__VK_UP__ が押されたら「移動中(MOVE)」の隣接した2ブロックのうち
	// 見つかった組を1つだけ90度回転させる（右回り）。回転先が範囲外／他ブロックで塞がれていたら何もしない。
	if (isKeyTrigger('X'))
	{
		RotateBlock(1);
	}
	if (isKeyTrigger('Z'))
	{
		RotateBlock(-1);
	}

	for (int y = FIELD_ROW - 1; y >= 0; --y)
	{
		if (m_isMoveRight)
		{
			// 右移動中は右側のマスから処理
			for (int x = FIELD_COLUMN - 1; x >= 0; --x)
			{
				syncBlock(x, y);
			}
		}
		else
		{
			// 左移動・静止時は左側のマスから処理
			for (int x = 0; x < FIELD_COLUMN; ++x)
			{
				syncBlock(x, y);
			}
		}
	}

	bool isIdle = true;
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			if (m_grid[y][x] == nullptr) continue;
			if (m_grid[y][x]->GetState() == Block::IDLE) continue;
			isIdle = false;
		}
	}

	if (isIdle)
		m_state = Field::CHECK;	//全てのブロックが待機状態ならチェックに切り替える
}

void Field::UpdateCheck()
{
	//生成に切り替える
	m_state = Field::CREATE;

	//確認済みか記録するデータの初期化
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			m_check[y][x] = false;
		}
	}

	//全てのブロックに対して同じ色のブロック数を確認する
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			//再起処理でブロックの数を確認
			if (m_check[y][x])	continue;				//すでにチェック済みであれば以降の処理を行わない
			Index index = { x,y };						//探索スタート箇所のインデックスを作成
			int   count = RecursiveBlockCount(index);	//再起処理で個数を探索

			//指定された削除数を超えたら該当のブロックを削除する
			if (count >= BLOCK_ERACE_NUM)
			{
				//再起処理でブロックを削除する
				RecursiveBlockDestroy(index);
				//ブロックを消したので削除待ちのステートに切り替える
				m_state = Field::DESTROY;
			}

			//確認用にブロックの個数をMessageBox関数で表示する処理(使用しない場合は0に変更)
#if 0
			if (count > 0)
			{
				std::stringstream caption;
				std::stringstream text;
				caption << "m_grid[" << y << "][" << x << "]";
				text << "count = " << count;
				MessageBox(NULL, text.str().c_str(), caption.str().c_str(), MB_OK);
			}
#endif
			if (m_state == Field::DESTROY)
			{
				
				//サウンド再生
				PlaySound(m_pBlockDestroySE, 0.005f);
			}
		}
	}
}

void Field::UpdateDestroy()
{
	//ブロックの削除アニメーションが終了しているか確認する
	bool isDestroy = false;

	//消す状態のブロックを実際に削除する
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			//ブロックが無ければスキップ
			if (m_grid[y][x] == nullptr)	continue;

			//ブロックが削除ステートなら削除する
			if (m_grid[y][x]->GetState() == Block::ERASE)
			{
				if (m_grid[y][x] == m_pivotBlock)
				{
					m_pivotBlock = nullptr;
				}

				delete m_grid[y][x];
				m_grid[y][x] = nullptr;
			}
			else if (m_grid[y][x]->GetState() == Block::DESTROY)
			{
				isDestroy = true;
			}
		}
	}

	if (!isDestroy)
	{
		//一度すべてのブロックを待機から落下状態に変更する
		for (int y = 0; y < FIELD_ROW; ++y)
		{
			for (int x = 0; x < FIELD_COLUMN; ++x)
			{
				//ブロックが無ければスキップする
				if (m_grid[y][x] == nullptr) continue;

				//ブロック状態に変更
				m_grid[y][x]->SetState(Block::FALL);
			}
		}

		//削除アニメーションが終了しているので、落下待機状態に切り替える
		m_state = Field::IDLE;
	}
}

void Field::UpdateGameOver()
{

}

float2 Field::IndexToPos(Index index)
{
	//計算結果を格納する変数
	float2 pos;

	//配列の添え字とブロックの大きさを掛け合わせる
	pos.x = index.x * BLOCK_WIDTH;
	pos.y = index.y * BLOCK_HEIGHT;

	//フィールドの配置でずらしている量を考慮
	pos.x += m_offset.x;
	pos.y += m_offset.y;

	return pos;
}

Field::Index Field::PosToIndex(float2 pos)
{
	//計算結果を格納
	Index index;

	//フィールドの配置でずらした分だけブロックを移動
	pos.x -= m_offset.x;
	pos.y -= m_offset.y;

	//座標からインデックスに計算する際、左上が基準の座標出ないと
	//計算の誤差で正しくないインデックスが取得される。
	//ブロックは中心を基準としてるため、この計算の時だけ左上が
	//基準(0,0)となるように計算。 

	//ブロックの原点を中心から左上に移動
	pos.x += BLOCK_WIDTH * 0.5f;
	pos.y += BLOCK_HEIGHT * 0.5f;

	//座標をブロックのサイズで割り、計算結果を整数に変換
	index.x = (int)(pos.x / BLOCK_WIDTH);
	index.y = (int)(pos.y / BLOCK_HEIGHT);

	//計算結果を返す
	return index;
}

int Field::RecursiveBlockCount(Index index)
{
	//二次元配列の範囲外チェック
	if (index.x < 0 || index.x >= FIELD_COLUMN || index.y < 0 || index.y >= FIELD_ROW)
		return 0;

	//ブロックの確認済みのフラグを立てる
	m_check[index.y][index.x] = true;

	//ブロックの数を記録する変数
	int count = 1;//自分を含めるため１つは必ず存在する

	//該当箇所のブロックを取得
	//indexを使用しブロックの二次元配列からブロックを取得
	Block* pCenter = m_grid[index.y][index.x];


	//調べる予定のブロックが空(NULL)の場合、ブロック個数を0として処理を終了する
	if (pCenter == nullptr) return 0;

	//上下左右のブロックを効率よく調べるための配列
	Index aroundIndex[] =
	{
		{index.x - 1,index.y},		//左
		{index.x + 1,index.y},		//右
		{index.x	,index.y - 1},	//上
		{index.x	,index.y + 1}	//下
	};

	//各方向を繰り返し探索
	for (int i = 0; i < _countof(aroundIndex); ++i)
	{
		//二次元配列の範囲外チェック
		if (aroundIndex[i].x < 0 || aroundIndex[i].x >= FIELD_COLUMN ||
			aroundIndex[i].y < 0 || aroundIndex[i].y >= FIELD_ROW)
			continue;

		//周辺ブロックを取得
		Block* pAround = m_grid[aroundIndex[i].y][aroundIndex[i].x];

		//確認済みのブロックであれば以降の処理を行わない
		if (m_check[aroundIndex[i].y][aroundIndex[i].x])	continue;

		//取得した周辺のブロックがからの場合、移行の処理を行わない
		if (pAround == nullptr) continue;

		//同じ色ではない場合移行の処理をしない
		if (pCenter->GetColor() != pAround->GetColor())	continue;

		//探索方向のブロック数を再起処理で取得
		count += RecursiveBlockCount(aroundIndex[i]);
	}

	//見つかったブロック数を戻り値で返す
	return count;
}

void Field::RecursiveBlockDestroy(Index index)
{
	// インデックスの範囲チェック
	if (index.x < 0 || index.x >= FIELD_COLUMN || index.y < 0 || index.y >= FIELD_ROW)
		return;

	//該当ブロックを取得
	Block* pCenter = m_grid[index.y][index.x];

	//ブロックが空なら処理しない
	if (pCenter == nullptr) return;

	//該当箇所のブロックを削除
	pCenter->SetState(Block::DESTROY);

	//繰り返しのインデックスを定義
	Index aroundIndex[] =
	{
		{index.x - 1,index.y},		//左
		{index.x + 1,index.y},		//右
		{index.x	,index.y - 1},	//上
		{index.x	,index.y + 1}	//下
	};

	//上下左右に繰り返しで調べる
	for (int i = 0; i < _countof(aroundIndex); ++i)
	{
		//インデックスがフィールド内か判定
		if (aroundIndex[i].x < 0)				continue;
		if (aroundIndex[i].x >= FIELD_COLUMN)	continue;
		if (aroundIndex[i].y < 0)				continue;
		if (aroundIndex[i].y >= FIELD_ROW)		continue;

		//周辺のブロックを取得
		Block* pAround = m_grid[aroundIndex[i].y][aroundIndex[i].x];

		//取得したブロックが空かチェック
		if (pAround == nullptr) continue;

		//削除ステートなら移行の処理をスキップ
		if (pAround->GetState() == Block::DESTROY) continue;

		//同じ色でなければスキップ
		if (pAround->GetColor() != pCenter->GetColor())	continue;

		RecursiveBlockDestroy(aroundIndex[i]);
	}
}

void Field::syncBlock(int x, int y)
{
	if (!m_grid[y][x]) return;
	if (m_grid[y][x]->GetState() != Block::MOVE) return;

	float2 pos = m_grid[y][x]->GetPos();

	if (-m_offset.y < pos.y)
	{
		m_grid[y][x]->SetPos(pos.x, -m_offset.y);
		m_grid[y][x]->SetState(Block::IDLE);
	}

	pos = m_grid[y][x]->GetPos();
	Index index = PosToIndex(pos);

	if(m_grid[index.y][index.x] != nullptr &&
		m_grid[index.y][index.x] != m_grid[y][x])
	{
		index.y = y;
		pos = IndexToPos(index);
		m_grid[y][x]->SetPos(pos.x, pos.y);
		m_grid[y][x]->SetState(Block::IDLE);
	}
	else
	{
		m_grid[index.y][index.x] = m_grid[y][x];
		m_grid[y][x] = nullptr;
	}
}

void Field::RotateBlock(int direction)
{
	//============================================================
	// 現在操作中の2個のブロックを、固定した回転中心を使って回転する
	//
	// フィールド座標：
	//   X : 右が +
	//   Y : 下が +
	//
	// direction > 0 : 時計回り
	// direction < 0 : 反時計回り
	//
	// 生成時に下側のブロックを m_pivotBlock に保存する。
	// 以降の回転では、毎回「下にあるブロック」を中心にせず、
	// 必ず同じブロックを中心にする。
	//============================================================

	if (m_pivotBlock == nullptr)
		return;

	if (m_pivotBlock->GetState() != Block::MOVE)
		return;


	//============================================================
	// 落下によって配列上の位置が変わっていても、
	// m_grid から回転中心の現在位置を探す
	//============================================================

	int pivotX = -1;
	int pivotY = -1;

	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			if (m_grid[y][x] == m_pivotBlock)
			{
				pivotX = x;
				pivotY = y;
				break;
			}
		}

		if (pivotX != -1)
			break;
	}

	if (pivotX == -1 || pivotY == -1)
		return;


	//============================================================
	// 回転中心に隣接している、もう片方の MOVE ブロックを探す
	//============================================================

	const int dx[] = { 0, 0, -1, 1 };
	const int dy[] = { -1, 1, 0, 0 };

	Block* moverBlock = nullptr;

	int moverX = -1;
	int moverY = -1;

	for (int i = 0; i < 4; ++i)
	{
		int x = pivotX + dx[i];
		int y = pivotY + dy[i];

		if (x < 0 || x >= FIELD_COLUMN ||
			y < 0 || y >= FIELD_ROW)
		{
			continue;
		}

		if (m_grid[y][x] == nullptr)
			continue;

		if (m_grid[y][x]->GetState() != Block::MOVE)
			continue;

		moverBlock = m_grid[y][x];
		moverX = x;
		moverY = y;
		break;
	}

	if (moverBlock == nullptr)
		return;


	//============================================================
	// 回転前の回転中心からの相対座標
	//============================================================

	int relativeX = moverX - pivotX;
	int relativeY = moverY - pivotY;

	int rotatedX;
	int rotatedY;


	//============================================================
	// 画面座標系での90度回転
	//
	// 上 ( 0,-1 ) を時計回りに回すと右 ( 1,0 )
	//
	// 時計回り：
	//   (x,y) -> (-y,x)
	//
	// 反時計回り：
	//   (x,y) -> (y,-x)
	//============================================================

	if (direction > 0)
	{
		// 時計回り
		rotatedX = -relativeY;
		rotatedY = relativeX;
	}
	else
	{
		// 反時計回り
		rotatedX = relativeY;
		rotatedY = -relativeX;
	}


	int targetX = pivotX + rotatedX;
	int targetY = pivotY + rotatedY;


	//============================================================
	// 回転先がフィールド外なら回転しない
	//============================================================

	if (targetX < 0 || targetX >= FIELD_COLUMN ||
		targetY < 0 || targetY >= FIELD_ROW)
	{
		return;
	}


	//============================================================
	// 回転先に別のブロックがあれば回転しない
	//============================================================

	if (m_grid[targetY][targetX] != nullptr &&
		m_grid[targetY][targetX] != moverBlock)
	{
		return;
	}


	//============================================================
	// 配列を更新
	//
	// m_pivotBlock は絶対に移動させない。
	// moverBlock だけを回転先へ移動する。
	//============================================================

	m_grid[moverY][moverX] = nullptr;
	m_grid[targetY][targetX] = moverBlock;


	//============================================================
	// 表示位置をグリッドに合わせる
	//============================================================

	Index pivotIndex;
	pivotIndex.x = pivotX;
	pivotIndex.y = pivotY;

	Index targetIndex;
	targetIndex.x = targetX;
	targetIndex.y = targetY;

	// 回転中心はその場に固定
	m_pivotBlock->SetPos(IndexToPos(pivotIndex));

	// もう片方だけ回転後の位置へ移動
	moverBlock->SetPos(IndexToPos(targetIndex));
}

void Field::UpdateFallBlock(int x, int y)
{
	Block* pBlock = m_grid[y][x];
	if (pBlock == nullptr) return;

	float2 pos = pBlock->GetPos();

	//現在のマスのグリッド座標(Y)
	Index curIndex = { x, y };
	float curY = IndexToPos(curIndex).y;

	//最下段か、真下のマスが埋まっているか
	bool isBottom = (y + 1 >= FIELD_ROW);
	bool isBlocked = isBottom || (m_grid[y + 1][x] != nullptr);

	if (isBlocked)
	{
		//これ以上落下できないので、現在のマスにきっちり揃えて着地させる
		pBlock->SetPos(pos.x, curY);
		pBlock->SetState(Block::IDLE);
		pBlock->ResetFallSpeed();
		return;
	}

	//下のマスが空いている場合、そこに到達したかチェック
	Index belowIndex = { x, y + 1 };
	float belowY = IndexToPos(belowIndex).y;

	if (pos.y >= belowY)
	{
		//1マス分落下完了。行き過ぎた分は切り捨てて座標を揃え、
		//配列参照を1つ下へ移す
		pBlock->SetPos(pos.x, belowY);
		m_grid[y + 1][x] = pBlock;
		m_grid[y][x] = nullptr;
	}
}

Field::State Field::GetState() const
{
	return m_state;
}
