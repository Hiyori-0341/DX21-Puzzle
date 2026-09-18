#include "Field.h"
#include "SpriteDrawer.h"
#include "VertexBuffer.h"
#include "DirectXTex/TextureLoad.h"
#include "Block.h"
#include <iostream>
#include <ctime>
#include "Input/Keyboard.h"


//============================================================
// コンストラクタ
//============================================================

Field::Field()
    : m_pFrameBuf(nullptr)
    , m_pFrameTex(nullptr)
    , m_grid{}
    , m_offset{}
    , m_state(Field::State::CREATE)
    , m_check{}
    , m_isMoveRight(false)
    , m_moveInputHandled(false)
    , m_horizontalInputHandled(false)
    , m_pivotBlock(nullptr)
    , m_pBlockDestroySE(nullptr)
	, m_fallTimer(0)
	, m_rotateFailed(false)
	, m_rotateFailedDirection(0)
	, m_isSpawning(false)
	, m_spawnBlock{ nullptr, nullptr }
{
    //============================================================
    // フィールドの表示位置
    //============================================================

    m_offset.x =
        0.5f * (FIELD_COLUMN - 1.0f) * BLOCK_WIDTH;

    m_offset.y =
        0.5f * (FIELD_ROW - 1.0f) * BLOCK_HEIGHT;

    m_offset.x = -m_offset.x;
    m_offset.y = -m_offset.y;


    //============================================================
    // フィールド枠の頂点バッファ
    //============================================================

    float width = BLOCK_WIDTH / 2;
    float height = BLOCK_HEIGHT / 2;

    Vertex vtx[] =
    {
        { {-width, -height, 0.0f}, {0.0f, 0.0f} },
        { {-width,  height, 0.0f}, {0.0f, 1.0f} },
        { { width, -height, 0.0f}, {1.0f, 0.0f} },
        { { width,  height, 0.0f}, {1.0f, 1.0f} }
    };

    m_pFrameBuf =
        CreateVertexBuffer(
            GetDevice(),
            vtx,
            _countof(vtx));


    //============================================================
    // フィールドテクスチャ
    //============================================================

    const char* texture =
        "Image/Field/Frame.png";

    HRESULT hr =
        LoadTextureFromFile(
            GetDevice(),
            texture,
            &m_pFrameTex);

    if (FAILED(hr))
    {
        MessageBox(
            NULL,
            "FIELD Texture failed.",
            "Error",
            MB_OK);
    }


    //============================================================
    // グリッド初期化
    //============================================================

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            m_grid[y][x] = nullptr;
        }
    }


    //============================================================
    // SE
    //============================================================

    m_pBlockDestroySE =
        LoadSound("Sound/SE/BlockErase.wav");
}


//============================================================
// デストラクタ
//============================================================

Field::~Field()
{
    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            if (m_grid[y][x] != nullptr)
            {
                delete m_grid[y][x];
                m_grid[y][x] = nullptr;
            }
        }
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
}


//============================================================
// Update
//============================================================

void Field::Update()
{
    //============================================================
    // 入力処理済みフラグを毎フレームリセット
    //============================================================

    m_moveInputHandled = false;
    m_horizontalInputHandled = false;





    if (m_state == Field::State::IDLE ||
        m_state == Field::State::DESTROY)
    {
        Block* updateList[FIELD_ROW * FIELD_COLUMN];

        int updateCount = 0;

        for (int y = 0; y < FIELD_ROW; ++y)
        {
            for (int x = 0; x < FIELD_COLUMN; ++x)
            {
                Block* pBlock = m_grid[y][x];

                if (pBlock == nullptr)
                    continue;

                // 同じBlockが複数セルに入っている場合の重複更新防止
                bool alreadyAdded = false;

                for (int i = 0; i < updateCount; ++i)
                {
                    if (updateList[i] == pBlock)
                    {
                        alreadyAdded = true;
                        break;
                    }
                }

                if (!alreadyAdded)
                {
                    updateList[updateCount] = pBlock;
                    ++updateCount;
                }
            }
        }


        // Block更新
        for (int i = 0; i < updateCount; ++i)
        {
            if (updateList[i] != nullptr)
            {
                updateList[i]->Update();
            }
        }
    }


    //============================================================
    // Block::UpdateMove() によって変更された座標を
    // m_gridと同期する
    //============================================================

    if (m_state == Field::State::IDLE)
    {
        for (int y = FIELD_ROW - 1; y >= 0; --y)
        {
            if (m_isMoveRight)
            {
                for (int x = FIELD_COLUMN - 1; x >= 0; --x)
                {
                    syncBlock(x, y);
                }
            }
            else
            {
                for (int x = 0; x < FIELD_COLUMN; ++x)
                {
                    syncBlock(x, y);
                }
            }
        }
    }


    //============================================================
    // FALL状態のブロックを処理
    //============================================================

    if (m_state == Field::State::IDLE ||
        m_state == Field::State::DESTROY)
    {
        for (int y = FIELD_ROW - 1; y >= 0; --y)
        {
            for (int x = 0; x < FIELD_COLUMN; ++x)
            {
                if (m_grid[y][x] == nullptr)
                    continue;

                if (m_grid[y][x]->GetState()
                    == Block::State::FALL)
                {
                    UpdateFallBlock(x, y);
                }
            }
        }
    }


    //============================================================
    // Fieldステート処理
    //============================================================

    switch (m_state)
    {
    case Field::State::CREATE:
        UpdateCreate();
        break;

	case Field::State::SPAWN:
		UpdateSpawn();
		break;

    case Field::State::IDLE:
        UpdateIdle();
        break;

    case Field::State::CHECK:
        UpdateCheck();
        break;

    case Field::State::DESTROY:
        UpdateDestroy();
        break;

    case Field::State::GAMEOVER:
        UpdateGameOver();
        break;
    }
}


//============================================================
// Draw
//============================================================

