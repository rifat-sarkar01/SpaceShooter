#pragma once
// ============================================================
//  game.h  –  Main game loop, state, and GLFW callbacks
// ============================================================
#include "common.h"
#include "projectile.h"
#include "alien.h"
#include "player.h"
#include "renderer.h"

struct Game {
    GLFWwindow*              window       = nullptr;
    Player                   player;
    std::vector<Alien>       aliens;
    std::vector<Projectile>  bullets;
    std::vector<Projectile>  enemyBullets;
    RageFangRenderer         ship;

    int    score         = 0;
    int    normalKills   = 0;
    float  spawnTimer    = 0.f;
    float  spawnInterval = 2.5f;
    bool   gameOver      = false;
    float  mouseX        = 0.f;
    float  mouseY        = 0.f;
    double gameTime      = 0.0;

    // ── Lifecycle ─────────────────────────────────────────
    bool init();
    void run();
    void restart();

    // ── Per-frame ─────────────────────────────────────────
    void update(float dt);
    void render();
    void renderGameOver();

    // ── Helpers ───────────────────────────────────────────
    void setupProjection();
    void drawStarfield();
    void drawHUD();

    // ── GLFW static callbacks ──────────────────────────────
    static void keyCallback(GLFWwindow* w, int key, int scan,
                            int action, int mods);
    static void mouseButtonCallback(GLFWwindow* w, int button,
                                    int action, int mods);
    static void cursorCallback(GLFWwindow* w, double xpos,
                               double ypos);
};
