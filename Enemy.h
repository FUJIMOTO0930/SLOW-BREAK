#pragma once

#include "DxLib.h"

class Enemy
{
public:
    VECTOR position;

    float radius;
    int hp;
    bool active;

    // ヒット演出
    int hitEffectTimer;
    const static int HIT_EFFECT_TIME = 6;
  
    // 撃破演出
    bool dying;
    int deathEffectTimer;

    const static int DEATH_EFFECT_TIME = 20;

    int shootTimer;

    // 3Dモデル用
    int modelHandle;

    // 敵の向き
    float angleY;

    // 移動
    float moveSpeed;
    int moveDirection;
    int moveTimer;

    // ====================================
    // 初期化
    // ====================================
    void Initialize(VECTOR startPosition);

    // ====================================
    // 更新
    // ====================================
    void Update(VECTOR playerPosition);

    // ====================================
    // ダメージ
    // ====================================
    void Damage(int damage);

    // ====================================
    // 描画
    // ====================================
    void Draw();
};