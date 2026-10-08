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
            955.0f, 455.0f + y,
            0.30, 0.0,
            soldierFrontHandle, TRUE
        );
    }

    // ========================================
   // 兵士の目：強い赤色発光
   // ========================================

   // ゆっくり明滅
    float glow = (sinf(t * 2.5f) + 1.0f) * 0.5f;

    // 発光の強さ
    int glowAlpha = 100 + (int)(glow * 155.0f);

    // 目の位置
    int eyeX[2] = { 980, 994 };
    int eyeY[2] = { 352, 350 };

    for (int i = 0; i < 2; i++)
    {
        // 外側のぼんやりした赤い光
        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha / 5);
        DrawCircle(eyeX[i], eyeY[i], 10, GetColor(255, 0, 0), TRUE);

        // 中間の赤い光
        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha / 3);
        DrawCircle(eyeX[i], eyeY[i], 6, GetColor(255, 0, 0), TRUE);

        // 内側の強い赤い光
        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);
        DrawCircle(eyeX[i], eyeY[i], 2, GetColor(255, 30, 30), TRUE);

        // 中心の白い光
        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);
        DrawCircle(eyeX[i], eyeY[i], 1, GetColor(255, 220, 220), TRUE);
    }

    // 描画設定を戻す
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ========================================
    // 煙：2層でゆっくり流す
    // ========================================
    if (smokeHandle >= 0)
    {
        // 奥の煙
        float smokeX1 = sinf(t * 0.15f) * 35.0f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 45);

        DrawExtendGraph(
            (int)(-80 + smokeX1), 180,
            (int)(1360 + smokeX1), 720,
            smokeHandle, TRUE
        );

        // 手前の煙
        float smokeX2 = cosf(t * 0.10f) * 55.0f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 75);

        DrawExtendGraph(
            (int)(-120 + smokeX2), 280,
            (int)(1400 + smokeX2), 760,
            smokeHandle, TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ========================================
    // 赤い警告灯：明滅
    // ========================================
    if (lightHandle >= 0)
    {
        // ゆっくり明るさを変える
        float lightPulse = (sinf(t * 1.5f) + 1.0f) * 0.5f;

        int lightAlpha = 35 + (int)(lightPulse * 90.0f);

        SetDrawBlendMode(DX_BLENDMODE_ADD, lightAlpha);

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
   // ========================================
   // タイトル：グリッチ＋走査線
   // ========================================
    if (titleFont >= 0)                            
    {
        // たまにグリッチを発生させる
        bool glitch = (animationTimer % 180 < 5);

        int glitchX = 0;

        if (glitch)
        {
            glitchX = (animationTimer % 2 == 0) ? 5 : -5;
        }

        // ========================================
        // 赤と青の色ズレ
        // ========================================
        if (glitch)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);

            // 赤い残像
            DrawStringToHandle(
                94 + glitchX + 5, 105,
                "SLOW",
                GetColor(255, 30, 50),
                titleFont
            );

            // 青い残像
            DrawStringToHandle(
                94 + glitchX - 5, 105,
                "SLOW",
                GetColor(0, 180, 255),
                titleFont
            );

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ========================================
        // 通常のタイトル文字
        // ========================================
        DrawStringToHandle(
            94 + glitchX, 105,
            "SLOW",
            GetColor(245, 245, 245),
            titleFont
        );

        // ========================================
        // 走査線：文字の上を光が通過
        // ========================================

        // 3秒周期で上から下へ
        float scanProgress = fmodf(t, 3.0f) / 3.0f;

        int scanY = 105 + (int)(scanProgress * 120.0f);

        // タイトルの範囲だけに描画
        SetDrawArea(94, 105, 450, 225);

        // ぼんやりした光
        SetDrawBlendMode(DX_BLENDMODE_ADD, 45);

        DrawBox(
            94, scanY - 6,
            450, scanY + 6,
            GetColor(0, 180, 255),
            TRUE
        );

        // 中心の細い光
        SetDrawBlendMode(DX_BLENDMODE_ADD, 120);

        DrawLine(
            94, scanY,
            450, scanY,
            GetColor(180, 230, 255),
            2
        );

        // 描画設定を戻す
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        SetDrawArea(0, 0, 1280, 720);

        // ========================================
// 横方向のグリッチノイズ
// ========================================
        if (animationTimer % 180 < 8)
        {
            // グリッチする文字の横帯
            int glitchY = 120 + (animationTimer * 17) % 85;

            // 横に大きくズラす
            int glitchOffset = (animationTimer % 2 == 0) ? 18 : -18;

            // 横帯の範囲だけ描画
            SetDrawArea(94, glitchY, 450, glitchY + 12);

            // 赤いズレ
            DrawStringToHandle(
                94 + glitchOffset + 4,
                105,
                "SLOW",
                GetColor(255, 30, 50),
                titleFont
            );

            // 白いズレ
            DrawStringToHandle(
                94 + glitchOffset,
                105,
                "SLOW",
                GetColor(255, 255, 255),
                titleFont
            );

            // 青いズレ
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);

            DrawStringToHandle(
                94 + glitchOffset - 5,
                105,
                "SLOW",
                GetColor(0, 200, 255),
                titleFont
            );

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // 描画範囲を戻す
            SetDrawArea(0, 0, 1280, 720);
        }
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
    int menuGlowAlpha = 90 + (int)(sinf(t * 0.075f) * 50.0f);

    SetDrawBlendMode(DX_BLENDMODE_ADD, menuGlowAlpha);
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