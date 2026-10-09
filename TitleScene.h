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
    bool howToPlayRequested = false;
    bool optionRequested = false;
  
    int breakGraphHandle = -1;

    // OPTION画面の選択項目
    int selectedOption = 0;

    // マウス感度設定
    int mouseSensitivityLevel = 50;

    bool oldOptionLeft = false;
    bool oldOptionRight = false;

    // 左右キー長押し用
    int optionLeftHold = 0;
    int optionRightHold = 0;

    // マウスドラッグ用
    bool draggingSensitivity = false;

    // OPTION画面のキー入力
    bool oldOptionUp = false;
    bool oldOptionDown = false;

public:
    void Initialize();
    void Update();
    void Draw();
    void DrawHowToPlay();
    void DrawOption();
    void UpdateOption();
    float GetMouseSensitivity();
    void Finalize();

    bool IsStartSelected();
    bool IsHowToPlaySelected();
    bool IsOptionSelected();
};