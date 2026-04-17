#pragma once
// ============================================================
//  common.h  –  Shared constants, types, math & GL helpers
// ============================================================
#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  Window / World constants
// ─────────────────────────────────────────────
const int   WINDOW_W        = 1280;
const int   WINDOW_H        = 800;
const float HALF_W          = WINDOW_W / 2.0f;
const float HALF_H          = WINDOW_H / 2.0f;

// ─────────────────────────────────────────────
//  Gameplay constants
// ─────────────────────────────────────────────
const float PLAYER_SPEED    = 200.0f;
const float BULLET_SPEED    = 450.0f;
const float ALIEN_SPEED     = 80.0f;
const float BOSS_SPEED      = 45.0f;
const float BOSS_BULLET_SPEED = 220.0f;
const float BOSS_FIRE_RATE  = 2.2f;

const float PLAYER_RADIUS   = 20.0f;
const float ALIEN_RADIUS    = 14.0f;
const float BOSS_RADIUS     = 28.0f;
const float BULLET_RADIUS   = 6.0f;
const float SHIP_SCALE      = 38.0f;

const int   BOSS_HP         = 5;
const int   BOSS_SPAWN_EVERY= 5;

// ─────────────────────────────────────────────
//  Vec2 + math helpers
// ─────────────────────────────────────────────
struct Vec2 { float x, y; };

inline float dist2(Vec2 a, Vec2 b){
    float dx=a.x-b.x, dy=a.y-b.y;
    return dx*dx + dy*dy;
}
inline bool circleCollide(Vec2 a, float ra, Vec2 b, float rb){
    float r = ra + rb;
    return dist2(a, b) < r * r;
}
inline float randRange(float lo, float hi){
    return lo + (hi - lo) * (float)rand() / RAND_MAX;
}

// ─────────────────────────────────────────────
//  Legacy GL helpers (shared by all draw code)
// ─────────────────────────────────────────────
inline void setColor(float r, float g, float b, float a = 1.f){
    glColor4f(r, g, b, a);
}

inline void drawPoly(Vec2 c, float rad, int sides, float off = 0.f){
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(c.x, c.y);
    for(int i = 0; i <= sides; i++){
        float a = off + (float)i / sides * 2.f * (float)M_PI;
        glVertex2f(c.x + cosf(a)*rad, c.y + sinf(a)*rad);
    }
    glEnd();
}

inline void drawSquare(Vec2 c, float h){
    glBegin(GL_QUADS);
    glVertex2f(c.x-h, c.y-h); glVertex2f(c.x+h, c.y-h);
    glVertex2f(c.x+h, c.y+h); glVertex2f(c.x-h, c.y+h);
    glEnd();
}

// ─────────────────────────────────────────────
//  Modern GL – shader compiler helpers
// ─────────────────────────────────────────────
GLuint compileShader(GLenum type, const char* src);
GLuint buildProgram(const char* vs, const char* fs);

// ─────────────────────────────────────────────
//  Segment font
// ─────────────────────────────────────────────
void drawChar(float px, float py, float sc, char c);
void drawString(float x, float y, float sc, const std::string& txt);
