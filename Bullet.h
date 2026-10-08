#pragma once

#include "DxLib.h"

struct Bullet
{
    VECTOR position;   // ’e‚ÌˆÊ’u
    VECTOR direction;  // ’e‚ª”ò‚Ô•ûŒü

    float speed;       // ’e‚Ì‘¬“x

    bool active;       // ’e‚ª‘¶İ‚µ‚Ä‚¢‚é‚©
};