void Field::Draw()
{
    SetSpriteTexture(m_pFrameTex);
    SetSpriteScale(1.0f, 1.0f);

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            SetSpritePos(
                BLOCK_WIDTH * x + m_offset.x,
                BLOCK_HEIGHT * y + m_offset.y);

            DrawSprite(m_pFrameBuf);
        }
    }


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

    for (int i = 0; i < 2; ++i)
    {
        if (m_spawnBlock[i] != nullptr)
        {
            m_spawnBlock[i]->Draw();
        }
    }


    SetSpriteColor(
        1.0f,
        1.0f,
        1.0f,
        1.0f);

    SetSpriteScale(1.0f, 1.0f);
}


//============================================================
// 移動方向
//============================================================

bool Field::IsMoveRight()
{
    return m_isMoveRight;
}


void Field::SetMoveRight(bool isMoveRight)
{
    m_isMoveRight = isMoveRight;
}


//============================================================
// Hard Drop
//============================================================

void Field::HardDrop()
{
    if (m_moveInputHandled)
        return;

    m_moveInputHandled = true;


    Block* moveBlock[2] =
    {
        nullptr,
        nullptr
    };

    int blockX[2] =
    {
        -1,
        -1
    };

    int blockY[2] =
    {
        -1,
        -1
    };


    //============================================================
    // MOVEブロックを取得
    //============================================================

    int blockCount = 0;

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            if (m_grid[y][x] == nullptr)
                continue;

            if (m_grid[y][x]->GetState()
                != Block::State::MOVE)
                continue;

            moveBlock[blockCount] =
                m_grid[y][x];

            blockX[blockCount] = x;
            blockY[blockCount] = y;

            ++blockCount;

            if (blockCount == 2)
                break;
        }

        if (blockCount == 2)
            break;
    }


    // 2個揃っていない場合は何もしない
    if (blockCount != 2)
        return;


    //============================================================
    // 落下可能距離を計算
    //============================================================

    int dropDist = FIELD_ROW;

    for (int i = 0; i < 2; ++i)
    {
        int x = blockX[i];
        int y = blockY[i];

        int dist = 0;

        while (y + dist + 1 < FIELD_ROW)
        {
            int checkY =
                y + dist + 1;

            Block* pBlock =
                m_grid[checkY][x];

            // 自分たちのブロックは無視
            if (pBlock == moveBlock[0] ||
                pBlock == moveBlock[1])
            {
                ++dist;
                continue;
            }

            // 他のブロックに衝突
            if (pBlock != nullptr)
            {
                break;
            }

            ++dist;
        }

        if (dist < dropDist)
        {
            dropDist = dist;
        }
    }


    //============================================================
    // 元の位置を空にする
    //============================================================

    m_grid[blockY[0]][blockX[0]] = nullptr;
    m_grid[blockY[1]][blockX[1]] = nullptr;


    //============================================================
    // 新しい位置へ移動
    //============================================================

    for (int i = 0; i < 2; ++i)
    {
        int newX = blockX[i];
        int newY = blockY[i] + dropDist;

        m_grid[newY][newX] =
            moveBlock[i];

        Index index;

        index.x = newX;
        index.y = newY;

        moveBlock[i]->SetPos(
            IndexToPos(index));

        moveBlock[i]->ResetFallSpeed();
        moveBlock[i]->SetState(
            Block::State::IDLE);
    }


    //============================================================
    // 操作終了
    //============================================================

    m_pivotBlock = nullptr;


    //============================================================
    // 空中に残っているブロックをFALLへ
    //============================================================

    for (int y = FIELD_ROW - 2; y >= 0; --y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock =
                m_grid[y][x];

            if (pBlock == nullptr)
                continue;

            if (pBlock->GetState()
                == Block::State::MOVE)
                continue;

            if (m_grid[y + 1][x] == nullptr)
            {
                pBlock->SetState(
                    Block::State::FALL);

                pBlock->ResetFallSpeed();
            }
        }
    }
}


//============================================================
// Soft Drop
//============================================================

void Field::SoftDrop()
{
    if (m_moveInputHandled)
        return;

    m_moveInputHandled = true;


    Block* moveBlock[2] =
    {
        nullptr,
        nullptr
    };

    int blockX[2] =
    {
        -1,
        -1
    };

    int blockY[2] =
    {
        -1,
        -1
    };


    //============================================================
    // MOVEブロック取得
    //============================================================

    int blockCount = 0;

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            if (m_grid[y][x] == nullptr)
                continue;

            if (m_grid[y][x]->GetState()
                != Block::State::MOVE)
                continue;

            moveBlock[blockCount] =
                m_grid[y][x];

            blockX[blockCount] = x;
            blockY[blockCount] = y;

            ++blockCount;

            if (blockCount == 2)
                break;
        }

        if (blockCount == 2)
            break;
    }


    if (blockCount != 2)
        return;


    //============================================================
    // 2個とも下へ移動可能か確認
    //============================================================

    bool canDrop = true;

    for (int i = 0; i < 2; ++i)
    {
        int x = blockX[i];
        int y = blockY[i];

        if (y + 1 >= FIELD_ROW)
        {
            canDrop = false;
            break;
        }

        Block* below =
            m_grid[y + 1][x];

        if (below != nullptr &&
            below != moveBlock[0] &&
            below != moveBlock[1])
        {
            canDrop = false;
            break;
        }
    }


    //============================================================
    // 落下できない
    //============================================================

    if (!canDrop)
    {
        moveBlock[0]->SetState(
            Block::State::IDLE);

        moveBlock[1]->SetState(
            Block::State::IDLE);

        moveBlock[0]->ResetFallSpeed();
        moveBlock[1]->ResetFallSpeed();

        m_pivotBlock = nullptr;

        return;
    }


    //============================================================
    // 元の位置を空にする
    //============================================================

    m_grid[blockY[0]][blockX[0]] = nullptr;
    m_grid[blockY[1]][blockX[1]] = nullptr;


    //============================================================
    // 1マス下へ
    //============================================================

    for (int i = 0; i < 2; ++i)
    {
        int newX = blockX[i];
        int newY = blockY[i] + 1;

        m_grid[newY][newX] =
            moveBlock[i];

        Index index;

        index.x = newX;
        index.y = newY;

        moveBlock[i]->SetPos(
            IndexToPos(index));

        moveBlock[i]->ResetFallSpeed();

        moveBlock[i]->SetState(
            Block::State::MOVE);
    }
}


