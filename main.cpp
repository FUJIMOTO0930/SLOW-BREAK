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

    // マップ外周（前後）
    if (pos.z - playerRadius < -30.0f) return true;
    if (pos.z + playerRadius > 30.0f) return true;

    // 左右の研究室を含む外側の限界
    if (pos.x - playerRadius < -50.0f) return true;
    if (pos.x + playerRadius > 50.0f) return true;

    // 左壁：入口以外は通れない
    if (pos.x - playerRadius < -30.0f &&
        pos.x + playerRadius > -30.5f &&
        (pos.z - playerRadius < -4.0f ||
            pos.z + playerRadius > 4.0f))
    {
        return true;
    }

    // 右壁：入口以外は通れない
    if (pos.x + playerRadius > 30.0f &&
        pos.x - playerRadius < 30.5f &&
        (pos.z - playerRadius < -4.0f ||
            pos.z + playerRadius > 4.0f))
    {
        return true;
    }

    // 研究室の前後の壁
    if (pos.x - playerRadius < -30.0f ||
        pos.x + playerRadius > 30.0f)
    {
        if (pos.z - playerRadius < -10.0f ||
            pos.z + playerRadius > 10.0f)
        {
            return true;
        }
    }

    // 柱4本の当たり判定
    const float pillarX[] = { -16.0f, 16.0f };
    const float pillarZ[] = { -13.0f, 13.0f };

    for (int x = 0; x < 2; x++)
    {
        for (int z = 0; z < 2; z++)
        {
            if (pos.x + playerRadius > pillarX[x] - 1.0f &&
                pos.x - playerRadius < pillarX[x] + 1.0f &&
                pos.z + playerRadius > pillarZ[z] - 1.0f &&
                pos.z - playerRadius < pillarZ[z] + 1.0f)
            {
                return true;
            }
        }
    }

    // 研究設備4つの当たり判定
    const float deviceX[] = { -7.0f, 7.0f };
    const float deviceZ[] = { -7.0f, 7.0f };

    for (int x = 0; x < 2; x++)
    {
        for (int z = 0; z < 2; z++)
        {
            if (pos.x + playerRadius > deviceX[x] - 2.0f &&
                pos.x - playerRadius < deviceX[x] + 2.0f &&
                pos.z + playerRadius > deviceZ[z] - 1.0f &&
                pos.z - playerRadius < deviceZ[z] + 1.0f)
            {
                return true;
            }
        }
    }

    return false;
}

// ========================================
// 弾と壁の当たり判定
// ========================================
bool IsBulletHitWall(VECTOR pos)
{
    // 外側の限界
    if (pos.x < -50.0f || pos.x > 50.0f) return true;
    if (pos.z < -30.0f || pos.z > 30.0f) return true;

    // 左右の入口以外の壁
    if ((pos.x >= -30.5f && pos.x <= -30.0f) ||
        (pos.x >= 30.0f && pos.x <= 30.5f))
    {
        if (pos.z < -4.0f || pos.z > 4.0f)
            return true;
    }

    // 研究室の前後の壁
    if (pos.x < -30.0f || pos.x > 30.0f)
    {
        if (pos.z < -10.0f || pos.z > 10.0f)
            return true;
    }

    // 柱4本の当たり判定
    const float pillarX[] = { -16.0f, 16.0f };
    const float pillarZ[] = { -13.0f, 13.0f };

    for (int x = 0; x < 2; x++)
    {
        for (int z = 0; z < 2; z++)
        {
            if (pos.x >= pillarX[x] - 1.0f &&
                pos.x <= pillarX[x] + 1.0f &&
                pos.z >= pillarZ[z] - 1.0f &&
                pos.z <= pillarZ[z] + 1.0f)
            {
                return true;
            }
        }
    }

    // 研究設備4つの当たり判定
    const float deviceX[] = { -7.0f, 7.0f };
    const float deviceZ[] = { -7.0f, 7.0f };

    for (int x = 0; x < 2; x++)
    {
        for (int z = 0; z < 2; z++)
        {
            if (pos.x >= deviceX[x] - 2.0f &&
                pos.x <= deviceX[x] + 2.0f &&
                pos.z >= deviceZ[z] - 1.0f &&
                pos.z <= deviceZ[z] + 1.0f &&
                pos.y >= 0.0f &&
                pos.y <= 1.6f)
            {
                return true;
            }
        }
    }

    return false;
}


