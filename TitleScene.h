#pragma once
#include "DxLib.h"

class TitleScene
{
private:
    // 画像レイヤー
    int backgroundHandle = -1;
    int soldierFrontHandle = -1;
    int soldierBackHandle = -1;
    int weaponHandle = -1;
    int smokeHandle = -1;
    int lightHandle = -1;

    // メニュー
    int selectedMenu = 0;
    float menuFrameY = 0.0f;

    // アニメーション
    int animationTimer = 0;

    // フォント
    int titleFont = -1;
    int subtitleFont = -1;
    int menuFont = -1;

    // 入力
    bool oldUp = false;
    bool oldDown = false;
    bool oldEnter = false;
    bool startRequested = false;

public:
    void Initialize();
    void Update();
    void Draw();
    void Finalize();
    bool IsStartSelected();
};