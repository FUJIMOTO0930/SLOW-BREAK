#include "DxLib.h"
#include <math.h>

#include "Bullet.h"
#include "Enemy.h"
#include "EnemyBullet.h"
#include "GameState.h"
#include "ImpactEffect.h"
#include "TitleScene.h"

// ========================================
// プレイヤー
// ========================================
VECTOR playerPos = VGet(0.0f, 1.7f, 0.0f);

float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float moveSpeed = 0.12f;
float mouseSensitivity = 0.003f;

int playerHP = 100;

// 被弾演出
int damageEffectTimer = 0;
const int DAMAGE_EFFECT_TIME = 12;

// ========================================
// ゲーム状態
// ========================================
GameState gameState = GAME_STATE_PLAY;
TitleScene titleScene;

// ========================================
// SLOW能力
// ========================================
bool isSlow = false;

const float MAX_SLOW_GAUGE = 100.0f;
float slowGauge = MAX_SLOW_GAUGE;

float slowRate = 0.2f;
float slowUseSpeed = 0.35f;
float slowRecoverySpeed = 0.15f;

// SLOW演出
bool wasSlow = false;
int slowStartEffect = 0;

const int SLOW_START_EFFECT_TIME = 15;

// ========================================
// 射撃演出
// ========================================
int muzzleFlashTimer = 0;
const int MUZZLE_FLASH_TIME = 4;

float recoil = 0.0f;

// ========================================
// プレイヤー弾
// ========================================
const int MAX_BULLETS = 100;
Bullet bullets[MAX_BULLETS];

// マガジン
const int MAX_AMMO = 10;
int currentAmmo = MAX_AMMO;

// リロード
bool isReloading = false;
int reloadTimer = 0;
const int RELOAD_TIME = 90;

// ========================================
// 敵
// ========================================
const int MAX_ENEMIES = 3;
Enemy enemies[MAX_ENEMIES];

// ========================================
// 敵弾
// ========================================
const int MAX_ENEMY_BULLETS = 300;
EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];

// ========================================
// 着弾エフェクト
// ========================================
const int MAX_IMPACT_EFFECTS = 50;
ImpactEffect impactEffects[MAX_IMPACT_EFFECTS];

// ========================================
// 壁との当たり判定
// ========================================
bool IsHitWall(VECTOR pos)
{
    const float playerRadius = 0.4f;

    // マップ外周
    if (pos.x - playerRadius < -20.0f) return true;
    if (pos.x + playerRadius > 20.0f) return true;
    if (pos.z + playerRadius > 20.0f) return true;

    // 左側の仕切り
    if (pos.x + playerRadius > -8.0f &&
        pos.x - playerRadius < -7.5f &&
        pos.z + playerRadius > 5.0f &&
        pos.z - playerRadius < 16.0f)
    {
        return true;
    }

    // 右側の仕切り
    if (pos.x + playerRadius > 7.5f &&
        pos.x - playerRadius < 8.0f &&
        pos.z + playerRadius > 5.0f &&
        pos.z - playerRadius < 16.0f)
    {
        return true;
    }

    // 中央の遮蔽物
    if (pos.x + playerRadius > -2.0f &&
        pos.x - playerRadius < 2.0f &&
        pos.z + playerRadius > 7.0f &&
        pos.z - playerRadius < 8.0f)
    {
        return true;
    }

    return false;
}

// ========================================
// 弾と壁の当たり判定
// ========================================
bool IsBulletHitWall(VECTOR pos)
{
    // マップ外周
    if (pos.x < -20.0f) return true;
    if (pos.x > 20.0f) return true;
    if (pos.z > 20.0f) return true;

    // 左側の仕切り
    if (pos.x > -8.0f && pos.x < -7.5f &&
        pos.z > 5.0f && pos.z < 16.0f)
    {
        return true;
    }

    // 右側の仕切り
    if (pos.x > 7.5f && pos.x < 8.0f &&
        pos.z > 5.0f && pos.z < 16.0f)
    {
        return true;
    }

    // 中央の遮蔽物
    if (pos.x > -2.0f && pos.x < 2.0f &&
        pos.z > 7.0f && pos.z < 8.0f)
    {
        return true;
    }

    return false;
}