// ========================================
// 弾の移動経路と壁の当たり判定
// ========================================
bool IsBulletPathHitWall(VECTOR start, VECTOR end, VECTOR* hitPosition)
{
    VECTOR movement = VSub(end, start);
    float distance = VSize(movement);

    // 0.1単位以下で細かく判定
    int steps = (int)ceilf(distance / 0.1f);
    if (steps < 1) steps = 1;

    for (int i = 1; i <= steps; i++)
    {
        float t = (float)i / steps;
        VECTOR pos = VAdd(start, VScale(movement, t));

        if (IsBulletHitWall(pos))
        {
            if (hitPosition != nullptr)
            {
                *hitPosition = pos;
            }
            return true;
        }
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
            SetMouseDispFlag(TRUE);

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

            // EXITを選んだらゲームを終了
            if (titleScene.IsExitSelected())
            {
                break;
            }

            ScreenFlip();
            continue;
        }


        // ========================================
        // HOW TO PLAY画面
        // ========================================
        if (gameState == GAME_STATE_HOW_TO_PLAY)
        {
            // マウスカーソルを表示
            SetMouseDispFlag(TRUE);

            // 操作説明画面を描画
            titleScene.DrawHowToPlay();

            // ESCキーでタイトルへ戻る
            if (CheckHitKey(KEY_INPUT_ESCAPE))
            {
                gameState = GAME_STATE_TITLE;
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
            VECTOR moveDirection = VGet(0.0f, 0.0f, 0.0f);

            // 前後移動
            if (CheckHitKey(KEY_INPUT_W))
            {
                moveDirection = VAdd(moveDirection, forward);
            }

            if (CheckHitKey(KEY_INPUT_S))
            {
                moveDirection = VSub(moveDirection, forward);
            }

            // 左右移動
            if (CheckHitKey(KEY_INPUT_D))
            {
                moveDirection = VAdd(moveDirection, right);
            }

            if (CheckHitKey(KEY_INPUT_A))
            {
                moveDirection = VSub(moveDirection, right);
            }

            // 斜め移動でも速度が変わらないように正規化
            if (VSize(moveDirection) > 0.0f)
            {
                moveDirection = VNorm(moveDirection);

                nextPos = VAdd(
                    playerPos,
                    VScale(moveDirection, moveSpeed)
                );
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


                VECTOR oldPosition = bullets[i].position;

                VECTOR nextPosition = VAdd(
                    oldPosition,
                    VScale(bullets[i].direction, bullets[i].speed)
                );

                VECTOR hitPosition;

                if (IsBulletPathHitWall(oldPosition, nextPosition, &hitPosition))
                {
                    CreateImpactEffect(hitPosition);
                    bullets[i].active = false;
                    continue;
                }

                bullets[i].position = nextPosition;

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

            // クリア確定後は敵の更新・射撃を行わない
            if (gameState == GAME_STATE_PLAY)
            {
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

                                for (int t = 0; t < TRAIL_COUNT; t++)
                                {
                                    enemyBullets[b].trailPositions[t] =
                                        enemyBullets[b].position;
                                }

                                VECTOR direction = VSub(
                                    playerPos,
                                    enemies[e].position
                                );

                                enemyBullets[b].direction = VNorm(direction);
                                enemyBullets[b].speed = 0.15f;
                                break;
                            }
                        }

                        enemies[e].shootTimer = 60;
                    }
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


                VECTOR oldPosition = enemyBullets[i].position;

                VECTOR nextPosition = VAdd(
                    oldPosition,
                    VScale(enemyBullets[i].direction, currentSpeed)
                );

                VECTOR hitPosition;

                if (IsBulletPathHitWall(oldPosition, nextPosition, &hitPosition))
                {
                    CreateImpactEffect(hitPosition);
                    enemyBullets[i].active = false;
                    continue;
                }

                enemyBullets[i].position = nextPosition;


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
        // 近未来研究施設：中央戦闘エリア
        // ====================================

        // 床
        DrawCube3D(
            VGet(-30.0f, -0.5f, -30.0f),
            VGet(30.0f, 0.0f, 30.0f),
            GetColor(75, 85, 100),
            GetColor(95, 110, 125),
            TRUE
        );

        // 奥の壁
        DrawCube3D(
            VGet(-30.0f, 0.0f, 30.0f),
            VGet(30.0f, 8.0f, 30.5f),
            GetColor(120, 135, 150),
            GetColor(70, 85, 100),
            TRUE
        );

        // 手前の壁
        DrawCube3D(
            VGet(-30.0f, 0.0f, -30.5f),
            VGet(30.0f, 8.0f, -30.0f),
            GetColor(120, 135, 150),
            GetColor(70, 85, 100),
            TRUE
        );

        // ====================================
        // 左右の壁：研究室への入口付き
        // ====================================

        // 左壁：手前側
        DrawCube3D(
            VGet(-30.5f, 0.0f, -30.0f),
            VGet(-30.0f, 8.0f, -4.0f),
            GetColor(120, 135, 150),
            GetColor(70, 85, 100), TRUE
        );

        // 左壁：奥側
        DrawCube3D(
            VGet(-30.5f, 0.0f, 4.0f),
            VGet(-30.0f, 8.0f, 30.0f),
            GetColor(120, 135, 150),
            GetColor(70, 85, 100), TRUE
        );

        // 右壁：手前側
        DrawCube3D(
            VGet(30.0f, 0.0f, -30.0f),
            VGet(30.5f, 8.0f, -4.0f),
            GetColor(120, 135, 150),
            GetColor(70, 85, 100), TRUE
        );

        // 右壁：奥側
        DrawCube3D(
            VGet(30.0f, 0.0f, 4.0f),
            VGet(30.5f, 8.0f, 30.0f),
            GetColor(120, 135, 150),
            GetColor(70, 85, 100), TRUE
        );

        // ====================================
        // 近未来研究施設：柱4本
        // ====================================

        // 左奥の柱
        DrawCube3D(
            VGet(-17.0f, 0.0f, 12.0f),
            VGet(-15.0f, 8.0f, 14.0f),
            GetColor(100, 125, 145),
            GetColor(60, 85, 110),
            TRUE
        );

        // 右奥の柱
        DrawCube3D(
            VGet(15.0f, 0.0f, 12.0f),
            VGet(17.0f, 8.0f, 14.0f),
            GetColor(100, 125, 145),
            GetColor(60, 85, 110),
            TRUE
        );

        // 左手前の柱
        DrawCube3D(
            VGet(-17.0f, 0.0f, -14.0f),
            VGet(-15.0f, 8.0f, -12.0f),
            GetColor(100, 125, 145),
            GetColor(60, 85, 110),
            TRUE
        );

        // 右手前の柱
        DrawCube3D(
            VGet(15.0f, 0.0f, -14.0f),
            VGet(17.0f, 8.0f, -12.0f),
            GetColor(100, 125, 145),
            GetColor(60, 85, 110),
            TRUE
        );

        // ====================================
        // 近未来研究施設：研究設備4つ
        // ====================================

        const float deviceX[] = { -7.0f, 7.0f };
        const float deviceZ[] = { -7.0f, 7.0f };

        for (int x = 0; x < 2; x++)
        {
            for (int z = 0; z < 2; z++)
            {
                float px = deviceX[x];
                float pz = deviceZ[z];

                // 装置本体
                DrawCube3D(
                    VGet(px - 2.0f, 0.0f, pz - 1.0f),
                    VGet(px + 2.0f, 1.5f, pz + 1.0f),
                    GetColor(65, 85, 105),
                    GetColor(35, 55, 75),
                    TRUE
                );

                // ====================================
                // 上部の発光パネル
                // ====================================

                // 照明の影響を受けない
                SetUseLighting(FALSE);

                // パネルの土台（暗いフレーム）
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                DrawCube3D(
                    VGet(px - 1.85f, 1.5f, pz - 0.9f),
                    VGet(px + 1.85f, 1.56f, pz + 0.9f),
                    GetColor(15, 35, 50),
                    GetColor(15, 35, 50),
                    TRUE
                );

                // フレームの内側に青い発光パネル
                DrawCube3D(
                    VGet(px - 1.7f, 1.56f, pz - 0.75f),
                    VGet(px + 1.7f, 1.59f, pz + 0.75f),
                    GetColor(80, 230, 255),
                    GetColor(80, 230, 255),
                    TRUE
                );

                // パネルのすぐ上に弱い光を重ねる
                SetWriteZBuffer3D(FALSE);
                SetDrawBlendMode(DX_BLENDMODE_ADD, 65);

                DrawCube3D(
                    VGet(px - 1.75f, 1.60f, pz - 0.8f),
                    VGet(px + 1.75f, 1.62f, pz + 0.8f),
                    GetColor(30, 150, 255),
                    GetColor(30, 150, 255),
                    TRUE
                );

                // 描画設定を戻す
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                SetWriteZBuffer3D(TRUE);
                SetUseLighting(TRUE);
            }
        }

        // ====================================
        // 近未来研究施設：左右の研究室
        // ====================================

        // 左研究室の床
        DrawCube3D(
            VGet(-50.0f, -0.5f, -10.0f),
            VGet(-30.0f, 0.0f, 10.0f),
            GetColor(80, 95, 110),
            GetColor(60, 75, 90),
            TRUE
        );

        // 右研究室の床
        DrawCube3D(
            VGet(30.0f, -0.5f, -10.0f),
            VGet(50.0f, 0.0f, 10.0f),
            GetColor(80, 95, 110),
            GetColor(60, 75, 90),
            TRUE
        );

        // 左研究室：奥と手前の壁
        DrawCube3D(VGet(-50, 0, -10.5f), VGet(-30, 5, -10),
            GetColor(115, 135, 150), GetColor(65, 85, 100), TRUE);

        DrawCube3D(VGet(-50, 0, 10), VGet(-30, 5, 10.5f),
            GetColor(115, 135, 150), GetColor(65, 85, 100), TRUE);

        // 左研究室：外側の壁
        DrawCube3D(VGet(-50.5f, 0, -10), VGet(-50, 5, 10),
            GetColor(115, 135, 150), GetColor(65, 85, 100), TRUE);

        // 右研究室：奥と手前の壁
        DrawCube3D(
            VGet(30.5f, 0, -10.5f),
            VGet(50, 5, -10),
            GetColor(115, 135, 150),
            GetColor(65, 85, 100), TRUE
        );

        DrawCube3D(
            VGet(30.5f, 0, 10),
            VGet(50, 5, 10.5f),
            GetColor(115, 135, 150),
            GetColor(65, 85, 100), TRUE
        );

        // 右研究室：外側の壁
        DrawCube3D(
            VGet(50, 0, -10),
            VGet(50.5f, 5, 10),
            GetColor(115, 135, 150),
            GetColor(65, 85, 100), TRUE
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