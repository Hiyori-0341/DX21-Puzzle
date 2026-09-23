#include "Field.h"
#include "Chain.h"
#include "Frame.h"
#include "NextTsumo.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Input/Keyboard.h"
#include <algorithm>
#include <cstdlib>
#include <utility>


//============================================================
// コンストラクタ / デストラクタ
//============================================================

Field::Field()
	: m_pFrameBuf(nullptr)
	, m_pFrameTex(nullptr)
	, m_offset{}
	, m_grid{}
	, m_state(Field::CREATE)
	, m_pairBlock{ nullptr, nullptr }
	, m_pairIndex{}
	, m_fallTimer(0)
	, m_quickTurnDirection(0)
	, m_pBlockDestroySE(nullptr)
{
	//フレーム
	float frameWidth = BLOCK_WIDTH * FIELD_COLUMN + 32.0f;
	float frameHeight = BLOCK_HEIGHT * FIELD_ROW + 32.0f;
	m_pFrame = new Frame("Image/UI/FieldFrame.png", frameWidth, frameHeight, { 0.0f, 0.0f });
	//数字描画
	m_pChain = new Chain();

	//次のツモ表示
	for(int i = 0; i < PAIR_NUM; ++i)
	{
		m_nextColor[i] = m_colorGen.Next();
	}
	m_pNextTsumo = new NextTsumo();
	m_pNextTsumo->SetColor(m_nextColor[PIVOT], m_nextColor[SUB]);

	//ブロックの共有リソース(頂点バッファ・テクスチャ)を読み込む
	Block::LoadResources();

	//フィールドが画面中央に来るように、左上マスの表示位置を決める
	m_offset.x = -0.5f * (FIELD_COLUMN - 1) * BLOCK_WIDTH;
	m_offset.y = -0.5f * (FIELD_ROW - 1) * BLOCK_HEIGHT;

	//背景フレーム(1マス分)の頂点バッファ
	float width = BLOCK_WIDTH / 2;
	float height = BLOCK_HEIGHT / 2;
	Vertex vtx[] =
	{
		{ {-width, -height, 0.0f}, {0.0f, 0.0f} },
		{ {-width,  height, 0.0f}, {0.0f, 1.0f} },
		{ { width, -height, 0.0f}, {1.0f, 0.0f} },
		{ { width,  height, 0.0f}, {1.0f, 1.0f} }
	};
	m_pFrameBuf = CreateVertexBuffer(GetDevice(), vtx, _countof(vtx));

	HRESULT hr = LoadTextureFromFile(GetDevice(), "Image/Field/Frame.png", &m_pFrameTex);
	if (FAILED(hr))
	{
		MessageBox(NULL, "FIELD Texture failed.", "Error", MB_OK);
	}

	m_pBlockDestroySE = LoadSound("Sound/SE/BlockErase.wav");
}

Field::~Field()
{
	if(m_pChain)
	{
		delete m_pChain;
		m_pChain = nullptr;
	}

	if(m_pNextTsumo)
	{
		delete m_pNextTsumo;
		m_pNextTsumo = nullptr;
	}

	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			delete m_grid[y][x];
			m_grid[y][x] = nullptr;
		}
	}

	//操作中のペアは盤面に入っていないので別に削除する
	for (int i = 0; i < PAIR_NUM; ++i)
	{
		delete m_pairBlock[i];
		m_pairBlock[i] = nullptr;
	}

	if (m_pFrameTex)
	{
		m_pFrameTex->Release();
		m_pFrameTex = nullptr;
	}

	if (m_pFrameBuf)
	{
		m_pFrameBuf->Release();
		m_pFrameBuf = nullptr;
	}

	Block::ReleaseResources();
}


//============================================================
// Update / Draw
//============================================================

void Field::Update()
{
	m_pChain->Update();

	//落下・消滅アニメーションの進行
	if (m_state == Field::IDLE || m_state == Field::DESTROY)
	{
		UpdateBlocks();
	}

	switch (m_state)
	{
	case Field::CREATE:		UpdateCreate();		break;
	case Field::IDLE:		UpdateIdle();		break;
	case Field::CHECK:		UpdateCheck();		break;
	case Field::DESTROY:	UpdateDestroy();	break;
	case Field::GAMEOVER:						break;
	}
}

