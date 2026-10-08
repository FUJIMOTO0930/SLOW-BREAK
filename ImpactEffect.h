#pragma once

#include "DxLib.h"

struct ImpactEffect
{
    VECTOR position;
    VECTOR sparkPosition[6];
    VECTOR sparkDirection[6];

    int timer;
    bool active;
};