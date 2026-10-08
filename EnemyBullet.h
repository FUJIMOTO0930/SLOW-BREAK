#pragma once

#include "DxLib.h"

const int TRAIL_COUNT = 12;

struct EnemyBullet
{
    VECTOR position;
    VECTOR direction;
    VECTOR trailPositions[TRAIL_COUNT];

    float speed;
    bool active;
};