void Field::Draw()
{
	//背景フレーム
	SetSpriteTexture(m_pFrameTex);
	SetSpriteScale(1.0f, 1.0f);

	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			float2 pos = IndexToPos({ x, y });
			SetSpritePos(pos.x, pos.y);
			DrawSprite(m_pFrameBuf);
		}
	}

	//盤面のブロック
	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			if (m_grid[y][x] != nullptr)
			{
				m_grid[y][x]->Draw();
			}
		}
	}

	//操作中のペア(生成直後は盤面の1マス上にいるため、m_gridとは別に描画する)
	for (int i = 0; i < PAIR_NUM; ++i)
	{
		if (m_pairBlock[i] != nullptr)
		{
			m_pairBlock[i]->Draw();
		}
	}

	//連鎖数の表示
	m_pChain->Draw();

	//次のツモの表示
	m_pNextTsumo->Draw();

	//フィールド枠
	m_pFrame->Draw();

	//リセット
	SetSpriteColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetSpriteScale(1.0f, 1.0f);
}

Field::State Field::GetState() const
{
	return m_state;
}

int Field::GetChainCount() const
{
	return m_pChain->GetCount();
}


//============================================================
// ステート別の更新処理
//============================================================

//ブロックの生成
void Field::UpdateCreate()
{
	const int spawnX = FIELD_COLUMN / 2;

	//生成位置(row0)にブロックがある場合はゲームオーバー
	if (m_grid[0][spawnX] != nullptr)
	{
		m_state = Field::GAMEOVER;
		return;
	}

	for (int i = 0; i < PAIR_NUM; ++i)
	{
		m_pairBlock[i] = new Block(m_nextColor[i]);
	}

	//軸はフィールド最上段(row0)、もう一方はその1マス上(row-1)から開始
	m_pairIndex[PIVOT] = { spawnX, 0 };
	m_pairIndex[SUB] = { spawnX, -1 };
	ApplyPairPos();

	//次のツモを生成する
	for (int i = 0; i < PAIR_NUM; ++i)
	{
		m_nextColor[i] = m_colorGen.Next();
	}
	m_pNextTsumo->SetColor(m_nextColor[PIVOT], m_nextColor[SUB]);

	m_fallTimer = 0;
	m_quickTurnDirection = 0;
	m_state = Field::IDLE;
}

//操作・落下待機
void Field::UpdateIdle()
{
	//操作中のペアがあれば、入力と自動落下を処理する
	if (HasPair())
	{
		UpdatePair();
		return;
	}

	//浮いているブロックがあれば落下させ、全て落ち切ったら消去判定へ
	if (StartFalling())
		return;

	m_state = Field::CHECK;
}

//消去判定
void Field::UpdateCheck()
{
	bool visited[FIELD_ROW][FIELD_COLUMN] = {};
	Index group[FIELD_ROW * FIELD_COLUMN];
	bool isDestroy = false;

	//消去されたブロックの座標の平均を求めるための変数
	float sumX = 0.0f;
	float sumY = 0.0f;
	int totalErased = 0;

	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			if (m_grid[y][x] == nullptr || visited[y][x])
				continue;

			//同じ色でつながっているブロックを集め、規定数以上なら消す
			int count = CollectSameColor(x, y, visited, group);
			if (count < BLOCK_ERACE_NUM)
				continue;

			for (int i = 0; i < count; ++i)
			{
				m_grid[group[i].y][group[i].x]->StartDestroy();

				float2 pos = IndexToPos(group[i]);
				sumX += pos.x;
				sumY += pos.y;
				++totalErased;
			}
			isDestroy = true;
		}
	}

	if (isDestroy)
	{
		//連鎖数を増やす
		m_pChain->Add({ sumX / totalErased, sumY / totalErased });
		IXAudio2SourceVoice* pVoice = PlaySound(m_pBlockDestroySE, 0.6f);
		if (pVoice)
		{
			float ratio = 1.0f + 0.15f * (m_pChain->GetCount());
			pVoice->SetFrequencyRatio(std::min(ratio, 2.0f));

			m_state = Field::DESTROY;
		}
		else
		{
			m_state = Field::CREATE;
		}
	}
}

//消去アニメーション
void Field::UpdateDestroy()
{
	bool isAnimating = false;

	for (int y = 0; y < FIELD_ROW; ++y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			Block* pBlock = m_grid[y][x];

			if (pBlock == nullptr)
				continue;

			if (pBlock->IsErased())
			{
				delete pBlock;
				m_grid[y][x] = nullptr;
			}
			else if (pBlock->IsDestroying())
			{
				isAnimating = true;
			}
		}
	}

	//全て消えたらIDLEへ戻る(上に残ったブロックはUpdateIdleで落下させる)
	if (!isAnimating)
	{
		m_state = Field::IDLE;
	}
}


//============================================================
// 盤面上のブロック
//============================================================

//盤面のブロックを更新する(下の段から順に処理する)
void Field::UpdateBlocks()
{
	for (int y = FIELD_ROW - 1; y >= 0; --y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			Block* pBlock = m_grid[y][x];

			if (pBlock == nullptr)
				continue;

			pBlock->Update();

			if (pBlock->IsFalling())
			{
				UpdateFallBlock(x, y);
			}
		}
	}
}

