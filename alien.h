#pragma once
// ============================================================
//  alien.h  –  Normal alien + Boss alien
// ============================================================
//
//  Both alien types now render as PNG textures instead of
//  procedural geometry.
//
//  Required files next to the .exe:
//    normal_alien.png  – sprite for normal aliens
//    boss_alien.png    – sprite for the boss
//
//  The AlienTextureCache (below) loads each PNG exactly once
//  and shares the GL texture ID across all alien instances.
// ============================================================
#include "common.h"
#include "projectile.h"

// ─────────────────────────────────────────────────────────────
//  AlienTextureCache
//  Holds one GL texture per alien type.
//  Call AlienTextureCache::init() once after GL context is ready.
// ─────────────────────────────────────────────────────────────
struct AlienTextureCache {

    // ── Texture handles ───────────────────────────────────
    static GLuint normalTexID;   // GL handle for normal_alien.png
    static GLuint bossTexID;     // GL handle for boss_alien.png

    // ── Shared GLSL program ───────────────────────────────
    // Both alien types use the same vertex+fragment shader pair.
    static GLuint prog;

    // ── Cached uniform locations ──────────────────────────
    static GLint  locPos;        // vec2  – world position
    static GLint  locAngle;      // float – rotation (radians)
    static GLint  locScaleX;     // float – display width  (px)
    static GLint  locScaleY;     // float – display height (px)
    static GLint  locHW;         // float – HALF_W
    static GLint  locHH;         // float – HALF_H
    static GLint  locTex;        // sampler2D – texture unit 0
    static GLint  locAlpha;      // float – opacity

    // ── Shared quad geometry ──────────────────────────────
    static GLuint vao;           // one quad VAO shared by all aliens
    static GLuint vbo;

    // ── Lifecycle ─────────────────────────────────────────
    // Load both PNGs and compile the shared shader program.
    static void init();

    // Free all GL resources (call on shutdown).
    static void cleanup();

private:
    // Helper: load a PNG file and upload it as a GL texture.
    // Returns the new texture ID, or 0 on failure.
    static GLuint loadTexture(const char* path);
};

// ─────────────────────────────────────────────────────────────
//  AlienType
// ─────────────────────────────────────────────────────────────
enum class AlienType { NORMAL, BOSS };

// ─────────────────────────────────────────────────────────────
//  Alien
// ─────────────────────────────────────────────────────────────
struct Alien {

    // ── State ─────────────────────────────────────────────
    Vec2      pos;             // world-space centre position
    Vec2      vel;             // velocity in world pixels/second
    int       hp;              // remaining hit points
    AlienType type;            // NORMAL (1 HP) or BOSS (5 HP)
    bool      active = true;   // false = marked for removal

    // Rotation angle used by the boss's slow spin
    float     angle      = 0.f;

    // Boss-only: seconds until the next shot (counts down each frame)
    float     shootTimer = 0.f;

    // Boss-only: normalised unit vector pointing toward the player.
    // Updated every frame in faceTarget(); used both for rotation
    // and for aiming fireAt().
    Vec2      facingDir  = {0.f, -1.f};

    // ── Factory methods ───────────────────────────────────
    // Spawn a normal alien on a random screen edge
    static Alien makeNormal();

    // Spawn a boss alien on a random screen edge
    static Alien makeBoss();

    // ── Per-frame logic ───────────────────────────────────

    // Move the alien, update timers, deactivate if out of bounds
    void       update(float dt);

    // (Boss only) Recalculate facingDir toward 'target'
    void       faceTarget(Vec2 target);

    // (Boss only) Create a projectile aimed at 'target'
    Projectile fireAt(Vec2 target) const;

    // Return the collision circle radius for this alien type
    float      radius() const;

    // ── Rendering ─────────────────────────────────────────

    // Dispatches to drawNormal() or drawBoss()
    void draw() const;

private:
    // Draw the normal alien sprite + HP-bar (if damaged)
    void drawNormal() const;

    // Draw the boss alien sprite + rotating facing + HP bar
    void drawBoss()   const;

    // Internal helper: draw a textured quad at (pos) using the
    // shared AlienTextureCache shader.  displaySize controls how
    // large the sprite appears in world pixels.
    void drawSprite(GLuint texID, float displaySize,
                    float angleRad, float alpha = 1.f) const;

    // Draw the HP bar above the alien (boss only)
    void drawHPBar() const;
};
