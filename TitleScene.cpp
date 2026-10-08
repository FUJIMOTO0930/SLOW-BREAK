#include "TitleScene.h"
#include <math.h>

// ========================================
// 初期化
// ========================================
void TitleScene::Initialize()
{
    backgroundHandle = LoadGraph("./resources/title_bg.png");
    soldierFrontHandle = LoadGraph("./resources/title_soldier_front.png");
    soldierBackHandle = LoadGraph("./resources/title_soldier_back.png");
    weaponHandle = LoadGraph("./resources/title_weapon.png");
    smokeHandle = LoadGraph("./resources/title_smoke.png");
    lightHandle = LoadGraph("./resources/title_light.png");

    selectedMenu = 0;
    menuFrameY = 350.0f;
    animationTimer = 0;

    oldUp = false;
    oldDown = false;
    oldEnter = false;
    startRequested = false;

    titleFont = CreateFontToHandle("Arial", 110, 9);
    subtitleFont = CreateFontToHandle("Arial", 22, 3);
    menuFont = CreateFontToHandle("Arial", 28, 4);
}

// ========================================
// 更新
// ========================================
void TitleScene::Update()
{
    animationTimer++;

    bool up = CheckHitKey(KEY_INPUT_UP) != 0;
    bool down = CheckHitKey(KEY_INPUT_DOWN) != 0;
    bool enter = CheckHitKey(KEY_INPUT_RETURN) != 0;

    if (up && !oldUp)
    {
        selectedMenu--;
        if (selectedMenu < 0) selectedMenu = 3;
    }

    if (down && !oldDown)
    {
        selectedMenu++;
        if (selectedMenu > 3) selectedMenu = 0;
    }

    oldUp = up;
    oldDown = down;

    if (enter && !oldEnter && selectedMenu == 0)
    {
        startRequested = true;
    }
    oldEnter = enter;

    // 赤枠を滑らかに移動
    float targetY = 350.0f + selectedMenu * 52.0f;
    menuFrameY += (targetY - menuFrameY) * 0.18f;
}

