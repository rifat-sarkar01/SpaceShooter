#pragma once
// ============================================================
//  renderer.h  –  PNG-textured Rage-Fang player ship
// ============================================================
//
//  SETUP: stb_image.h must be in the same folder.
//  Install via MSYS2:
//    pacman -S mingw-w64-x86_64-stb
//    cp /c/msys64/mingw64/include/stb/stb_image.h .
//
//  Place "ship.png" next to the compiled .exe before running.
// ============================================================
#include "common.h"

class RageFangRenderer {

    // ── OpenGL texture handle ─────────────────────────────
    GLuint texID  = 0;   // GPU texture object
    int    texW   = 0;   // original PNG pixel width
    int    texH   = 0;   // original PNG pixel height

    // ── Modern GL objects ────────────────────────────────
    GLuint vao    = 0;   // Vertex Array Object  – stores attribute layout
    GLuint vbo    = 0;   // Vertex Buffer Object – quad vertices + UVs
    GLuint prog   = 0;   // compiled+linked GLSL program

    // ── Shader uniform locations (cached at init time) ───
    GLint  locPos;       // vec2  – world-space ship centre
    GLint  locAngle;     // float – rotation angle in radians
    GLint  locScaleX;    // float – world-pixel width  of the quad
    GLint  locScaleY;    // float – world-pixel height of the quad
    GLint  locHW;        // float – HALF_W (for NDC conversion)
    GLint  locHH;        // float – HALF_H (for NDC conversion)
    GLint  locTex;       // sampler2D – texture unit 0
    GLint  locAlpha;     // float – overall opacity (used for blink)

public:
    // ── Lifecycle ─────────────────────────────────────────
    // Load the PNG, upload to GPU, build the quad VAO.
    // Call once after the OpenGL context is created.
    void init(const char* pngPath = "ship.png");

    // Draw the textured quad at (pos) rotated by angleDeg.
    // invTimer > 0 causes the ship to blink (invincibility flash).
    void draw(Vec2 pos, float angleDeg, float invTimer, double t);

    // Free all GPU resources.
    void cleanup();
};