//============================================================
// 横移動
//============================================================

void Field::MoveHorizontal(int direction)
{
    if (m_horizontalInputHandled)
        return;

    m_horizontalInputHandled = true;


    //============================================================
    // 生成中のブロックを移動
    //
    // 生成中はまだm_gridに登録されていないため、
    // m_spawnBlockを直接移動する
    //============================================================

    if (m_isSpawning)
    {
        if (m_spawnBlock[0] == nullptr ||
            m_spawnBlock[1] == nullptr)
        {
            return;
        }


        //========================================================
        // 現在位置
        //========================================================

        float2 pos0 =
            m_spawnBlock[0]->GetPos();

        float2 pos1 =
            m_spawnBlock[1]->GetPos();


        //========================================================
        // 移動後のX座標
        //========================================================

        float newX0 =
            pos0.x + BLOCK_WIDTH * direction;

        float newX1 =
            pos1.x + BLOCK_WIDTH * direction;


        //========================================================
        // 左右の範囲チェック
        //========================================================

        float minX =
            m_offset.x;

        float maxX =
            m_offset.x +
            (FIELD_COLUMN - 1) * BLOCK_WIDTH;


        if (newX0 < minX ||
            newX0 > maxX ||
            newX1 < minX ||
            newX1 > maxX)
        {
            return;
        }


        //========================================================
        // 既存ブロックとの衝突確認
        //========================================================

        for (int i = 0; i < 2; ++i)
        {
            float2 pos;

            if (i == 0)
            {
                pos = pos0;
                pos.x = newX0;
            }
            else
            {
                pos = pos1;
                pos.x = newX1;
            }


            // フィールド外なら衝突判定しない
            if (pos.y < m_offset.y)
            {
                continue;
            }


            int x =
                (int)((pos.x - m_offset.x +
                    BLOCK_WIDTH * 0.5f) /
                    BLOCK_WIDTH);

            int y =
                (int)((pos.y - m_offset.y +
                    BLOCK_HEIGHT * 0.5f) /
                    BLOCK_HEIGHT);


            if (x < 0 ||
                x >= FIELD_COLUMN ||
                y < 0 ||
                y >= FIELD_ROW)
            {
                continue;
            }


            if (m_grid[y][x] != nullptr)
            {
                return;
            }
        }


        //========================================================
        // 実際に移動
        //========================================================

        pos0.x = newX0;
        pos1.x = newX1;

        m_spawnBlock[0]->SetPos(pos0);
        m_spawnBlock[1]->SetPos(pos1);

        m_isMoveRight =
            direction > 0;

        return;
    }


    //============================================================
    // ここから下は今までのMoveHorizontal()をそのまま残す
    //============================================================

    // MOVEブロック取得

    Block* moveBlock[2] =
    {
        nullptr,
        nullptr
    };

    int blockX[2] =
    {
        -1,
        -1
    };

    int blockY[2] =
    {
        -1,
        -1
    };


    int blockCount = 0;

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            if (m_grid[y][x] == nullptr)
                continue;

            if (m_grid[y][x]->GetState()
                != Block::State::MOVE)
                continue;

            moveBlock[blockCount] =
                m_grid[y][x];

            blockX[blockCount] = x;
            blockY[blockCount] = y;

            ++blockCount;

            if (blockCount == 2)
                break;
        }

        if (blockCount == 2)
            break;
    }


    if (blockCount != 2)
        return;


    //============================================================
    // ここから先は既存の処理
    //============================================================

    int targetX[2];

    bool canMove = true;

    for (int i = 0; i < 2; ++i)
    {
        targetX[i] =
            blockX[i] + direction;

        if (targetX[i] < 0 ||
            targetX[i] >= FIELD_COLUMN)
        {
            canMove = false;
            break;
        }

        Block* targetBlock =
            m_grid[blockY[i]][targetX[i]];

        if (targetBlock != nullptr &&
            targetBlock != moveBlock[0] &&
            targetBlock != moveBlock[1])
        {
            canMove = false;
            break;
        }
    }


    if (!canMove)
        return;


    // 元の位置を空にする

    for (int i = 0; i < 2; ++i)
    {
        m_grid[blockY[i]][blockX[i]] =
            nullptr;
    }


    // 新しい位置へ移動

    for (int i = 0; i < 2; ++i)
    {
        m_grid[blockY[i]][targetX[i]] =
            moveBlock[i];

        Index index;

        index.x = targetX[i];
        index.y = blockY[i];

        moveBlock[i]->SetPos(
            IndexToPos(index));
    }


    m_isMoveRight =
        direction > 0;
}

//============================================================
// Block生成
//============================================================

void Field::UpdateCreate()
{
    int x = FIELD_COLUMN / 2;

    //============================================================
    // 出現位置チェック
    //
    // ここではまだm_gridへ登録しない。
    // 2個ともフィールド内へ入れるスペースがあるか確認する。
    //============================================================

    if (m_grid[0][x] != nullptr &&
        m_grid[1][x] != nullptr)
    {
        m_state = Field::State::GAMEOVER;
        return;
    }


    //============================================================
    // ブロック生成
    //============================================================

    m_spawnBlock[0] =
        new Block(
            rand() % BLOCK_COLOR_NUM,
            this);

    m_spawnBlock[1] =
        new Block(
            rand() % BLOCK_COLOR_NUM,
            this);


    //============================================================
    // 回転中心
    //============================================================

    m_pivotBlock =
        m_spawnBlock[1];


    //============================================================
    // フィールド外に生成
    //============================================================

    float2 pos0;

    pos0.x =
        x * BLOCK_WIDTH + m_offset.x;

    pos0.y =
        m_offset.y - BLOCK_HEIGHT * 2.0f;


    float2 pos1;

    pos1.x =
        x * BLOCK_WIDTH + m_offset.x;

    pos1.y =
        m_offset.y - BLOCK_HEIGHT;


    m_spawnBlock[0]->SetPos(pos0);
    m_spawnBlock[1]->SetPos(pos1);


    //============================================================
    // MOVE状態にする
    //
    // m_gridにはまだ登録しない
    //============================================================

    m_spawnBlock[0]->SetState(
        Block::State::MOVE);

    m_spawnBlock[1]->SetState(
        Block::State::MOVE);


    //============================================================
    // 生成中
    //============================================================

    m_isSpawning = true;

    m_fallTimer = 0;

    m_state =
        Field::State::SPAWN;
}