//落下中ブロックが着地したか、次のマスへ進むかを判定する
void Field::UpdateFallBlock(int x, int y)
{
	Block* pBlock = m_grid[y][x];

	//一番下なら着地
	if (y + 1 >= FIELD_ROW)
	{
		pBlock->Land(IndexToPos({ x, y }));
		return;
	}

	//下にブロックがある場合
	//  止まっているブロックなら着地
	//  落下中のブロックなら、一緒に落ちるためそのまま待つ
	Block* pBelow = m_grid[y + 1][x];

	if (pBelow != nullptr)
	{
		if (!pBelow->IsFalling())
		{
			pBlock->Land(IndexToPos({ x, y }));
		}
		return;
	}

	//下のマスの位置まで進んだら、盤面上でも1マス下へ移す
	float2 belowPos = IndexToPos({ x, y + 1 });

	if (pBlock->HasReachedY(belowPos.y))
	{
		m_grid[y][x] = nullptr;
		m_grid[y + 1][x] = pBlock;
		pBlock->SetPos(belowPos);
	}
}

//下が空いているブロックを落下させる。落下中のブロックがあればtrueを返す
bool Field::StartFalling()
{
	bool hasFall = false;

	//下の段から順に見ることで、落下するブロックの上に積まれたブロックも同時に落とす
	for (int y = FIELD_ROW - 1; y >= 0; --y)
	{
		for (int x = 0; x < FIELD_COLUMN; ++x)
		{
			Block* pBlock = m_grid[y][x];

			if (pBlock == nullptr)
				continue;

			if (pBlock->IsFalling())
			{
				hasFall = true;
				continue;
			}

			if (y + 1 >= FIELD_ROW)
				continue;

			Block* pBelow = m_grid[y + 1][x];

			if (pBelow == nullptr || pBelow->IsFalling())
			{
				pBlock->StartFall();
				hasFall = true;
			}
		}
	}

	return hasFall;
}

//同じ色でつながっているブロックを幅優先で集め、個数を返す
//pOutには集めたマスが入る(FIELD_ROW * FIELD_COLUMN個分の領域が必要)
int Field::CollectSameColor(int startX, int startY,
	bool visited[FIELD_ROW][FIELD_COLUMN], Index* pOut) const
{
	static const int dx[4] = { -1, 1, 0, 0 };
	static const int dy[4] = { 0, 0, -1, 1 };

	int count = 0;
	pOut[count++] = { startX, startY };
	visited[startY][startX] = true;

	//pOutをそのまま探索待ちのキューとして使う
	for (int head = 0; head < count; ++head)
	{
		Index current = pOut[head];
		const Block* pCurrent = m_grid[current.y][current.x];

		for (int i = 0; i < 4; ++i)
		{
			int nx = current.x + dx[i];
			int ny = current.y + dy[i];

			if (nx < 0 || nx >= FIELD_COLUMN ||
				ny < 0 || ny >= FIELD_ROW)
				continue;

			if (visited[ny][nx])
				continue;

			if (!pCurrent->IsSameColor(m_grid[ny][nx]))
				continue;

			visited[ny][nx] = true;
			pOut[count++] = { nx, ny };
		}
	}

	return count;
}


//============================================================
// 操作中のペア
//============================================================

bool Field::HasPair() const
{
	return m_pairBlock[PIVOT] != nullptr;
}

//キー入力と自動落下
void Field::UpdatePair()
{
	//左右移動(同時押しは左を優先)
	if (isKeyRepeat(VK_LEFT) || isKeyTrigger(VK_LEFT))
	{
		MovePair(-1, 0);
	}
	else if (isKeyRepeat(VK_RIGHT) || isKeyTrigger(VK_RIGHT))
	{
		MovePair(1, 0);
	}

	//回転
	if (isKeyTrigger('X'))
	{
		RotatePair(1);
	}

	if (isKeyTrigger('Z'))
	{
		RotatePair(-1);
	}

	//ハードドロップ / ソフトドロップ(着地するとペアがなくなるため、ここで終了)
	if (isKeyTrigger(VK_UP))
	{
		HardDrop();
		return;
	}

	if (isKeyTrigger(VK_DOWN))
	{
		StepDown();
		return;
	}

	//一定時間ごとに1マス自動落下
	++m_fallTimer;

	if (m_fallTimer >= BLOCK_MOVE_WAIT_TIME)
	{
		m_fallTimer = 0;
		StepDown();
	}
}

//ペアを(dx, dy)だけ動かしたとき、2個とも置けるか
bool Field::CanPlacePair(int dx, int dy) const
{
	for (int i = 0; i < PAIR_NUM; ++i)
	{
		if (!IsCellFree(m_pairIndex[i].x + dx, m_pairIndex[i].y + dy))
			return false;
	}

	return true;
}