// ========================================
// 着弾エフェクト発生
// ========================================
void CreateImpactEffect(VECTOR position)
{
    for (int i = 0; i < MAX_IMPACT_EFFECTS; i++)
    {
        if (!impactEffects[i].active)
        {
            impactEffects[i].active = true;
            impactEffects[i].position = position;
            impactEffects[i].timer = 15;

            for (int s = 0; s < 6; s++)
            {
                impactEffects[i].sparkPosition[s] = position;

                float x = (GetRand(200) - 100) / 100.0f;
                float y = GetRand(100) / 100.0f;
                float z = (GetRand(200) - 100) / 100.0f;

                VECTOR direction = VGet(x, y, z);

                if (VSize(direction) > 0.0f)
                {
                    direction = VNorm(direction);
                }

                impactEffects[i].sparkDirection[s] = direction;
            }

            break;
        }
    }
}

// ========================================
// ゲームリセット
// ========================================
void ResetGame()
{
    // プレイヤー
    playerPos = VGet(0.0f, 1.7f, 0.0f);
    playerHP = 100;
    damageEffectTimer = 0;

    cameraYaw = 0.0f;
    cameraPitch = 0.0f;

    // SLOW
    slowGauge = MAX_SLOW_GAUGE;
    isSlow = false;
    wasSlow = false;
    slowStartEffect = 0;

    muzzleFlashTimer = 0;
    recoil = 0.0f;

    // 弾数・リロード
    currentAmmo = MAX_AMMO;
    isReloading = false;
    reloadTimer = 0;

    // プレイヤー弾削除
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].active = false;
    }

    // 敵弾削除
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        enemyBullets[i].active = false;
    }

    // 着弾エフェクト削除
    for (int i = 0; i < MAX_IMPACT_EFFECTS; i++)
    {
        impactEffects[i].active = false;
    }

    // 敵初期化
    enemies[0].Initialize(VGet(-4.0f, 1.0f, 12.0f));
    enemies[1].Initialize(VGet(0.0f, 1.0f, 15.0f));
    enemies[2].Initialize(VGet(4.0f, 1.0f, 12.0f));

    // 射撃開始時間をずらす
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].shootTimer = 60 + i * 20;
    }

    gameState = GAME_STATE_PLAY;

}

