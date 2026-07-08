#include "Field.h"
#include "SpriteDrawer.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Block.h"
#include <sstream>
#include "Input/Keyboard.h" // 追加：キー入力を使うため

Field::Field()
	:m_pFrameBuf (nullptr)
	,m_pFrameTex (nullptr)
	,m_grid		 {}
	,m_offset	 {}
	,m_state	 ()
	,m_check	 {}
	,m_isMoveRight (false)
{
	m_offset.x =   0.5f * (FIELD_COLUMN - 1.0f)	 * BLOCK_WIDTH;
	m_offset.y =   0.5f * (FIELD_ROW - 1.0f)	 * BLOCK_HEIGHT;
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
	m_pFrameBuf = CreateVertexBuffer(GetDevice(), vtx, 4);

	//テクスチャ読み込み
	HRESULT hr = LoadTextureFromFile(GetDevice(), "Image/Field/Frame.png", &m_pFrameTex);
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
	for (int y = 0; y < FIELD_ROW; ++y) {
		for (int x = 0; x < FIELD_COLUMN; ++x) 
		{
			if (m_grid[y][x] != nullptr)
			{
				m_grid[y][x]->Update();
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
	for (int y = 0; y < FIELD_ROW; ++y) {
		for (int x = 0; x < FIELD_COLUMN; ++x) {
			SetSpritePos(BLOCK_WIDTH * x + m_offset.x,
						 BLOCK_HEIGHT * y  +  m_offset.y);
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
}

bool Field::IsMoveRight()
{
	return m_isMoveRight;
}

void Field::SetMoveRight(bool isMoveRight)
{
	m_isMoveRight = isMoveRight;
}


void Field::UpdateCreate()
{
	//乱数の初期化
	srand((unsigned int)time(NULL));

	//ブロックの生成場所を計算
	int x = FIELD_COLUMN / 2;
	int y = 0;
	int time = timeBeginPeriod(1);

	//ブロックを縦に２個生成（色数に合わせてランダム）
	m_grid[y][x] = new Block(rand() % BLOCK_COLOR_NUM, this);
	m_grid[y + 1][x] = new Block(rand() % BLOCK_COLOR_NUM, this);

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
		m_state = Field::CHECK;
	if(isIdle)
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
		}
	}
}

void Field::UpdateDestroy()
{
	//消す状態のブロックを実際に削除する
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			//ブロックが無ければスキップ
			if(m_grid[y][x] == nullptr)	continue;

			//ブロックが削除ステートなら削除する
			if (m_grid[y][x]->GetState() == Block::DESTROY)
			{
				delete m_grid[y][x];
				m_grid[y][x] = nullptr;
			}
		}
	}

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

	m_state = Field::IDLE;
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
		if (aroundIndex[i].x < 0)				continue;
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

	float2 pos = m_grid[y][x]->GetPos();

	if (m_offset.x > pos.x)
	{
		m_grid[y][x]->SetPos(m_offset.x, pos.y);
	}
	if (-m_offset.x < pos.x)
	{
		m_grid[y][x]->SetPos(-m_offset.x, pos.y);
	}
	if (-m_offset.y < pos.y)
	{
		m_grid[y][x]->SetPos(pos.x, -m_offset.y);
		m_grid[y][x]->SetState(Block::IDLE);
	}

	pos = m_grid[y][x]->GetPos();
	Index index = PosToIndex(pos);

	if (m_grid[index.y][index.x] != nullptr)
	{
		if (index.y != y)
		{
			index.y = y;
			pos = IndexToPos(index);
			m_grid[y][x]->SetPos(pos);
			m_grid[y][x]->SetState(Block::IDLE);
		}
		else
		{
			index.x = x;
			pos = IndexToPos(index);
			m_grid[y][x]->SetPos(pos);
		}
	}
	else
	{
		m_grid[index.y][index.x] = m_grid[y][x];
		m_grid[y][x] = nullptr;
	}
}

void Field::RotateBlock(int direction)
{
	bool rotated = false;
	for (int y = 0; y < FIELD_ROW && !rotated; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN && !rotated; ++x)
		{
			Block* firstBlock = m_grid[y][x];
			if (firstBlock == nullptr) continue;
			if (firstBlock->GetState() != Block::MOVE) continue;

			// 隣接方向を探索（上, 下, 左, 右）
			Index aroundIndex[] = { {0,-1}, {0,1}, {-1,0}, {1,0} };

			for (int i = 0; i < _countof(aroundIndex) && !rotated; ++i)
			{
				int nx = x + aroundIndex[i].x;
				int ny = y + aroundIndex[i].y;
				if (nx < 0 || nx >= FIELD_COLUMN || ny < 0 || ny >= FIELD_ROW) continue;

				Block* secondBlock = m_grid[ny][nx];
				if (secondBlock == nullptr) continue;
				if (secondBlock->GetState() != Block::MOVE) continue;

				// firstBlock をピボットにして secondBlock を回転させる
				int dx = nx - x;
				int dy = ny - y;
				// 90度右回転 (x,y) -> (y, -x) （フィールドの y は下方向増加を想定）
				int rdx =  dy * direction;
				int rdy = -dx * direction;
				int tx = x + rdx;
				int ty = y + rdy;

				// 回転先チェック
				if (tx < 0 || tx >= FIELD_COLUMN || ty < 0 || ty >= FIELD_ROW) continue;
				Block* target = m_grid[ty][tx];
				// 回転先が別のブロックで塞がれていたら不可（自分(b)がいる位置は既に cleared するため OK）
				if (target != nullptr && target != secondBlock) continue;

				// 実際に移動（配列と見た目座標）
				m_grid[ny][nx] = nullptr;
				m_grid[ty][tx] = secondBlock;
				Index idx; 
				idx.x = tx; 
				idx.y = ty;

				float2 newPos = IndexToPos(idx);
				secondBlock->SetPos(newPos);

				rotated = true;
			}
		}
	}
}