//============================================================
// Idle
//============================================================

void Field::UpdateIdle()
{
    //============================================================
    // 回転
    //============================================================

    if (isKeyTrigger('X'))
    {
        RotateBlock(1);
    }

    if (isKeyTrigger('Z'))
    {
        RotateBlock(-1);
    }


    //============================================================
    // MOVEブロックの数を確認
    //============================================================

    int moveCount = 0;

    Block* moveBlock[2] =
    {
        nullptr,
        nullptr
    };

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock = m_grid[y][x];

            if (pBlock == nullptr)
                continue;

            if (pBlock->GetState() != Block::State::MOVE)
                continue;

            if (moveCount < 2)
            {
                moveBlock[moveCount] = pBlock;
            }

            ++moveCount;
        }
    }


    //============================================================
    // MOVEブロックが2個ある場合
    // → 現在操作中なので通常の操作を継続
    //============================================================

    if (moveCount >= 2)
    {
        //============================================================
        // 操作中ブロックの自動落下
        //============================================================

        ++m_fallTimer;

        if (m_fallTimer >= BLOCK_MOVE_WAIT_TIME)
        {
            m_fallTimer = 0;

            // 1マス下に移動できるか確認
            bool canDrop = true;

            for (int i = 0; i < 2; ++i)
            {
                float2 pos = moveBlock[i]->GetPos();
                Index index = PosToIndex(pos);

                if (index.y + 1 >= FIELD_ROW)
                {
                    canDrop = false;
                    break;
                }

                Block* below = m_grid[index.y + 1][index.x];

                // 2個の操作中ブロック自身なら問題なし
                if (below != nullptr &&
                    below != moveBlock[0] &&
                    below != moveBlock[1])
                {
                    canDrop = false;
                    break;
                }
            }

            if (canDrop)
            {
                //====================================================
                // 現在位置を一旦削除
                //====================================================

                for (int i = 0; i < 2; ++i)
                {
                    Index index = PosToIndex(moveBlock[i]->GetPos());

                    m_grid[index.y][index.x] = nullptr;
                }

                //====================================================
                // 1マス下へ移動
                //====================================================

                for (int i = 0; i < 2; ++i)
                {
                    Index index = PosToIndex(moveBlock[i]->GetPos());

                    index.y++;

                    m_grid[index.y][index.x] = moveBlock[i];

                    moveBlock[i]->SetPos(IndexToPos(index));
                    moveBlock[i]->ResetFallSpeed();
                }
            }
            else
            {
                //====================================================
                // 下に移動できないので着地
                //====================================================

                moveBlock[0]->SetState(Block::State::IDLE);
                moveBlock[1]->SetState(Block::State::IDLE);

                moveBlock[0]->ResetFallSpeed();
                moveBlock[1]->ResetFallSpeed();

                m_pivotBlock = nullptr;
            }
        }

        return;
    }


    //============================================================
    // MOVEが1個だけ残ってしまった場合
    //============================================================

    if (moveCount == 1)
    {
        moveBlock[0]->SetState(
            Block::State::IDLE);

        moveBlock[0]->ResetFallSpeed();

        m_pivotBlock = nullptr;

        m_state =
            Field::State::CHECK;

        return;
    }


    //============================================================
    // ★重要
    //
    // IDLEなのに下が空いているブロックを
    // 毎フレームFALLへ変更する
    //
    // これによって消去後の連鎖落下を確実に行う。
    //============================================================

    bool hasFall = false;

    for (int y = FIELD_ROW - 2; y >= 0; --y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock = m_grid[y][x];

            if (pBlock == nullptr)
                continue;


            // MOVEは操作中なので対象外
            if (pBlock->GetState() == Block::State::MOVE)
                continue;


            // DESTROYは対象外
            if (pBlock->GetState() == Block::State::DESTROY)
                continue;


            // すでにFALL中
            if (pBlock->GetState() == Block::State::FALL)
            {
                hasFall = true;
                continue;
            }


            //====================================================
            // 真下が空いている
            //====================================================

            if (m_grid[y + 1][x] == nullptr)
            {
                pBlock->SetState(
                    Block::State::FALL);

                pBlock->ResetFallSpeed();

                hasFall = true;
            }
        }
    }


    //============================================================
    // FALL中のブロックが残っているなら
    // まだCHECKへ進まない
    //============================================================

    if (hasFall)
    {
        return;
    }


    //============================================================
    // 全ブロックが着地している
    // → 消去チェックへ
    //============================================================

    m_state =
        Field::State::CHECK;
}

//============================================================
// 消去チェック
//============================================================

void Field::UpdateCheck()
{
    m_state =
        Field::State::CREATE;


    //============================================================
    // チェックフラグ初期化
    //============================================================

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            m_check[y][x] = false;
        }
    }


    //============================================================
    // 全ブロック確認
    //============================================================

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            if (m_grid[y][x] == nullptr)
                continue;

            if (m_check[y][x])
                continue;


            Index index;

            index.x = x;
            index.y = y;


            int count =
                RecursiveBlockCount(index);


            if (count >= BLOCK_ERACE_NUM)
            {
                RecursiveBlockDestroy(index);

                m_state =
                    Field::State::DESTROY;

                PlaySound(
                    m_pBlockDestroySE,
                    0.005f);
            }
        }
    }
}


//============================================================
// 消去処理
//============================================================

