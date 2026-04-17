// ============================================================
//  player.cpp
// ============================================================
#include "player.h"

void Player::update(float dt){
    float dx=0, dy=0;
    if(moveUp)    dy += 1;
    if(moveDown)  dy -= 1;
    if(moveRight) dx += 1;
    if(moveLeft)  dx -= 1;

    float len = sqrtf(dx*dx + dy*dy);
    if(len > 0){ dx /= len; dy /= len; }

    pos.x += dx * PLAYER_SPEED * dt;
    pos.y += dy * PLAYER_SPEED * dt;
    pos.x = std::max(-HALF_W + PLAYER_RADIUS,
            std::min( HALF_W - PLAYER_RADIUS, pos.x));
    pos.y = std::max(-HALF_H + PLAYER_RADIUS,
            std::min( HALF_H - PLAYER_RADIUS, pos.y));

    if(invTimer > 0) invTimer -= dt;
}

void Player::aimAt(float mx, float my){
    float dx = mx - pos.x, dy = my - pos.y;
    angle = atan2f(dy, dx) * 180.f / (float)M_PI;
}

Projectile Player::fire() const {
    Projectile p;
    float a = angle * (float)M_PI / 180.f;
    p.pos = pos;
    p.vel = {cosf(a) * BULLET_SPEED, sinf(a) * BULLET_SPEED};
    return p;
}