//ペアを(dx, dy)だけ動かす(動かせない場合は何もしない)
void Field::MovePair(int dx, int dy)
{
	if (!CanPlacePair(dx, dy))
		return;

	for (int i = 0; i < PAIR_NUM; ++i)
	{
		m_pairIndex[i].x += dx;
		m_pairIndex[i].y += dy;
	}

	//横に動いたら、回転できなかった記憶は無効にする
	if (dx != 0)
	{
		m_quickTurnDirection = 0;
	}

	ApplyPairPos();
}

//ペアを回転させる(direction: 1=右回転, -1=左回転)
void Field::RotatePair(int direction)
{
	const Index pivot = m_pairIndex[PIVOT];
	const Index sub = m_pairIndex[SUB];

	int dx = sub.x - pivot.x;
	int dy = sub.y - pivot.y;
	bool isVertical = (dx == 0);

	//クイックターン
	//縦向きで左右とも塞がって回転できなかった直後に、同じ方向へもう一度入力すると
	//2個の位置を入れ替える(180度回転)
	if (isVertical && m_quickTurnDirection == direction)
	{
		std::swap(m_pairIndex[PIVOT], m_pairIndex[SUB]);
		m_quickTurnDirection = 0;
		ApplyPairPos();
		return;
	}

	//軸を中心に、もう一方を90度回した位置を求める
	int newDx = (direction > 0) ? -dy : dy;
	int newDy = (direction > 0) ? dx : -dx;

	Index newPivot = pivot;
	Index newSub = { pivot.x + newDx, pivot.y + newDy };

	//壁にめり込む場合は、ペアごと横にずらす(壁蹴り)
	int kick = 0;

	if (newSub.x < 0)
		kick = 1;
	else if (newSub.x >= FIELD_COLUMN)
		kick = -1;

	newPivot.x += kick;
	newSub.x += kick;

	//回転できる場合
	if (IsCellFree(newPivot.x, newPivot.y) &&
		IsCellFree(newSub.x, newSub.y))
	{
		m_pairIndex[PIVOT] = newPivot;
		m_pairIndex[SUB] = newSub;
		m_quickTurnDirection = 0;
		ApplyPairPos();
		return;
	}

	//回転できなかった場合、縦向きで左右とも塞がっているときだけクイックターンを受け付ける
	bool isSideBlocked =
		!IsCellFree(pivot.x - 1, pivot.y) &&
		!IsCellFree(pivot.x + 1, pivot.y);

	m_quickTurnDirection = (isVertical && isSideBlocked) ? direction : 0;
}

//ペアを一番下まで落として着地させる
void Field::HardDrop()
{
	while (CanPlacePair(0, 1))
	{
		MovePair(0, 1);
	}

	LockPair();
}

//ペアを1マス落とす。落とせなければ着地させる
void Field::StepDown()
{
	if (CanPlacePair(0, 1))
	{
		MovePair(0, 1);
	}
	else
	{
		LockPair();
	}
}

//ペアを盤面(m_grid)へ移して操作を終える
void Field::LockPair()
{
	//リセット
	m_pChain->Reset();

	//盤面の上にはみ出したまま止まった場合はゲームオーバー
	for (int i = 0; i < PAIR_NUM; ++i)
	{
		if (m_pairIndex[i].y < 0)
		{
			m_state = Field::GAMEOVER;
			return;
		}
	}

	for (int i = 0; i < PAIR_NUM; ++i)
	{
		Index index = m_pairIndex[i];

		m_pairBlock[i]->Land(IndexToPos(index));
		m_grid[index.y][index.x] = m_pairBlock[i];
		m_pairBlock[i] = nullptr;
	}

	m_fallTimer = 0;
	m_quickTurnDirection = 0;
}

//ペアのマス目をブロックの表示位置へ反映する
void Field::ApplyPairPos()
{
	for (int i = 0; i < PAIR_NUM; ++i)
	{
		m_pairBlock[i]->SetPos(IndexToPos(m_pairIndex[i]));
	}
}


//============================================================
// マス目の判定・座標変換
//============================================================

//そのマスにブロックを置けるか
bool Field::IsCellFree(int x, int y) const
{
	if (x < 0 || x >= FIELD_COLUMN || y >= FIELD_ROW)
		return false;

	//盤面の1マス上(生成位置)までは空きとして扱う。それより上は不可
	if (y < 0)
		return y >= -1;

	return m_grid[y][x] == nullptr;
}

//マス目 → 表示座標
float2 Field::IndexToPos(Index index) const
{
	float2 pos;
	pos.x = index.x * BLOCK_WIDTH + m_offset.x;
	pos.y = index.y * BLOCK_HEIGHT + m_offset.y;
	return pos;
}