void Field::UpdateDestroy()
{
    bool isDestroy = false;


    //============================================================
    // ERASE状態のBlockを削除
    //============================================================

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock =
                m_grid[y][x];

            if (pBlock == nullptr)
                continue;


            if (pBlock->GetState()
                == Block::State::ERASE)
            {
                if (pBlock == m_pivotBlock)
                {
                    m_pivotBlock = nullptr;
                }

                delete pBlock;

                m_grid[y][x] = nullptr;
            }
            else if (pBlock->GetState()
                == Block::State::DESTROY)
            {
                isDestroy = true;
            }
        }
    }


    //============================================================
    // 消去アニメーション終了
    //============================================================

    if (!isDestroy)
    {
        //========================================================
        // 空いている場所の上にあるブロックをFALLへ
        //========================================================

        for (int y = FIELD_ROW - 2; y >= 0; --y)
        {
            for (int x = 0; x < FIELD_COLUMN; ++x)
            {
                Block* pBlock =
                    m_grid[y][x];

                if (pBlock == nullptr)
                    continue;

                if (m_grid[y + 1][x] == nullptr)
                {
                    pBlock->SetState(
                        Block::State::FALL);

                    pBlock->ResetFallSpeed();
                }
                else
                {
                    pBlock->SetState(
                        Block::State::IDLE);
                }
            }
        }


        m_state =
            Field::State::IDLE;
    }
}


//============================================================
// GameOver
//============================================================

void Field::UpdateGameOver()
{
}

void Field::UpdateSpawn()
{
    if (!m_isSpawning)
        return;

    //============================================================
    // 生成中の左右移動
    //============================================================

    if (isKeyRepeat(VK_LEFT) || isKeyTrigger(VK_LEFT))
    {
        MoveHorizontal(-1);
    }

    if (isKeyRepeat(VK_RIGHT) || isKeyTrigger(VK_RIGHT))
    {
        MoveHorizontal(1);
    }

    //============================================================
    // 生成中の回転
    //============================================================

    if (isKeyTrigger('X'))
    {
        RotateBlock(1);
    }

    if (isKeyTrigger('Z'))
    {
        RotateBlock(-1);
    }

    //============================================================
    // ★変更：通常の自動落下(m_fallTimer)と同じ速度・同じ1マス刻みで降ろす
    //============================================================

    float targetY[2] =
    {
        m_offset.y,                    // 上側ブロックの目標(row0)
        m_offset.y + BLOCK_HEIGHT      // 下側ブロックの目標(row1)
    };

    bool finished = true;

    for (int i = 0; i < 2; ++i)
    {
        if (m_spawnBlock[i] == nullptr)
            continue;

        float2 pos = m_spawnBlock[i]->GetPos();

        if (pos.y < targetY[i])
        {
            finished = false;
        }
    }

    //まだ目標の行に届いていない場合は、通常落下と同じタイミングで1マス進める
    if (!finished)
    {
        ++m_fallTimer;

        if (m_fallTimer >= BLOCK_MOVE_WAIT_TIME)
        {
            m_fallTimer = 0;

            for (int i = 0; i < 2; ++i)
            {
                if (m_spawnBlock[i] == nullptr)
                    continue;

                float2 pos = m_spawnBlock[i]->GetPos();

                if (pos.y < targetY[i])
                {
                    pos.y += BLOCK_HEIGHT;

                    //行き過ぎないように目標でクランプ
                    if (pos.y > targetY[i])
                        pos.y = targetY[i];

                    m_spawnBlock[i]->SetPos(pos);
                }
            }
        }

        return;
    }

    //============================================================
    // ここから先(フィールドへ登録する処理)は変更なし
    //============================================================

    int gridX[2];
    int gridY[2];

    bool canEnter = true;

    for (int i = 0; i < 2; ++i)
    {
        Index index = PosToIndex(m_spawnBlock[i]->GetPos());

        gridX[i] = index.x;
        gridY[i] = index.y;

        if (gridY[i] < 0 || gridY[i] >= FIELD_ROW ||
            gridX[i] < 0 || gridX[i] >= FIELD_COLUMN)
        {
            canEnter = false;
            break;
        }

        if (IsCellOccupied(gridX[i], gridY[i], m_spawnBlock[0], m_spawnBlock[1]))
        {
            canEnter = false;
            break;
        }
    }

    if (!canEnter)
    {
        m_state = Field::State::GAMEOVER;
        return;
    }

    for (int i = 0; i < 2; ++i)
    {
        m_grid[gridY[i]][gridX[i]] = m_spawnBlock[i];

        m_spawnBlock[i]->SetPos(
            IndexToPos({ gridX[i], gridY[i] }));
    }

    m_spawnBlock[0] = nullptr;
    m_spawnBlock[1] = nullptr;

    m_isSpawning = false;
    m_fallTimer = 0;

    m_state = Field::State::IDLE;
}

//============================================================
// Index → 座標
//============================================================

float2 Field::IndexToPos(Index index)
{
    float2 pos;

    pos.x =
        index.x * BLOCK_WIDTH;

    pos.y =
        index.y * BLOCK_HEIGHT;

    pos.x += m_offset.x;
    pos.y += m_offset.y;

    return pos;
}


//============================================================
// 座標 → Index
//============================================================

Field::Index Field::PosToIndex(float2 pos)
{
    Index index;

    pos.x -= m_offset.x;
    pos.y -= m_offset.y;

    pos.x += BLOCK_WIDTH * 0.5f;
    pos.y += BLOCK_HEIGHT * 0.5f;

    index.x =
        (int)(pos.x / BLOCK_WIDTH);

    index.y =
        (int)(pos.y / BLOCK_HEIGHT);


    // 念のため範囲制限
    if (index.x < 0)
        index.x = 0;

    if (index.x >= FIELD_COLUMN)
        index.x = FIELD_COLUMN - 1;

    if (index.y < 0)
        index.y = 0;

    if (index.y >= FIELD_ROW)
        index.y = FIELD_ROW - 1;


    return index;
}


//============================================================
// 同色ブロック数を再帰的に検索
//============================================================

