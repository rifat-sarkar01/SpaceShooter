#pragma once
// ============================================================
//  player.h  –  Player ship data and logic
// ============================================================
#include "common.h"
#include "projectile.h"

struct Player {
    Vec2  pos       = {0, 0};
    float angle     = 90.f;
    int   lives     = 3;
    float invTimer  = 0.f;

    bool moveUp    = false;
    bool moveDown  = false;
    bool moveLeft  = false;
    bool moveRight = false;

    void       update(float dt);
    void       aimAt(float mx, float my);
    Projectile fire() const;
    bool       isAlive() const { return lives > 0; }
};