// ========================================
// メイン
// ========================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ChangeWindowMode(TRUE);
    SetGraphMode(1280, 720, 32);

    if (DxLib_Init() == -1)
    {
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetCameraNearFar(0.1f, 1000.0f);

    // ====================================
    // フォント
    // ====================================
    int gameOverFont = CreateFontToHandle(NULL, 80, 8);
    int restartFont = CreateFontToHandle(NULL, 32, 4);
    int clearFont = CreateFontToHandle(NULL, 80, 8);

    // ====================================
    // マウス
    // ====================================
    int centerX = 1280 / 2;
    int centerY = 720 / 2;

    // タイトル画面ではカーソルを表示
    SetMouseDispFlag(TRUE);

    ResetGame();

    titleScene.Initialize();
    gameState = GAME_STATE_TITLE;

    int oldMouseInput = 0;

    SetDrawMode(DX_DRAWMODE_BILINEAR);

    // ====================================
    // ゲームループ
    // ====================================
    while (ProcessMessage() == 0)
    {
        ClearDrawScreen();

        // ========================================
        // タイトル画面
        // ========================================
        if (gameState == GAME_STATE_TITLE)
        {
            // カーソルを表示して自由に動かせる
            SetMouseDispFlag(TRUE);

            titleScene.Update();
            titleScene.Draw();

            if (titleScene.IsStartSelected())
            {
                ResetGame();
                gameState = GAME_STATE_PLAY;

                // ゲーム開始時はカーソルを非表示
                SetMouseDispFlag(FALSE);
                SetMousePoint(centerX, centerY);
            }

            // HOW TO PLAYを選んだとき
            if (titleScene.IsHowToPlaySelected())
            {
                gameState = GAME_STATE_HOW_TO_PLAY;
            }

            // OPTIONを選んだとき
            if (titleScene.IsOptionSelected())
            {
                gameState = GAME_STATE_OPTION;
            }

            ScreenFlip();
            continue;
        }

        // ========================================
        // OPTION画面
        // ========================================
        if (gameState == GAME_STATE_OPTION)
        {
            SetMouseDispFlag(TRUE);

            titleScene.UpdateOption();

            // OPTIONで変更した感度をゲームに反映
            mouseSensitivity = titleScene.GetMouseSensitivity();

            titleScene.DrawOption();

            // ESCキーでタイトルに戻る
            if (CheckHitKey(KEY_INPUT_ESCAPE))
            {
                gameState = GAME_STATE_TITLE;
            }

            ScreenFlip();
            continue;
        }

        if (damageEffectTimer > 0)
        {
            damageEffectTimer--;
        }

        // ====================================
        // GAME OVER
        // ====================================
        if (gameState == GAME_STATE_GAMEOVER)
        {
            if (CheckHitKey(KEY_INPUT_R))
            {
                ResetGame();
                oldMouseInput = 0;
                SetMousePoint(centerX, centerY);
            }
        }

        // ====================================
        // プレイ中
        // ====================================
        if (gameState == GAME_STATE_PLAY)
        {
            // ====================================
            // SLOW
            // ====================================
            if (CheckHitKey(KEY_INPUT_LSHIFT))
            {
                if (slowGauge > 0.0f)
                {
                    isSlow = true;
                    slowGauge -= slowUseSpeed;

                    if (slowGauge < 0.0f)
                    {
                        slowGauge = 0.0f;
                    }
                }
                else
                {
                    isSlow = false;
                }
            }
            else
            {
                isSlow = false;
                slowGauge += slowRecoverySpeed;

                if (slowGauge > MAX_SLOW_GAUGE)
                {
                    slowGauge = MAX_SLOW_GAUGE;
                }
            }

            // SLOWを発動した瞬間
            if (isSlow && !wasSlow)
            {
                slowStartEffect = SLOW_START_EFFECT_TIME;
            }

            wasSlow = isSlow;

            if (slowStartEffect > 0)
            {
                slowStartEffect--;
            }

            // ====================================
            // マウス視点
            // ====================================
            int mouseX;
            int mouseY;

            GetMousePoint(&mouseX, &mouseY);

            int deltaX = mouseX - centerX;
            int deltaY = mouseY - centerY;

            cameraYaw += deltaX * mouseSensitivity;
            cameraPitch -= deltaY * mouseSensitivity;

            if (cameraPitch > 1.4f)
            {
                cameraPitch = 1.4f;
            }

            if (cameraPitch < -1.4f)
            {
                cameraPitch = -1.4f;
            }

            SetMousePoint(centerX, centerY);

            // ====================================
            // 移動方向
            // ====================================
            VECTOR forward = VGet(
                sinf(cameraYaw),
                0.0f,
                cosf(cameraYaw)
            );

            VECTOR right = VGet(
                cosf(cameraYaw),
                0.0f,
                -sinf(cameraYaw)
            );

            // ====================================
            // WASD移動
            // ====================================
            VECTOR nextPos = playerPos;

            // 前後移動
            if (CheckHitKey(KEY_INPUT_W))
            {
                nextPos.x += forward.x * moveSpeed;
                nextPos.z += forward.z * moveSpeed;
            }

            if (CheckHitKey(KEY_INPUT_S))
            {
                nextPos.x -= forward.x * moveSpeed;
                nextPos.z -= forward.z * moveSpeed;
            }

            // 横移動
            if (CheckHitKey(KEY_INPUT_D))
            {
                nextPos.x += right.x * moveSpeed;
                nextPos.z += right.z * moveSpeed;
            }

            if (CheckHitKey(KEY_INPUT_A))
            {
                nextPos.x -= right.x * moveSpeed;
                nextPos.z -= right.z * moveSpeed;
            }

            // X方向だけ移動できるか確認
            VECTOR checkX = playerPos;
            checkX.x = nextPos.x;

            if (!IsHitWall(checkX))
            {
                playerPos.x = checkX.x;
            }

            // Z方向だけ移動できるか確認
            VECTOR checkZ = playerPos;
            checkZ.z = nextPos.z;

            if (!IsHitWall(checkZ))
            {
                playerPos.z = checkZ.z;
            }
            // ====================================
            // カメラ方向
            // ====================================
            VECTOR cameraDirection = VGet(
                sinf(cameraYaw) * cosf(cameraPitch),
                sinf(cameraPitch),
                cosf(cameraYaw) * cosf(cameraPitch)
            );

            // ====================================
            // リロード
            // ====================================
            if (!isReloading &&
                currentAmmo < MAX_AMMO &&
                CheckHitKey(KEY_INPUT_R))
            {
                isReloading = true;
                reloadTimer = RELOAD_TIME;
            }

            if (isReloading)
            {
                reloadTimer--;

                if (reloadTimer <= 0)
                {
                    currentAmmo = MAX_AMMO;
                    isReloading = false;
                }
            }

            // ====================================
            // プレイヤー射撃
            // ====================================
            int mouseInput = GetMouseInput();

            if ((mouseInput & MOUSE_INPUT_LEFT) &&
                !(oldMouseInput & MOUSE_INPUT_LEFT) &&
                currentAmmo > 0 &&
                !isReloading)
            {
                for (int i = 0; i < MAX_BULLETS; i++)
                {
                    if (!bullets[i].active)
                    {
                        bullets[i].active = true;
                        bullets[i].position = playerPos;
                        bullets[i].direction = cameraDirection;
                        bullets[i].speed = 0.8f;

                        currentAmmo--;

                        // 射撃演出
                        muzzleFlashTimer = MUZZLE_FLASH_TIME;
                        recoil += 0.018f;

                        if (recoil > 0.05f)
                        {
                            recoil = 0.05f;
                        }

                        break;
                    }
                }
            }

            oldMouseInput = mouseInput;

            // ====================================
            // 射撃反動
            // ====================================
           /* if (recoil > 0.0f)
            {
                cameraPitch += recoil;
                recoil *= 0.35f;

                if (recoil < 0.001f)
                {
                    recoil = 0.0f;
                }
            } */

            if (muzzleFlashTimer > 0)
            {
                muzzleFlashTimer--;
            }

           

            // ====================================
            // プレイヤー弾移動
            // ====================================
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (!bullets[i].active)
                {
                    continue;
                }

                bullets[i].position = VAdd(
                    bullets[i].position,
                    VScale(
                        bullets[i].direction,
                        bullets[i].speed
                    )
                );

                // 壁に当たったら弾を消す
                if (IsBulletHitWall(bullets[i].position))
                {
                    CreateImpactEffect(bullets[i].position);

                    bullets[i].active = false;
                    continue;
                }

                VECTOR distance = VSub(
                    bullets[i].position,
                    playerPos
                );

                if (VSize(distance) > 100.0f)
                {
                    bullets[i].active = false;
                }
            }

            // ====================================
            // プレイヤー弾 × 敵
            // ====================================
            for (int b = 0; b < MAX_BULLETS; b++)
            {
                if (!bullets[b].active)
                {
                    continue;
                }

                for (int e = 0; e < MAX_ENEMIES; e++)
                {
                    if (!enemies[e].active || enemies[e].dying)
                    {
                        continue;
                    }

                    VECTOR distance = VSub(
                        bullets[b].position,
                        enemies[e].position
                    );

                    if (VSize(distance) < enemies[e].radius)
                    {
                        bullets[b].active = false;
                        enemies[e].Damage(1);

                        break;
                    }
                }
            }

            // ====================================
            // 敵全滅チェック
            // ====================================
            bool allEnemiesDefeated = true;

            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                if (enemies[i].active)
                {
                    allEnemiesDefeated = false;
                    break;
                }
            }

            if (allEnemiesDefeated)
            {
                gameState = GAME_STATE_CLEAR;
                isSlow = false;
            }

            // ====================================
            // 敵更新
            // ====================================
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                enemies[i].Update(playerPos);
            }

            // ====================================
            // 敵射撃
            // ====================================
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (!enemies[e].active || enemies[e].dying)
                {
                    continue;
                }

                enemies[e].shootTimer--;

                if (enemies[e].shootTimer <= 0)
                {
                    for (int b = 0; b < MAX_ENEMY_BULLETS; b++)
                    {
                        if (!enemyBullets[b].active)
                        {
                            enemyBullets[b].active = true;
                            enemyBullets[b].position = enemies[e].position;

                            // 残像位置初期化
                            for (int t = 0; t < TRAIL_COUNT; t++)
                            {
                                enemyBullets[b].trailPositions[t] =
                                    enemyBullets[b].position;
                            }

                            VECTOR direction = VSub(
                                playerPos,
                                enemies[e].position
                            );

                            enemyBullets[b].direction =
                                VNorm(direction);

                            enemyBullets[b].speed = 0.15f;

                            break;
                        }
                    }

                    enemies[e].shootTimer = 60;
                }
            }

            // ====================================
            // 敵弾移動
            // ====================================
            for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
            {
                if (!enemyBullets[i].active)
                {
                    continue;
                }

                // 過去位置を後ろへずらす
                for (int t = TRAIL_COUNT - 1; t > 0; t--)
                {
                    enemyBullets[i].trailPositions[t] =
                        enemyBullets[i].trailPositions[t - 1];
                }

                // 現在位置を保存
                enemyBullets[i].trailPositions[0] =
                    enemyBullets[i].position;

                float currentSpeed =
                    enemyBullets[i].speed;

                if (isSlow)
                {
                    currentSpeed *= slowRate;
                }

                enemyBullets[i].position = VAdd(
                    enemyBullets[i].position,
                    VScale(
                        enemyBullets[i].direction,
                        currentSpeed
                    )
                );

                // 壁に当たったら敵弾を消す
                if (IsBulletHitWall(enemyBullets[i].position))
                {
                    CreateImpactEffect(enemyBullets[i].position);

                    enemyBullets[i].active = false;
                    continue;
                }

                VECTOR distance = VSub(
                    enemyBullets[i].position,
                    playerPos
                );

                // プレイヤーに命中
                if (VSize(distance) < 0.5f)
                {
                    enemyBullets[i].active = false;
                    playerHP -= 10;

                    damageEffectTimer = DAMAGE_EFFECT_TIME;

                    if (playerHP <= 0)
                    {
                        playerHP = 0;
                        gameState = GAME_STATE_GAMEOVER;
                        isSlow = false;

                        break;
                    }
                }

                // 遠くまで飛んだ弾を削除
                VECTOR bulletDistance = VSub(
                    enemyBullets[i].position,
                    playerPos
                );

                if (VSize(bulletDistance) > 100.0f)
                {
                    enemyBullets[i].active = false;
                }
            }

            // ====================================
            // 着弾エフェクト更新
            // ====================================
            for (int i = 0; i < MAX_IMPACT_EFFECTS; i++)
            {
                if (!impactEffects[i].active)
                {
                    continue;
                }

                impactEffects[i].timer--;

                for (int s = 0; s < 6; s++)
                {
                    impactEffects[i].sparkPosition[s] = VAdd(
                        impactEffects[i].sparkPosition[s],
                        VScale(impactEffects[i].sparkDirection[s], 0.08f)
                    );

                    // 火花を少し下へ落とす
                    impactEffects[i].sparkDirection[s].y -= 0.03f;
                }

                if (impactEffects[i].timer <= 0)
                {
                    impactEffects[i].active = false;
                }
            }
        }

        // ====================================
        // カメラ
        // ====================================
        VECTOR drawCameraDirection = VGet(
            sinf(cameraYaw) * cosf(cameraPitch),
            sinf(cameraPitch),
            cosf(cameraYaw) * cosf(cameraPitch)
        );

        VECTOR cameraTarget = VAdd(
            playerPos,
            VScale(drawCameraDirection, 10.0f)
        );

        // 被弾時のカメラ揺れ
        VECTOR cameraShake = VGet(0.0f, 0.0f, 0.0f);

        if (damageEffectTimer > 0)
        {
            float shakePower =
                0.025f * ((float)damageEffectTimer / DAMAGE_EFFECT_TIME);

            if (damageEffectTimer % 2 == 0)
            {
                cameraShake.x = shakePower;
                cameraShake.y = -shakePower;
            }
            else
            {
                cameraShake.x = -shakePower;
                cameraShake.y = shakePower;
            }
        }

        SetCameraPositionAndTarget_UpVecY(
            VAdd(playerPos, cameraShake),
            VAdd(cameraTarget, cameraShake)
        );

        // ====================================
        // 床
        // ====================================
        DrawCube3D(
            VGet(-20.0f, -0.5f, -20.0f),
            VGet(20.0f, 0.0f, 20.0f),
            GetColor(100, 100, 100),
            GetColor(100, 100, 100),
            TRUE
        );

        // ====================================
        // 研究施設マップ
        // ====================================
        // 奥の壁
        DrawCube3D(
            VGet(-20.0f, 0.0f, 20.0f),
            VGet(20.0f, 4.0f, 20.5f),
            GetColor(120, 125, 135),
            GetColor(80, 85, 95),
            TRUE
        );

        // 左の壁
        DrawCube3D(
            VGet(-20.5f, 0.0f, -20.0f),
            VGet(-20.0f, 4.0f, 20.0f),
            GetColor(120, 125, 135),
            GetColor(80, 85, 95),
            TRUE
        );

        // 右の壁
        DrawCube3D(
            VGet(20.0f, 0.0f, -20.0f),
            VGet(20.5f, 4.0f, 20.0f),
            GetColor(120, 125, 135),
            GetColor(80, 85, 95),
            TRUE
        );

        // 左側の仕切り
        DrawCube3D(
            VGet(-8.0f, 0.0f, 5.0f),
            VGet(-7.5f, 3.0f, 16.0f),
            GetColor(100, 105, 115),
            GetColor(70, 75, 85),
            TRUE
        );

        // 右側の仕切り
        DrawCube3D(
            VGet(7.5f, 0.0f, 5.0f),
            VGet(8.0f, 3.0f, 16.0f),
            GetColor(100, 105, 115),
            GetColor(70, 75, 85),
            TRUE
        );

        // 中央の遮蔽物
        DrawCube3D(
            VGet(-2.0f, 0.0f, 7.0f),
            VGet(2.0f, 1.5f, 8.0f),
            GetColor(80, 90, 100),
            GetColor(60, 70, 80),
            TRUE
        );

        // ====================================
        // 敵描画
        // ====================================
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            enemies[i].Draw();
        }

        // ====================================
        // プレイヤー弾描画
        // ====================================
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (!bullets[i].active)
            {
                continue;
            }

            DrawSphere3D(
                bullets[i].position,
                0.08f,
                8,
                GetColor(255, 230, 50),
                GetColor(255, 100, 0),
                TRUE
            );
        }

        // ====================================
        // 敵弾描画
        // ====================================
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        {
            if (!enemyBullets[i].active)
            {
                continue;
            }

            // SLOW中だけ残像表示
            if (isSlow)
            {
                for (int t = 0; t < TRAIL_COUNT; t++)
                {
                    float rate =
                        1.0f - (float)t / TRAIL_COUNT;

                    float trailSize =
                        0.05f * rate;

                    DrawSphere3D(
                        enemyBullets[i].trailPositions[t],
                        trailSize,
                        6,
                        GetColor(180, 60, 60),
                        GetColor(180, 60, 60),
                        TRUE
                    );
                }
            }

            // 弾本体
            DrawSphere3D(
                enemyBullets[i].position,
                0.07f,
                8,
                GetColor(255, 70, 50),
                GetColor(255, 180, 100),
                TRUE
            );
        }

        // ====================================