int Field::RecursiveBlockCount(Index index)
{
    if (index.x < 0 ||
        index.x >= FIELD_COLUMN ||
        index.y < 0 ||
        index.y >= FIELD_ROW)
    {
        return 0;
    }


    if (m_check[index.y][index.x])
        return 0;


    Block* pCenter =
        m_grid[index.y][index.x];

    if (pCenter == nullptr)
    {
        m_check[index.y][index.x] = true;
        return 0;
    }


    m_check[index.y][index.x] = true;


    int count = 1;


    Index aroundIndex[] =
    {
        { index.x - 1, index.y },
        { index.x + 1, index.y },
        { index.x, index.y - 1 },
        { index.x, index.y + 1 }
    };


    for (int i = 0;
        i < _countof(aroundIndex);
        ++i)
    {
        Index around =
            aroundIndex[i];


        if (around.x < 0 ||
            around.x >= FIELD_COLUMN ||
            around.y < 0 ||
            around.y >= FIELD_ROW)
        {
            continue;
        }


        if (m_check[around.y][around.x])
            continue;


        Block* pAround =
            m_grid[around.y][around.x];

        if (pAround == nullptr)
            continue;


        if (pCenter->GetColor()
            != pAround->GetColor())
        {
            continue;
        }


        count +=
            RecursiveBlockCount(around);
    }


    return count;
}


//============================================================
// 同色ブロックをDESTROYへ
//============================================================

void Field::RecursiveBlockDestroy(Index index)
{
    if (index.x < 0 ||
        index.x >= FIELD_COLUMN ||
        index.y < 0 ||
        index.y >= FIELD_ROW)
    {
        return;
    }


    Block* pCenter =
        m_grid[index.y][index.x];

    if (pCenter == nullptr)
        return;


    if (pCenter->GetState()
        == Block::State::DESTROY)
    {
        return;
    }


    pCenter->SetState(
        Block::State::DESTROY);


    Index aroundIndex[] =
    {
        { index.x - 1, index.y },
        { index.x + 1, index.y },
        { index.x, index.y - 1 },
        { index.x, index.y + 1 }
    };


    for (int i = 0;
        i < _countof(aroundIndex);
        ++i)
    {
        Index around =
            aroundIndex[i];


        if (around.x < 0 ||
            around.x >= FIELD_COLUMN ||
            around.y < 0 ||
            around.y >= FIELD_ROW)
        {
            continue;
        }


        Block* pAround =
            m_grid[around.y][around.x];

        if (pAround == nullptr)
            continue;


        if (pAround->GetState()
            == Block::State::DESTROY)
        {
            continue;
        }


        if (pAround->GetColor()
            != pCenter->GetColor())
        {
            continue;
        }


        RecursiveBlockDestroy(around);
    }
}


//============================================================
// BlockとGridの同期
//============================================================

void Field::syncBlock(int x, int y)
{
    if (x < 0 ||
        x >= FIELD_COLUMN ||
        y < 0 ||
        y >= FIELD_ROW)
    {
        return;
    }


    Block* pBlock =
        m_grid[y][x];

    if (pBlock == nullptr)
        return;


    //============================================================
    // MOVE以外は同期しない
    //============================================================

    if (pBlock->GetState()
        != Block::State::MOVE)
    {
        return;
    }


    float2 pos =
        pBlock->GetPos();


    //============================================================
    // 現在の描画座標からグリッド位置を取得
    //============================================================

    Index index =
        PosToIndex(pos);


    //============================================================
    // 現在位置と同じなら何もしない
    //============================================================

    if (index.x == x &&
        index.y == y)
    {
        return;
    }


    //============================================================
    // 移動先が別ブロックで埋まっている場合
    //============================================================

    if (m_grid[index.y][index.x] != nullptr &&
        m_grid[index.y][index.x] != pBlock)
    {
        // 現在位置へ戻す
        pBlock->SetPos(
            IndexToPos({ x, y }));

        // 操作終了
        pBlock->SetState(
            Block::State::IDLE);

        pBlock->ResetFallSpeed();

        return;
    }


    //============================================================
    // 元の位置を空にする
    //============================================================

    m_grid[y][x] = nullptr;


    //============================================================
    // 新しい位置へ
    //============================================================

    m_grid[index.y][index.x] =
        pBlock;


    //============================================================
    // 座標をグリッドに合わせる
    //============================================================

    pBlock->SetPos(
        IndexToPos(index));
}


//============================================================
// 回転
//============================================================