// ========================================
// 描画
// ========================================
void TitleScene::Draw()
{
    float t = GetNowCount() / 1000.0f;

    // 背景
    if (backgroundHandle >= 0)
    {
        DrawExtendGraph(0, 0, 1280, 720, backgroundHandle, TRUE);
    }

    // ========================================
 // 奥の兵士
 // ========================================
    if (soldierBackHandle >= 0)
    {
        float y = sinf(t * 0.012f) * 2.0f;

        DrawRotaGraphF(
            780.0f,
            480.0f + y,
            0.13,
            0.0,
            soldierBackHandle,
            TRUE
        );
    }

    // ========================================
    // 手前の兵士
    // ========================================
    if (soldierFrontHandle >= 0)
    {
        float y = sinf(t * 0.018f) * 3.0f;

        DrawRotaGraphF(
            955.0f,
            455.0f + y,
            0.23,
            0.0,
            soldierFrontHandle,
            TRUE
        );
    }

    // ========================================
    // 兵士の赤い目を発光させる
    // ========================================
    int eyeAlpha = 120 + (int)(sinf(t * 2.0f) * 80.0f);

    SetDrawBlendMode(DX_BLENDMODE_ADD, eyeAlpha);

    // 左目
    DrawCircle(980, 390, 5, GetColor(255, 0, 0), TRUE);

    // 右目
    DrawCircle(960, 420, 5, GetColor(255, 0, 0), TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 煙：ゆっくり移動
    if (smokeHandle >= 0)
    {
        int offset = (int)(sinf(t * 0.008f) * 35.0f);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 85);
        DrawExtendGraph(
            -30 + offset, 200,
            1310 + offset, 720,
            smokeHandle, TRUE
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 赤い照明：明滅
    if (lightHandle >= 0)
    {
        int alpha = 80 + (int)(sinf(t * 0.055f) * 45.0f);

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawExtendGraph(
            0, 0, 1280, 720,
            lightHandle, TRUE
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ========================================
 // 銃：縦横比を維持した滑らかな呼吸アニメーション
 // ========================================
    if (weaponHandle >= 0)
    {
        // 画像の元サイズを取得
        int imageW = 0;
        int imageH = 0;
        GetGraphSize(weaponHandle, &imageW, &imageH);

        // 縦横比を維持して拡大
        float scale = 0.50f;

        // ゆっくりした呼吸の揺れ
        float x = sinf(t * 0.65f) * 5.0f;
        float y = cosf(t * 0.85f) * 4.0f;

        // 画面下からはみ出すように配置
        float drawX = 1010.0f + x;
        float drawY = 590.0f + y;

        DrawRotaGraphF(
            1010.0f + x,
            590.0f + y,
            0.50,
            0.0,
            weaponHandle,
            TRUE
        );
    }

    // タイトル
    if (titleFont >= 0)
    {
        // グリッチ：ときどき文字全体が横にズレる
        int glitchX = 0;
        if (animationTimer % 120 < 4)
        {
            glitchX = (animationTimer % 2 == 0) ? 7 : -7;
        }

        // 赤い残像
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
        DrawStringToHandle(
            94 + glitchX + 3, 105,
            "SLOW", GetColor(255, 30, 45), titleFont
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 本体
        DrawStringToHandle(
            94 + glitchX, 105,
            "SLOW", GetColor(245, 245, 245), titleFont
        );
    }

    if (subtitleFont >= 0)
    {
        DrawStringToHandle(
            115, 242,
            "B R E A K",
            GetColor(255, 70, 70),
            subtitleFont
        );

        DrawStringToHandle(
            115, 280,
            "T I M E   C O N T R O L   F P S",
            GetColor(190, 200, 210),
            subtitleFont
        );
    }

    // 選択枠
    int frameY = (int)menuFrameY;
    int glowAlpha = 90 + (int)(sinf(t * 0.075f) * 50.0f);

    SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);
    DrawBox(
        85, frameY,
        480, frameY + 44,
        GetColor(180, 0, 0), TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawBox(
        85, frameY,
        480, frameY + 44,
        GetColor(255, 50, 50), FALSE
    );

    DrawBox(
        85, frameY,
        89, frameY + 44,
        GetColor(255, 80, 80), TRUE
    );

    // メニュー文字
    const char* menus[4] =
    {
        "GAME START",
        "HOW TO PLAY",
        "OPTION",
        "EXIT"
    };

    if (menuFont >= 0)
    {
        for (int i = 0; i < 4; i++)
        {
            int color = (i == selectedMenu)
                ? GetColor(255, 255, 255)
                : GetColor(155, 160, 170);

            DrawStringToHandle(
                120, 356 + i * 52,
                menus[i], color, menuFont
            );
        }
    }

    // ========================================
    // FPS表示
    // ========================================
    static int fpsTimer = GetNowCount();
    static int frameCount = 0;
    static int fps = 0;

    frameCount++;

    int now = GetNowCount();
    int elapsed = now - fpsTimer;

    if (elapsed >= 1000)
    {
        fps = frameCount * 1000 / elapsed;
        frameCount = 0;
        fpsTimer = now;
    }

    // 左上に表示
    DrawFormatString(
        10, 10,
        GetColor(0, 255, 0),
        "FPS : %d", fps
    );
}



// ========================================
// 開始判定
// ========================================
bool TitleScene::IsStartSelected()
{
    if (startRequested)
    {
        startRequested = false;
        return true;
    }
    return false;
}

// ========================================
// 終了処理
// ========================================
void TitleScene::Finalize()
{
    int* handles[] =
    {
        &backgroundHandle,
        &soldierFrontHandle,
        &soldierBackHandle,
        &weaponHandle,
        &smokeHandle,
        &lightHandle
    };

    for (int i = 0; i < 6; i++)
    {
        if (*handles[i] >= 0)
        {
            DeleteGraph(*handles[i]);
            *handles[i] = -1;
        }
    }

    if (titleFont >= 0) DeleteFontToHandle(titleFont);
    if (subtitleFont >= 0) DeleteFontToHandle(subtitleFont);
    if (menuFont >= 0) DeleteFontToHandle(menuFont);

    titleFont = -1;
    subtitleFont = -1;
    menuFont = -1;
}