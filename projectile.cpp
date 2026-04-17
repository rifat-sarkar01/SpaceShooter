// ============================================================
//  projectile.cpp
// ============================================================
#include "projectile.h"

void Projectile::update(float dt){
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    if(pos.x < -HALF_W || pos.x > HALF_W ||
       pos.y < -HALF_H || pos.y > HALF_H)
        active = false;
}

void Projectile::draw() const {
    if(!active) return;
    setColor(1.f, 0.92f, 0.3f);
    drawPoly(pos, BULLET_RADIUS, 8);
    setColor(1.f, 0.55f, 0.f, 0.5f);
    drawPoly(pos, BULLET_RADIUS + 3.f, 8);
}