void Field::RotateBlock(int direction)
{
    //============================================================
    // 生成中(まだm_grid未登録)は専用処理へ
    //============================================================

    if (m_isSpawning)
    {
        RotateSpawnBlock(direction);
        return;
    }

    //============================================================
    // ここから下は既存のm_grid検索ベースの処理(変更なし)
    //============================================================

    if (m_pivotBlock == nullptr)
        return;

    //============================================================
    // 回転中心が存在しない場合
    //============================================================

    if (m_pivotBlock == nullptr)
        return;


    //============================================================
    // 回転するもう一方のブロックを探す
    //============================================================

    Block* rotateBlock = nullptr;

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock = m_grid[y][x];

            if (pBlock == nullptr)
                continue;

            if (pBlock == m_pivotBlock)
                continue;

            if (pBlock->GetState() != Block::State::MOVE)
                continue;

            rotateBlock = pBlock;
            break;
        }

        if (rotateBlock != nullptr)
            break;
    }

    if (rotateBlock == nullptr)
        return;


    //============================================================
    // pivot の現在位置を m_grid から探す
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
    // rotateBlock の現在位置を探す
    //============================================================

    int rotateX = -1;
    int rotateY = -1;

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            if (m_grid[y][x] == rotateBlock)
            {
                rotateX = x;
                rotateY = y;
                break;
            }
        }

        if (rotateX != -1)
            break;
    }

    if (rotateX == -1 || rotateY == -1)
        return;


    //============================================================
    // 現在の配置が縦か確認
    //
    //     B
    //     A
    //
    // のような状態なら true
    //============================================================

    bool isVertical =
        (pivotX == rotateX);


    //============================================================
    // 前回の回転が失敗していて、
    // 同じ方向をもう一度押した場合
    //
    // 上下を入れ替える
    //============================================================

    if (m_rotateFailed &&
        m_rotateFailedDirection == direction &&
        isVertical)
    {
        // 現在の位置を保存
        int pivotOldX = pivotX;
        int pivotOldY = pivotY;

        int rotateOldX = rotateX;
        int rotateOldY = rotateY;


        //========================================================
        // 元の位置を一旦クリア
        //========================================================

        m_grid[pivotOldY][pivotOldX] = nullptr;
        m_grid[rotateOldY][rotateOldX] = nullptr;


        //========================================================
        // 上下を入れ替える
        //========================================================

        m_grid[rotateOldY][rotateOldX] =
            m_pivotBlock;

        m_grid[pivotOldY][pivotOldX] =
            rotateBlock;


        //========================================================
        // 表示位置も入れ替える
        //========================================================

        Index pivotNewIndex;
        pivotNewIndex.x = rotateOldX;
        pivotNewIndex.y = rotateOldY;

        Index rotateNewIndex;
        rotateNewIndex.x = pivotOldX;
        rotateNewIndex.y = pivotOldY;


        m_pivotBlock->SetPos(
            IndexToPos(pivotNewIndex));

        rotateBlock->SetPos(
            IndexToPos(rotateNewIndex));


        //========================================================
        // 回転失敗状態を解除
        //========================================================

        m_rotateFailed = false;
        m_rotateFailedDirection = 0;

        return;
    }


    //============================================================
    // 回転中心から見た相対座標
    //============================================================

    int dx =
        rotateX - pivotX;

    int dy =
        rotateY - pivotY;


    //============================================================
    // 90度回転
    //============================================================

    int newDx;
    int newDy;

    if (direction > 0)
    {
        // 時計回り
        newDx = -dy;
        newDy = dx;
    }
    else
    {
        // 反時計回り
        newDx = dy;
        newDy = -dx;
    }


    //============================================================
    // 回転後の位置
    //============================================================

    Index newPivot;
    newPivot.x = pivotX;
    newPivot.y = pivotY;

    Index newRotate;
    newRotate.x = pivotX + newDx;
    newRotate.y = pivotY + newDy;


    //============================================================
    // 壁蹴り
    //============================================================

    int offsetX = 0;

    if (newRotate.x < 0)
    {
        offsetX = 1;
    }
    else if (newRotate.x >= FIELD_COLUMN)
    {
        offsetX = -1;
    }


    newPivot.x += offsetX;
    newRotate.x += offsetX;


    //============================================================
    // フィールド外チェック
    //============================================================

    if (newPivot.x < 0 ||
        newPivot.x >= FIELD_COLUMN ||
        newPivot.y < 0 ||
        newPivot.y >= FIELD_ROW)
    {
        m_rotateFailed = false;
        m_rotateFailedDirection = 0;

        return;
    }

    if (newRotate.x < 0 ||
        newRotate.x >= FIELD_COLUMN ||
        newRotate.y < 0 ||
        newRotate.y >= FIELD_ROW)
    {
        m_rotateFailed = false;
        m_rotateFailedDirection = 0;

        return;
    }


    //============================================================
    // 回転先のブロックを確認
    //============================================================

    Block* pivotTarget =
        m_grid[newPivot.y][newPivot.x];

    Block* rotateTarget =
        m_grid[newRotate.y][newRotate.x];


    bool pivotBlocked =
        pivotTarget != nullptr &&
        pivotTarget != m_pivotBlock &&
        pivotTarget != rotateBlock;

    bool rotateBlocked =
        rotateTarget != nullptr &&
        rotateTarget != m_pivotBlock &&
        rotateTarget != rotateBlock;


    //============================================================
    // 回転できない場合
    //============================================================

    if (pivotBlocked || rotateBlocked)
    {
        //========================================================
        // 縦配置の場合のみ、
        // 「次の同方向入力で上下入れ替え」を有効にする
        //========================================================

        if (isVertical)
        {
            //====================================================
            // pivot の左右が塞がっているか確認
            //====================================================

            bool leftBlocked = false;
            bool rightBlocked = false;


            // 左
            if (pivotX - 1 < 0)
            {
                leftBlocked = true;
            }
            else
            {
                Block* leftBlock =
                    m_grid[pivotY][pivotX - 1];

                if (leftBlock != nullptr &&
                    leftBlock != m_pivotBlock &&
                    leftBlock != rotateBlock)
                {
                    leftBlocked = true;
                }
            }


            // 右
            if (pivotX + 1 >= FIELD_COLUMN)
            {
                rightBlocked = true;
            }
            else
            {
                Block* rightBlock =
                    m_grid[pivotY][pivotX + 1];

                if (rightBlock != nullptr &&
                    rightBlock != m_pivotBlock &&
                    rightBlock != rotateBlock)
                {
                    rightBlocked = true;
                }
            }


            //====================================================
            // 左右両方が塞がっている
            //====================================================

            if (leftBlocked && rightBlocked)
            {
                m_rotateFailed = true;
                m_rotateFailedDirection = direction;
            }
            else
            {
                m_rotateFailed = false;
                m_rotateFailedDirection = 0;
            }
        }
        else
        {
            m_rotateFailed = false;
            m_rotateFailedDirection = 0;
        }

        return;
    }


    //============================================================
    // 回転成功
    //============================================================

    m_rotateFailed = false;
    m_rotateFailedDirection = 0;


    //============================================================
    // 元の位置をクリア
    //============================================================

    m_grid[pivotY][pivotX] = nullptr;
    m_grid[rotateY][rotateX] = nullptr;


    //============================================================
    // 新しい位置へ移動
    //============================================================

    m_pivotBlock->SetPos(
        IndexToPos(newPivot));

    rotateBlock->SetPos(
        IndexToPos(newRotate));


    //============================================================
    // grid に再登録
    //============================================================

    m_grid[newPivot.y][newPivot.x] =
        m_pivotBlock;

    m_grid[newRotate.y][newRotate.x] =
        rotateBlock;


    //============================================================
    // MOVE状態を維持
    //============================================================

    m_pivotBlock->SetState(
        Block::State::MOVE);

    rotateBlock->SetState(
        Block::State::MOVE);
}

//============================================================
// FALL処理
//============================================================

