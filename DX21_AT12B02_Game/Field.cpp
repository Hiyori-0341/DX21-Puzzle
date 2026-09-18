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
    m_offset.x =
        0.5f * (FIELD_COLUMN - 1.0f) * BLOCK_WIDTH;

    m_offset.y =
        0.5f * (FIELD_ROW - 1.0f) * BLOCK_HEIGHT;

    m_offset.x = -m_offset.x;
    m_offset.y = -m_offset.y;

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

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            m_grid[y][x] = nullptr;
        }
    }

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

        for (int i = 0; i < updateCount; ++i)
        {
            if (updateList[i] != nullptr)
            {
                updateList[i]->Update();
            }
        }
    }

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

    switch (m_state)
    {
    case Field::State::CREATE:
        UpdateCreate();
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

    // 生成中のブロックも描画する。
    // 上側のブロックはフィールド1マス上にいるため、
    // m_gridには入らずm_spawnBlockから直接描画する。
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

    //============================================================
    // 生成中のブロック
    //
    // 上側が場外(row -1)にいても、そのままハードドロップできる。
    // 2個をペアとして扱い、下方向へ最大まで落とす。
    //============================================================
    if (m_isSpawning &&
        m_spawnBlock[0] != nullptr &&
        m_spawnBlock[1] != nullptr)
    {
        Block* moveBlock[2] =
        {
            m_spawnBlock[0],
            m_spawnBlock[1]
        };

        Index index[2];

        for (int i = 0; i < 2; ++i)
            index[i] = PosToIndexRaw(moveBlock[i]->GetPos());

        int dropDist = FIELD_ROW + 1;

        for (int i = 0; i < 2; ++i)
        {
            int dist = 0;

            while (index[i].y + dist + 1 < FIELD_ROW)
            {
                int checkY = index[i].y + dist + 1;

                // ペア自身は障害物として扱わない。
                if (checkY == index[0].y &&
                    index[i].x == index[0].x)
                {
                    ++dist;
                    continue;
                }

                if (checkY == index[1].y &&
                    index[i].x == index[1].x)
                {
                    ++dist;
                    continue;
                }

                Block* pBlock =
                    m_grid[checkY][index[i].x];

                if (pBlock != nullptr &&
                    pBlock != moveBlock[0] &&
                    pBlock != moveBlock[1])
                {
                    break;
                }

                ++dist;
            }

            if (dist < dropDist)
                dropDist = dist;
        }

        // 生成中の下側ブロックがm_gridに入っていれば外す。
        for (int i = 0; i < 2; ++i)
        {
            Index oldIndex =
                PosToIndexRaw(moveBlock[i]->GetPos());

            if (oldIndex.y >= 0 &&
                oldIndex.y < FIELD_ROW &&
                oldIndex.x >= 0 &&
                oldIndex.x < FIELD_COLUMN &&
                m_grid[oldIndex.y][oldIndex.x] == moveBlock[i])
            {
                m_grid[oldIndex.y][oldIndex.x] = nullptr;
            }
        }

        for (int i = 0; i < 2; ++i)
        {
            Index newIndex = index[i];
            newIndex.y += dropDist;

            //if (newIndex.y < 0)
            //{
            //    // 通常は起こらないが、場外に残さない。
            //    return;
            //}

            moveBlock[i]->SetPos(
                IndexToPos(newIndex));

            moveBlock[i]->ResetFallSpeed();
            moveBlock[i]->SetState(
                Block::State::IDLE);

            if (newIndex.y >= 0 && newIndex.y < FIELD_ROW &&
                newIndex.x >= 0 && newIndex.x < FIELD_COLUMN)
            {
                m_grid[newIndex.y][newIndex.x] = moveBlock[i];
            }
        }

        m_spawnBlock[0] = nullptr;
        m_spawnBlock[1] = nullptr;
        m_isSpawning = false;
        m_fallTimer = 0;
        m_pivotBlock = nullptr;

        return;
    }

    //============================================================
    // 通常時のハードドロップ
    //============================================================

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

            moveBlock[blockCount] = m_grid[y][x];
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

    int dropDist = FIELD_ROW;

    for (int i = 0; i < 2; ++i)
    {
        int x = blockX[i];
        int y = blockY[i];
        int dist = 0;

        while (y + dist + 1 < FIELD_ROW)
        {
            int checkY = y + dist + 1;

            Block* pBlock = m_grid[checkY][x];

            if (pBlock == moveBlock[0] ||
                pBlock == moveBlock[1])
            {
                ++dist;
                continue;
            }

            if (pBlock != nullptr)
                break;

            ++dist;
        }

        if (dist < dropDist)
            dropDist = dist;
    }

    m_grid[blockY[0]][blockX[0]] = nullptr;
    m_grid[blockY[1]][blockX[1]] = nullptr;

    for (int i = 0; i < 2; ++i)
    {
        int newX = blockX[i];
        int newY = blockY[i] + dropDist;

        m_grid[newY][newX] = moveBlock[i];

        Index index;
        index.x = newX;
        index.y = newY;

        moveBlock[i]->SetPos(IndexToPos(index));
        moveBlock[i]->ResetFallSpeed();
        moveBlock[i]->SetState(Block::State::IDLE);
    }

    m_pivotBlock = nullptr;

    for (int y = FIELD_ROW - 2; y >= 0; --y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock = m_grid[y][x];

            if (pBlock == nullptr)
                continue;

            if (pBlock->GetState() == Block::State::MOVE)
                continue;

            if (m_grid[y + 1][x] == nullptr)
            {
                pBlock->SetState(Block::State::FALL);
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

    //============================================================
    // 生成中のブロック
    //
    // 上側がrow -1、下側がrow 0の状態からでも、
    // 下キー1回でペアを1マス下へ移動させる。
    // 上側がrow 0へ入った時点で通常のm_grid管理へ移行する。
    //============================================================
    if (m_isSpawning &&
        m_spawnBlock[0] != nullptr &&
        m_spawnBlock[1] != nullptr)
    {
        Block* moveBlock[2] =
        {
            m_spawnBlock[0],
            m_spawnBlock[1]
        };

        Index current[2];

        for (int i = 0; i < 2; ++i)
            current[i] =
            PosToIndexRaw(moveBlock[i]->GetPos());

        Index next[2] = { current[0], current[1] };

        next[0].y++;
        next[1].y++;

        bool canDrop = true;

        for (int i = 0; i < 2; ++i)
        {
            if (next[i].y >= FIELD_ROW)
            {
                canDrop = false;
                break;
            }

            Block* pBlock =
                m_grid[next[i].y][next[i].x];

            if (pBlock != nullptr &&
                pBlock != moveBlock[0] &&
                pBlock != moveBlock[1])
            {
                canDrop = false;
                break;
            }
        }

        if (!canDrop)
        {
            // 場外のまま止まる場合はゲームオーバー。
            if (next[0].y < 0)
            {
                m_state = Field::State::GAMEOVER;
                return;
            }

            // ペアをその場で固定する。
            for (int i = 0; i < 2; ++i)
            {
                Index oldIndex = current[i];

                if (oldIndex.y >= 0 &&
                    oldIndex.y < FIELD_ROW &&
                    oldIndex.x >= 0 &&
                    oldIndex.x < FIELD_COLUMN &&
                    m_grid[oldIndex.y][oldIndex.x] == moveBlock[i])
                {
                    m_grid[oldIndex.y][oldIndex.x] = nullptr;
                }
            }

            for (int i = 0; i < 2; ++i)
            {
                moveBlock[i]->SetState(Block::State::IDLE);
                moveBlock[i]->ResetFallSpeed();

                if (current[i].y >= 0 &&
                    current[i].y < FIELD_ROW)
                {
                    m_grid[current[i].y][current[i].x] =
                        moveBlock[i];
                    moveBlock[i]->SetPos(
                        IndexToPos(current[i]));
                }
            }

            m_spawnBlock[0] = nullptr;
            m_spawnBlock[1] = nullptr;
            m_isSpawning = false;
            m_fallTimer = 0;
            m_pivotBlock = nullptr;

            return;
        }

        // 現在位置をm_gridから一旦外す。
        for (int i = 0; i < 2; ++i)
        {
            Index oldIndex = current[i];

            if (oldIndex.y >= 0 &&
                oldIndex.y < FIELD_ROW &&
                oldIndex.x >= 0 &&
                oldIndex.x < FIELD_COLUMN &&
                m_grid[oldIndex.y][oldIndex.x] == moveBlock[i])
            {
                m_grid[oldIndex.y][oldIndex.x] = nullptr;
            }
        }

        for (int i = 0; i < 2; ++i)
        {
            moveBlock[i]->SetPos(IndexToPos(next[i]));
            moveBlock[i]->ResetFallSpeed();

            m_grid[next[i].y][next[i].x] =
                moveBlock[i];
        }

        // 上側がフィールドへ入ったら生成状態を終了。
        if (next[0].y >= 0)
        {
            m_spawnBlock[0] = nullptr;
            m_spawnBlock[1] = nullptr;
            m_isSpawning = false;
            m_fallTimer = 0;
        }

        return;
    }

    //============================================================
    // 通常時のソフトドロップ
    //============================================================

    Block* moveBlock[2] =
    {
        nullptr,
        nullptr
    };

    int blockX[2] = { -1, -1 };
    int blockY[2] = { -1, -1 };
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

            moveBlock[blockCount] = m_grid[y][x];
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

        Block* below = m_grid[y + 1][x];

        if (below != nullptr &&
            below != moveBlock[0] &&
            below != moveBlock[1])
        {
            canDrop = false;
            break;
        }
    }

    if (!canDrop)
    {
        moveBlock[0]->SetState(Block::State::IDLE);
        moveBlock[1]->SetState(Block::State::IDLE);
        moveBlock[0]->ResetFallSpeed();
        moveBlock[1]->ResetFallSpeed();
        m_pivotBlock = nullptr;
        return;
    }

    m_grid[blockY[0]][blockX[0]] = nullptr;
    m_grid[blockY[1]][blockX[1]] = nullptr;

    for (int i = 0; i < 2; ++i)
    {
        int newX = blockX[i];
        int newY = blockY[i] + 1;

        m_grid[newY][newX] = moveBlock[i];

        Index index;
        index.x = newX;
        index.y = newY;

        moveBlock[i]->SetPos(IndexToPos(index));
        moveBlock[i]->ResetFallSpeed();
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
    // 生成中のブロック
    //
    // 上側が場外でも、通常時と同じ左右操作を可能にする。
    //============================================================

    if (m_isSpawning &&
        m_spawnBlock[0] != nullptr &&
        m_spawnBlock[1] != nullptr)
    {
        float2 pos0 =
            m_spawnBlock[0]->GetPos();

        float2 pos1 =
            m_spawnBlock[1]->GetPos();

        int x0 =
            PosToIndexRaw(pos0).x;

        int x1 =
            PosToIndexRaw(pos1).x;

        int targetX0 =
            x0 + direction;

        int targetX1 =
            x1 + direction;

        if (targetX0 < 0 ||
            targetX0 >= FIELD_COLUMN ||
            targetX1 < 0 ||
            targetX1 >= FIELD_COLUMN)
        {
            return;
        }

        // フィールド内にある下側だけ衝突確認する。
        int y1 =
            PosToIndexRaw(pos1).y;

        if (y1 >= 0 && y1 < FIELD_ROW)
        {
            Block* pTarget =
                m_grid[y1][targetX1];

            if (pTarget != nullptr &&
                pTarget != m_spawnBlock[0] &&
                pTarget != m_spawnBlock[1])
            {
                return;
            }
        }

        pos0.x += BLOCK_WIDTH * direction;
        pos1.x += BLOCK_WIDTH * direction;

        m_spawnBlock[0]->SetPos(pos0);
        m_spawnBlock[1]->SetPos(pos1);

        m_isMoveRight =
            direction > 0;

        return;
    }

    //============================================================
    // 通常のMOVEブロック
    //============================================================

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

    for (int i = 0; i < 2; ++i)
    {
        m_grid[blockY[i]][blockX[i]] =
            nullptr;
    }

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
//
// 生成処理と通常の操作・落下処理を分離しない。
// フィールド最上段に直接配置し、直後からIDLEで操作する。
//============================================================

void Field::UpdateCreate()
{
    const int spawnX =
        FIELD_COLUMN / 2;

    //============================================================
    // 生成位置(row0)にブロックがある場合はゲームオーバー
    //============================================================

    if (m_grid[0][spawnX] != nullptr)
    {
        m_state =
            Field::State::GAMEOVER;

        return;
    }

    //============================================================
    // ブロック生成
    //
    // 現在の生成位置(row0/row1)から1マス上へずらし、
    // 上側を場外(row-1)、下側をフィールド最上段(row0)
    // から開始する。
    //============================================================

    m_spawnBlock[0] =
        new Block(
            rand() % BLOCK_COLOR_NUM,
            this);

    m_spawnBlock[1] =
        new Block(
            rand() % BLOCK_COLOR_NUM,
            this);

    m_pivotBlock =
        m_spawnBlock[1];

    //============================================================
    // 生成位置
    //
    // block0 : row -1（場外）
    // block1 : row  0（フィールド最上段）
    //============================================================

    float2 pos0;

    pos0.x =
        spawnX * BLOCK_WIDTH + m_offset.x;

    pos0.y =
        m_offset.y - BLOCK_HEIGHT;

    float2 pos1;

    pos1.x =
        spawnX * BLOCK_WIDTH + m_offset.x;

    pos1.y =
        m_offset.y;

    m_spawnBlock[0]->SetPos(pos0);
    m_spawnBlock[1]->SetPos(pos1);

    m_spawnBlock[0]->SetState(
        Block::State::MOVE);

    m_spawnBlock[1]->SetState(
        Block::State::MOVE);

    //============================================================
    // 下側ブロック(row0)だけ先にm_gridへ登録
    //
    // 上側はまだ場外なのでm_spawnBlockで管理する。
    //============================================================

    m_grid[0][spawnX] =
        m_spawnBlock[1];

    m_isSpawning = true;

    m_fallTimer = 0;

    // 生成後も通常のIDLE処理を使う。
    m_state =
        Field::State::IDLE;
}

//============================================================
// Idle
//============================================================

void Field::UpdateIdle()
{
    //============================================================
    // 生成直後のブロック処理
    //
    // 上側は1マス場外、下側はrow0にいる。
    // 左右移動・回転は通常操作と同じ入力で行い、
    // 一定時間ごとに1マスずつ下へ送る。
    // 上側がrow0へ入った時点で2個とも通常のm_grid管理へ移行する。
    //============================================================

    if (m_isSpawning &&
        m_spawnBlock[0] != nullptr &&
        m_spawnBlock[1] != nullptr)
    {
        // 左右移動
        if (isKeyRepeat(VK_LEFT) || isKeyTrigger(VK_LEFT))
        {
            MoveHorizontal(-1);
        }

        if (isKeyRepeat(VK_RIGHT) || isKeyTrigger(VK_RIGHT))
        {
            MoveHorizontal(1);
        }

        // 回転
        if (isKeyTrigger('X'))
        {
            RotateBlock(1);
        }

        if (isKeyTrigger('Z'))
        {
            RotateBlock(-1);
        }

        ++m_fallTimer;

        if (m_fallTimer >= BLOCK_MOVE_WAIT_TIME)
        {
            m_fallTimer = 0;

            float2 pos0 =
                m_spawnBlock[0]->GetPos();

            float2 pos1 =
                m_spawnBlock[1]->GetPos();

            Index index0 =
                PosToIndexRaw(pos0);

            Index index1 =
                PosToIndexRaw(pos1);

            Index next0 = index0;
            Index next1 = index1;

            next0.y++;
            next1.y++;

            // 下側が次のマスへ移動できるか確認。
            // 自分たちのブロックは衝突扱いしない。
            bool canDrop = true;

            if (next1.y >= FIELD_ROW)
            {
                canDrop = false;
            }
            else
            {
                Block* pBelow =
                    m_grid[next1.y][next1.x];

                if (pBelow != nullptr &&
                    pBelow != m_spawnBlock[0] &&
                    pBelow != m_spawnBlock[1])
                {
                    canDrop = false;
                }
            }

            if (!canDrop)
            {
                // 場外の上側がまだ入っている状態で
                // 下側が進めない場合はゲームオーバー。
                if (next0.y < 0)
                {
                    m_state =
                        Field::State::GAMEOVER;

                    return;
                }

                m_spawnBlock[0]->SetState(
                    Block::State::IDLE);

                m_spawnBlock[1]->SetState(
                    Block::State::IDLE);

                m_spawnBlock[0]->ResetFallSpeed();
                m_spawnBlock[1]->ResetFallSpeed();

                m_pivotBlock = nullptr;

                m_isSpawning = false;
                m_spawnBlock[0] = nullptr;
                m_spawnBlock[1] = nullptr;

                return;
            }

            // 現在の下側(row0)を一旦m_gridから外す。
            Index current1 = PosToIndexRaw(pos1);

            if (current1.y >= 0 &&
                current1.y < FIELD_ROW &&
                current1.x >= 0 &&
                current1.x < FIELD_COLUMN &&
                m_grid[current1.y][current1.x] ==
                m_spawnBlock[1])
            {
                m_grid[current1.y][current1.x] =
                    nullptr;
            }

            // 2個とも1マス下へ移動。
            m_spawnBlock[0]->SetPos(
                IndexToPos(next0));

            m_spawnBlock[1]->SetPos(
                IndexToPos(next1));

            // 上側がrow0に入ったら2個とも通常管理へ。
            if (next0.y >= 0)
            {
                m_grid[next0.y][next0.x] =
                    m_spawnBlock[0];

                m_grid[next1.y][next1.x] =
                    m_spawnBlock[1];

                m_spawnBlock[0]->SetPos(
                    IndexToPos(next0));

                m_spawnBlock[1]->SetPos(
                    IndexToPos(next1));

                m_spawnBlock[0] = nullptr;
                m_spawnBlock[1] = nullptr;

                m_isSpawning = false;
                m_fallTimer = 0;

                // MOVE状態のまま、以降は通常のUpdateIdle()で処理する。
            }
        }

        // 生成中は通常のMOVEブロック検索へ進ませない。
        return;
    }

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
    // → 生成直後も通常時も同じ処理
    //============================================================

    if (moveCount >= 2)
    {
        ++m_fallTimer;

        if (m_fallTimer >= BLOCK_MOVE_WAIT_TIME)
        {
            m_fallTimer = 0;

            bool canDrop = true;

            for (int i = 0; i < 2; ++i)
            {
                float2 pos =
                    moveBlock[i]->GetPos();

                Index index =
                    PosToIndex(pos);

                if (index.y + 1 >= FIELD_ROW)
                {
                    canDrop = false;
                    break;
                }

                Block* below =
                    m_grid[index.y + 1][index.x];

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
                for (int i = 0; i < 2; ++i)
                {
                    Index index =
                        PosToIndex(
                            moveBlock[i]->GetPos());

                    m_grid[index.y][index.x] =
                        nullptr;
                }

                for (int i = 0; i < 2; ++i)
                {
                    Index index =
                        PosToIndex(
                            moveBlock[i]->GetPos());

                    index.y++;

                    m_grid[index.y][index.x] =
                        moveBlock[i];

                    moveBlock[i]->SetPos(
                        IndexToPos(index));

                    moveBlock[i]->ResetFallSpeed();
                }
            }
            else
            {
                moveBlock[0]->SetState(
                    Block::State::IDLE);

                moveBlock[1]->SetState(
                    Block::State::IDLE);

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
    // IDLEなのに下が空いているブロックをFALLへ
    //============================================================

    bool hasFall = false;

    for (int y = FIELD_ROW - 2; y >= 0; --y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            Block* pBlock = m_grid[y][x];

            if (pBlock == nullptr)
                continue;

            if (pBlock->GetState() == Block::State::MOVE)
                continue;

            if (pBlock->GetState() == Block::State::DESTROY)
                continue;

            if (pBlock->GetState() == Block::State::FALL)
            {
                hasFall = true;
                continue;
            }

            if (m_grid[y + 1][x] == nullptr)
            {
                pBlock->SetState(
                    Block::State::FALL);

                pBlock->ResetFallSpeed();

                hasFall = true;
            }
        }
    }

    if (hasFall)
    {
        return;
    }

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

    for (int y = 0; y < FIELD_ROW; ++y)
    {
        for (int x = 0; x < FIELD_COLUMN; ++x)
        {
            m_check[y][x] = false;
        }
    }

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

    if (!isDestroy)
    {
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

    if (pBlock->GetState()
        != Block::State::MOVE)
    {
        return;
    }

    float2 pos =
        pBlock->GetPos();

    Index index =
        PosToIndex(pos);

    if (index.x == x &&
        index.y == y)
    {
        return;
    }

    if (m_grid[index.y][index.x] != nullptr &&
        m_grid[index.y][index.x] != pBlock)
    {
        pBlock->SetPos(
            IndexToPos({ x, y }));

        pBlock->SetState(
            Block::State::IDLE);

        pBlock->ResetFallSpeed();

        return;
    }

    m_grid[y][x] = nullptr;

    m_grid[index.y][index.x] =
        pBlock;

    pBlock->SetPos(
        IndexToPos(index));
}


//============================================================
// 回転
//============================================================

void Field::RotateBlock(int direction)
{
    //============================================================
    // 生成中の回転
    //
    // 上側が場外でも回転できるようにする。
    // PosToIndexRaw()を使用するため、y=-1も扱える。
    //============================================================

    if (m_isSpawning &&
        m_spawnBlock[0] != nullptr &&
        m_spawnBlock[1] != nullptr &&
        m_pivotBlock != nullptr)
    {
        Block* pivot =
            m_pivotBlock;

        Block* other =
            (m_spawnBlock[0] == pivot) ?
            m_spawnBlock[1] :
            m_spawnBlock[0];

        Index pivotIndex =
            PosToIndexRaw(pivot->GetPos());

        Index otherIndex =
            PosToIndexRaw(other->GetPos());

        int dx =
            otherIndex.x - pivotIndex.x;

        int dy =
            otherIndex.y - pivotIndex.y;

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

        Index newPivot =
            pivotIndex;

        Index newOther;

        newOther.x =
            pivotIndex.x + newDx;

        newOther.y =
            pivotIndex.y + newDy;

        // 左右方向だけ壁蹴りする。
        int offsetX = 0;

        if (newOther.x < 0)
            offsetX = 1;
        else if (newOther.x >= FIELD_COLUMN)
            offsetX = -1;

        newPivot.x += offsetX;
        newOther.x += offsetX;

        if (newPivot.x < 0 ||
            newPivot.x >= FIELD_COLUMN ||
            newOther.x < 0 ||
            newOther.x >= FIELD_COLUMN)
        {
            return;
        }

        auto isBlocked =
            [&](Index index) -> bool
            {
                if (index.y < 0 ||
                    index.y >= FIELD_ROW)
                {
                    return false;
                }

                Block* pBlock =
                    m_grid[index.y][index.x];

                return pBlock != nullptr &&
                    pBlock != pivot &&
                    pBlock != other;
            };

        if (isBlocked(newPivot) ||
            isBlocked(newOther))
        {
            return;
        }

        // 回転成功。
        pivot->SetPos(
            IndexToPos(newPivot));

        other->SetPos(
            IndexToPos(newOther));

        return;
    }

    if (m_pivotBlock == nullptr)
        return;

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

    bool isVertical =
        (pivotX == rotateX);

    if (m_rotateFailed &&
        m_rotateFailedDirection == direction &&
        isVertical)
    {
        int pivotOldX = pivotX;
        int pivotOldY = pivotY;

        int rotateOldX = rotateX;
        int rotateOldY = rotateY;

        m_grid[pivotOldY][pivotOldX] = nullptr;
        m_grid[rotateOldY][rotateOldX] = nullptr;

        m_grid[rotateOldY][rotateOldX] =
            m_pivotBlock;

        m_grid[pivotOldY][pivotOldX] =
            rotateBlock;

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

        m_rotateFailed = false;
        m_rotateFailedDirection = 0;

        return;
    }

    int dx =
        rotateX - pivotX;

    int dy =
        rotateY - pivotY;

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

    Index newPivot;
    newPivot.x = pivotX;
    newPivot.y = pivotY;

    Index newRotate;
    newRotate.x = pivotX + newDx;
    newRotate.y = pivotY + newDy;

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

    if (pivotBlocked || rotateBlocked)
    {
        if (isVertical)
        {
            bool leftBlocked = false;
            bool rightBlocked = false;

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

    m_rotateFailed = false;
    m_rotateFailedDirection = 0;

    m_grid[pivotY][pivotX] = nullptr;
    m_grid[rotateY][rotateX] = nullptr;

    m_pivotBlock->SetPos(
        IndexToPos(newPivot));

    rotateBlock->SetPos(
        IndexToPos(newRotate));

    m_grid[newPivot.y][newPivot.x] =
        m_pivotBlock;

    m_grid[newRotate.y][newRotate.x] =
        rotateBlock;

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

    Index current;

    current.x = x;
    current.y = y;

    float currentY =
        IndexToPos(current).y;

    if (y + 1 >= FIELD_ROW)
    {
        pBlock->SetPos(
            IndexToPos(current));

        pBlock->SetState(
            Block::State::IDLE);

        pBlock->ResetFallSpeed();

        return;
    }

    if (m_grid[y + 1][x] != nullptr)
    {
        pBlock->SetPos(
            IndexToPos(current));

        pBlock->SetState(
            Block::State::IDLE);

        pBlock->ResetFallSpeed();

        return;
    }

    Index below;

    below.x = x;
    below.y = y + 1;

    float belowY =
        IndexToPos(below).y;

    if (pos.y >= belowY)
    {
        m_grid[y][x] = nullptr;

        m_grid[y + 1][x] =
            pBlock;

        pBlock->SetPos(
            IndexToPos(below));
    }
}


//============================================================
// 以下はField.hとの互換性を維持するための関数
// 旧SPAWN方式では使用しない。
//============================================================

bool Field::IsCellOccupied(int x, int y, Block* ignoreBlock1, Block* ignoreBlock2)
{
    if (x < 0 || x >= FIELD_COLUMN ||
        y < 0 || y >= FIELD_ROW)
    {
        return true;
    }

    Block* pGridBlock =
        m_grid[y][x];

    if (pGridBlock != nullptr &&
        pGridBlock != ignoreBlock1 &&
        pGridBlock != ignoreBlock2)
    {
        return true;
    }

    return false;
}


Field::Index Field::PosToIndexRaw(float2 pos)
{
    Index index;

    pos.x -= m_offset.x;
    pos.y -= m_offset.y;

    pos.x += BLOCK_WIDTH * 0.5f;
    pos.y += BLOCK_HEIGHT * 0.5f;

    index.x =
        (int)std::floor(pos.x / BLOCK_WIDTH);

    index.y =
        (int)std::floor(pos.y / BLOCK_HEIGHT);

    return index;
}


//============================================================
// Field状態取得
//============================================================

Field::State Field::GetState() const
{
    return m_state;
}
