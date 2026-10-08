#include "Enemy.h"
#include <math.h>

// ========================================
// 初期化
// ========================================
void Enemy::Initialize(VECTOR startPosition)
{
    position = startPosition;

    radius = 1.0f;
    hp = 3;
    active = true;

    hitEffectTimer = 0;

    dying = false;
    deathEffectTimer = 0;

    shootTimer = 60;

    // 今はまだ3Dモデルを使わない
    modelHandle = -1;

    angleY = 0.0f;

    moveSpeed = 0.03f;
    moveDirection = 1;
    moveTimer = 120;
}

// ========================================
// 更新
// ========================================
void Enemy::Update(VECTOR playerPosition)
{
    if (!active)
    {
        return;
    }

    // 撃破演出中
    if (dying)
    {
        deathEffectTimer--;

        if (deathEffectTimer <= 0)
        {
            deathEffectTimer = 0;
            dying = false;
            active = false;
        }

        return;
    }

    // ヒット演出時間
    if (hitEffectTimer > 0)
    {
        hitEffectTimer--;
    }

    // プレイヤーへの方向
    VECTOR toPlayer = VSub(playerPosition, position);

    // プレイヤーの方向を向く
    angleY = atan2f(toPlayer.x, toPlayer.z);

    // プレイヤーに対して横方向
    VECTOR sideDirection = VGet(
        cosf(angleY),
        0.0f,
        -sinf(angleY)
    );

    // 左右移動
    position.x += sideDirection.x * moveSpeed * moveDirection;
    position.z += sideDirection.z * moveSpeed * moveDirection;

    // 一定時間ごとに方向転換
    moveTimer--;

    if (moveTimer <= 0)
    {
        moveDirection *= -1;
        moveTimer = 120;
    }
}

// ========================================
// ダメージ
// ========================================
void Enemy::Damage(int damage)
{
    if (!active || dying)
    {
        return;
    }

    hp -= damage;

    // ヒット演出開始
    hitEffectTimer = HIT_EFFECT_TIME;

    if (hp <= 0)
    {
        hp = 0;

        // 撃破演出へ
        dying = true;
        deathEffectTimer = DEATH_EFFECT_TIME;
    }
}

// ========================================
// 描画
// ========================================
void Enemy::Draw()
{
    if (!active)
    {
        return;
    }

    // ====================================
    // 撃破エフェクト
    // ====================================
    if (dying)
    {
        float progress =
            1.0f - (float)deathEffectTimer / DEATH_EFFECT_TIME;

        float size = 1.0f + progress * 4.0f;
        int alpha = (int)(255 * (1.0f - progress));

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

        DrawSphere3D(
            position,
            size,
            20,
            GetColor(255, 60, 30),
            GetColor(255, 60, 30),
            TRUE
        );

        DrawSphere3D(
            position,
            size * 0.45f,
            16,
            GetColor(255, 240, 150),
            GetColor(255, 255, 255),
            TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        return;
    }

    // ====================================
    // ヒットエフェクト
    // ====================================
    if (hitEffectTimer > 0)
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 180);

        DrawSphere3D(
            position,
            radius + 0.15f,
            16,
            GetColor(255, 255, 255),
            GetColor(255, 220, 150),
            TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        return;
    }

    // ====================================
    // 3Dモデルがある場合
    // ====================================
    if (modelHandle != -1)
    {
        MV1SetPosition(modelHandle, position);
        MV1SetRotationXYZ(
            modelHandle,
            VGet(0.0f, angleY, 0.0f)
        );

        MV1DrawModel(modelHandle);
    }
    // ====================================
    // 今は赤い球
    // ====================================
    else
    {
        DrawSphere3D(
            position,
            radius,
            16,
            GetColor(220, 50, 50),
            GetColor(255, 255, 255),
            TRUE
        );
    }
}