#pragma once
// ============================================================
//  projectile.h  –  Player bullet + enemy bullet
// ============================================================
#include "common.h"

struct Projectile {
    Vec2 pos, vel;
    bool active = true;

    void update(float dt);
    void draw()  const;
};