void Field::UpdateFallBlock(int x, int y)
{
    if (x < 0 ||
        x >= FIELD_COLUMN ||
        y < 0 ||
        y >= FIELD_ROW)
    {
        return;
    }


    Block* pBlock =
        m_grid[y][x];

    if (pBlock == nullptr)
        return;


    if (pBlock->GetState()
        != Block::State::FALL)
    {
        return;
    }


    float2 pos =
        pBlock->GetPos();


    //============================================================
    // 現在位置
    //============================================================

    Index current;

    current.x = x;
    current.y = y;


    float currentY =
        IndexToPos(current).y;


    //============================================================
    // 最下段
    //============================================================

    if (y + 1 >= FIELD_ROW)
    {
        pBlock->SetPos(
            IndexToPos(current));

        pBlock->SetState(
            Block::State::IDLE);

        pBlock->ResetFallSpeed();

        return;
    }


    //============================================================
    // 下のブロック
    //============================================================

    if (m_grid[y + 1][x] != nullptr)
    {
        pBlock->SetPos(
            IndexToPos(current));

        pBlock->SetState(
            Block::State::IDLE);

        pBlock->ResetFallSpeed();

        return;
    }


    //============================================================
    // 下のマス
    //============================================================

    Index below;

    below.x = x;
    below.y = y + 1;


    float belowY =
        IndexToPos(below).y;


    //============================================================
    // 1マス分落下した
    //============================================================

    if (pos.y >= belowY)
    {
        m_grid[y][x] = nullptr;

        m_grid[y + 1][x] =
            pBlock;

        pBlock->SetPos(
            IndexToPos(below));
    }
}

bool Field::IsCellOccupied(int x, int y, Block* ignoreBlock1, Block* ignoreBlock2)
{
    if (x < 0 || x >= FIELD_COLUMN || y < 0 || y >= FIELD_ROW)
        return true;   // フィールド外は「使用不可」扱い


    //============================================================
    // 着地済みブロック(m_grid)をチェック
    //============================================================

    Block* pGridBlock = m_grid[y][x];

    if (pGridBlock != nullptr &&
        pGridBlock != ignoreBlock1 &&
        pGridBlock != ignoreBlock2)
    {
        return true;
    }


    //============================================================
    // 生成中(まだm_gridに登録されていない)ブロックもチェック
    //============================================================

    for (int i = 0; i < 2; ++i)
    {
        Block* pSpawn = m_spawnBlock[i];

        if (pSpawn == nullptr ||
            pSpawn == ignoreBlock1 ||
            pSpawn == ignoreBlock2)
        {
            continue;
        }

        Index index = PosToIndex(pSpawn->GetPos());

        if (index.x == x && index.y == y)
        {
            return true;
        }
    }

    return false;
}

void Field::RotateSpawnBlock(int direction)
{
    if (m_spawnBlock[0] == nullptr || m_spawnBlock[1] == nullptr)
        return;

    if (m_pivotBlock == nullptr)
        return;


    //============================================================
    // pivotともう一方を判別
    //============================================================

    Block* pivot = m_pivotBlock;

    Block* other =
        (m_spawnBlock[0] == pivot) ?
        m_spawnBlock[1] : m_spawnBlock[0];


    //============================================================
    // 現在位置(画面外は負の値になりうるのでクランプしない)
    //============================================================

    Index pivotIndex = PosToIndexRaw(pivot->GetPos());
    Index otherIndex = PosToIndexRaw(other->GetPos());


    //============================================================
    // 相対座標
    //============================================================

    int dx = otherIndex.x - pivotIndex.x;
    int dy = otherIndex.y - pivotIndex.y;

    bool isVertical = (dx == 0);


    //============================================================
    // 90度回転
    //============================================================

    int newDx;
    int newDy;

    if (direction > 0)
    {
        newDx = -dy;
        newDy = dx;
    }
    else
    {
        newDx = dy;
        newDy = -dx;
    }

    Index newPivot = pivotIndex;

    Index newOther;
    newOther.x = pivotIndex.x + newDx;
    newOther.y = pivotIndex.y + newDy;


    //============================================================
    // 壁蹴り(左右のみ。上下はフィールド外でも許可する)
    //============================================================

    int offsetX = 0;

    if (newOther.x < 0)
        offsetX = 1;
    else if (newOther.x >= FIELD_COLUMN)
        offsetX = -1;

    newPivot.x += offsetX;
    newOther.x += offsetX;


    //============================================================
    // 左右のフィールド外チェック(縦方向はスルーする)
    //============================================================

    if (newPivot.x < 0 || newPivot.x >= FIELD_COLUMN ||
        newOther.x < 0 || newOther.x >= FIELD_COLUMN)
    {
        return;
    }


    //============================================================
    // 衝突チェック
    //
    // 行がフィールド内(0〜FIELD_ROW-1)の場合のみm_gridを確認する。
    // まだ画面外(行が負)の場合は何も置かれていないので常に通す。
    //============================================================

    auto isBlocked = [&](Index idx) -> bool
        {
            if (idx.y < 0 || idx.y >= FIELD_ROW)
                return false;

            Block* pTarget = m_grid[idx.y][idx.x];

            return pTarget != nullptr &&
                pTarget != m_spawnBlock[0] &&
                pTarget != m_spawnBlock[1];
        };

    if (isBlocked(newPivot) || isBlocked(newOther))
        return;


    //============================================================
    // 回転成功、位置を反映(m_gridへの登録は無いのでそのまま座標だけ更新)
    //============================================================

    pivot->SetPos(IndexToPos(newPivot));
    other->SetPos(IndexToPos(newOther));
}

Field::Index Field::PosToIndexRaw(float2 pos)
{
    Index index;

    pos.x -= m_offset.x;
    pos.y -= m_offset.y;

    pos.x += BLOCK_WIDTH * 0.5f;
    pos.y += BLOCK_HEIGHT * 0.5f;

    index.x = (int)std::floor(pos.x / BLOCK_WIDTH);
    index.y = (int)std::floor(pos.y / BLOCK_HEIGHT);

    return index;
}


//============================================================
// Field状態取得
//============================================================

Field::State Field::GetState() const
{
    return m_state;
}