// 着弾エフェクト描画
// ====================================
        for (int i = 0; i < MAX_IMPACT_EFFECTS; i++)
        {
            if (!impactEffects[i].active)
            {
                continue;
            }

            int alpha = impactEffects[i].timer * 12;

            if (alpha > 255)
            {
                alpha = 255;
            }

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

            // 着弾点の光
            DrawSphere3D(
                impactEffects[i].position,
                0.12f,
                8,
                GetColor(255, 240, 180),
                GetColor(255, 180, 50),
                TRUE
            );

            // 火花
            for (int s = 0; s < 6; s++)
            {
                DrawSphere3D(
                    impactEffects[i].sparkPosition[s],
                    0.035f,
                    6,
                    GetColor(255, 180, 50),
                    GetColor(255, 100, 20),
                    TRUE
                );
            }

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ====================================
        // 被弾画面演出
        // ====================================
        if (damageEffectTimer > 0)
        {
            float rate =
                (float)damageEffectTimer / DAMAGE_EFFECT_TIME;

            int alpha = (int)(90 * rate);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            DrawBox(
                0, 0,
                1280, 720,
                GetColor(180, 20, 20),
                TRUE
            );

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ====================================
        // マズルフラッシュ
        // ====================================
        if (muzzleFlashTimer > 0 &&
            gameState == GAME_STATE_PLAY)
        {
            int flashSize = 18 + muzzleFlashTimer * 3;

            SetDrawBlendMode(DX_BLENDMODE_ADD, 180);

            DrawCircle(
                centerX + 120,
                centerY + 100,
                flashSize,
                GetColor(255, 200, 80),
                TRUE
            );

            DrawCircle(
                centerX + 120,
                centerY + 100,
                flashSize / 2,
                GetColor(255, 255, 220),
                TRUE
            );

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ====================================
        // 照準
        // ====================================
        if (gameState == GAME_STATE_PLAY)
        {
            DrawLine(
                centerX - 10,
                centerY,
                centerX + 10,
                centerY,
                GetColor(255, 255, 255)
            );

            DrawLine(
                centerX,
                centerY - 10,
                centerX,
                centerY + 10,
                GetColor(255, 255, 255)
            );
        }

        // ====================================
        // SLOW画面演出
        // ====================================
        if (isSlow && gameState == GAME_STATE_PLAY)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 25);

            DrawBox(
                0, 0,
                1280, 720,
                GetColor(40, 100, 180),
                TRUE
            );

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // SLOW発動瞬間
        if (slowStartEffect > 0 &&
            gameState == GAME_STATE_PLAY)
        {
            float rate =
                (float)slowStartEffect /
                SLOW_START_EFFECT_TIME;

            int alpha = (int)(100 * rate);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            // 上
            DrawBox(
                0, 0,
                1280, 12,
                GetColor(80, 180, 255),
                TRUE
            );

            // 下
            DrawBox(
                0, 708,
                1280, 720,
                GetColor(80, 180, 255),
                TRUE
            );

            // 左
            DrawBox(
                0, 0,
                12, 720,
                GetColor(80, 180, 255),
                TRUE
            );

            // 右
            DrawBox(
                1268, 0,
                1280, 720,
                GetColor(80, 180, 255),
                TRUE
            );

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ====================================
        // HP表示
        // ====================================
        DrawFormatString(
            20,
            20,
            GetColor(255, 255, 255),
            "HP : %d / 100",
            playerHP
        );

        DrawBox(
            20,
            50,
            320,
            75,
            GetColor(50, 50, 50),
            TRUE
        );

        DrawBox(
            20,
            50,
            20 + playerHP * 3,
            75,
            GetColor(50, 220, 80),
            TRUE
        );

        // ====================================
        // 弾数表示
        // ====================================
        DrawFormatString(
            1100,
            650,
            GetColor(255, 255, 255),
            "AMMO %d / %d",
            currentAmmo,
            MAX_AMMO
        );

        if (isReloading)
        {
            DrawString(
                1100,
                620,
                "RELOADING...",
                GetColor(255, 230, 50)
            );
        }

        // ====================================
        // SLOWゲージ
        // ====================================
        DrawString(
            20,
            95,
            "SLOW",
            GetColor(100, 200, 255)
        );

        DrawBox(
            20,
            120,
            320,
            145,
            GetColor(50, 50, 50),
            TRUE
        );

        int slowBarWidth =
            (int)((slowGauge / MAX_SLOW_GAUGE) * 300.0f);

        DrawBox(
            20,
            120,
            20 + slowBarWidth,
            145,
            GetColor(50, 150, 255),
            TRUE
        );

        // ====================================
        // SLOW中表示
        // ====================================
        if (isSlow && gameState == GAME_STATE_PLAY)
        {
            DrawString(
                centerX - 35,
                100,
                "SLOW",
                GetColor(100, 200, 255)
            );
        }

        // ====================================
        // GAME OVER
        // ====================================
        if (gameState == GAME_STATE_GAMEOVER)
        {
            DrawStringToHandle(
                centerX - 220,
                centerY - 100,
                "GAME OVER",
                GetColor(255, 50, 50),
                gameOverFont
            );

            DrawStringToHandle(
                centerX - 160,
                centerY + 20,
                "Press ",
                GetColor(255, 255, 255),
                restartFont
            );

            if ((GetNowCount() / 500) % 2 == 0)
            {
                DrawStringToHandle(
                    centerX - 55,
                    centerY + 20,
                    "R",
                    GetColor(255, 230, 0),
                    restartFont
                );
            }

            DrawStringToHandle(
                centerX - 30,
                centerY + 20,
                " to Restart",
                GetColor(255, 255, 255),
                restartFont
            );
        }

        // ====================================
        // STAGE CLEAR
        // ====================================
        if (gameState == GAME_STATE_CLEAR)
        {
            DrawStringToHandle(
                centerX - 250,
                centerY - 50,
                "STAGE CLEAR",
                GetColor(255, 230, 50),
                clearFont
            );
        }

        ScreenFlip();
    }

    // ====================================
    // 終了処理
    // ====================================
    DeleteFontToHandle(gameOverFont);
    DeleteFontToHandle(restartFont);
    DeleteFontToHandle(clearFont);

    titleScene.Finalize();

    DxLib_End();

    return 0;
}