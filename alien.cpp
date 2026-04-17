// ============================================================
//  alien.cpp  –  Normal alien + Boss alien  (PNG-textured)
// ============================================================
//
//  Rendering pipeline (same as renderer.cpp):
//    load PNG → GL texture → shared quad VAO → GLSL shader
//    → scale → rotate → translate → discard black bg → draw
// ============================================================

// stb_image is compiled in renderer.cpp (STB_IMAGE_IMPLEMENTATION).
// We only need the declaration header here.
#include "stb_image.h"

#include "alien.h"

// ─────────────────────────────────────────────────────────────
//  Static member definitions for AlienTextureCache
// ─────────────────────────────────────────────────────────────
GLuint AlienTextureCache::normalTexID = 0;
GLuint AlienTextureCache::bossTexID   = 0;
GLuint AlienTextureCache::prog        = 0;
GLint  AlienTextureCache::locPos      = -1;
GLint  AlienTextureCache::locAngle    = -1;
GLint  AlienTextureCache::locScaleX   = -1;
GLint  AlienTextureCache::locScaleY   = -1;
GLint  AlienTextureCache::locHW       = -1;
GLint  AlienTextureCache::locHH       = -1;
GLint  AlienTextureCache::locTex      = -1;
GLint  AlienTextureCache::locAlpha    = -1;
GLuint AlienTextureCache::vao         = 0;
GLuint AlienTextureCache::vbo         = 0;

// ─────────────────────────────────────────────────────────────
//  AlienTextureCache::loadTexture
//  Load a PNG from disk and upload it to the GPU.
//  Returns the GL texture ID, or 0 on failure.
// ─────────────────────────────────────────────────────────────
GLuint AlienTextureCache::loadTexture(const char* path) {
    // Flip Y so the image is right-side-up in OpenGL's Y-up space
    stbi_set_flip_vertically_on_load(true);

    int w = 0, h = 0, ch = 0;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4); // force RGBA

    if (!data) {
        std::cerr << "[AlienTextureCache] Cannot load: " << path
                  << "  (" << stbi_failure_reason() << ")\n"
                  << "  Place " << path << " next to the .exe\n";
        return 0;
    }

    // Generate and configure a GL texture object
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload the decoded pixel data to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);   // CPU copy no longer needed
    return id;
}

// ─────────────────────────────────────────────────────────────
//  AlienTextureCache::init
//  Load both PNGs, build the shared quad VAO, compile shader.
//  Call once after the GL context is ready.
// ─────────────────────────────────────────────────────────────
void AlienTextureCache::init() {

    // ── Load textures ─────────────────────────────────────
    normalTexID = loadTexture("normal_alien.png");
    bossTexID   = loadTexture("boss_alien.png");

    // ── Build shared unit quad (-0.5 to +0.5) ────────────
    // Layout per vertex: x, y, u, v
    float quad[] = {
        -0.5f, -0.5f,   0.f, 0.f,   // bottom-left
         0.5f, -0.5f,   1.f, 0.f,   // bottom-right
         0.5f,  0.5f,   1.f, 1.f,   // top-right
        -0.5f,  0.5f,   0.f, 1.f,   // top-left
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    // Attribute 0: position (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: UV (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // ── Compile GLSL shader ───────────────────────────────
    // Vertex shader: scale → rotate → translate → NDC
    const char* VS = R"GLSL(
#version 330 core

layout(location = 0) in vec2 aPos;  // local quad vertex (-0.5..+0.5)
layout(location = 1) in vec2 aUV;   // texture coordinate (0..1)

uniform vec2  uPos;     // world-space alien centre
uniform float uAngle;   // rotation angle in radians
uniform float uScaleX;  // display width  in world pixels
uniform float uScaleY;  // display height in world pixels
uniform float uHalfW;   // ortho half-width  (HALF_W)
uniform float uHalfH;   // ortho half-height (HALF_H)

out vec2 vUV;

void main() {
    // Step 1: resize unit quad to the desired pixel footprint
    vec2 p = vec2(aPos.x * uScaleX, aPos.y * uScaleY);

    // Step 2: rotate around the alien's local origin
    float c = cos(uAngle), s = sin(uAngle);
    vec2 r = vec2(c*p.x - s*p.y,
                  s*p.x + c*p.y);

    // Step 3: translate to world position, then convert to NDC
    vec2 w = r + uPos;
    gl_Position = vec4(w.x / uHalfW, w.y / uHalfH, 0.0, 1.0);

    vUV = aUV;
}
)GLSL";

    // Fragment shader: sample texture, discard black background
    const char* FS = R"GLSL(
#version 330 core

in  vec2 vUV;

uniform sampler2D uTex;   // alien sprite (unit 0)
uniform float     uAlpha; // overall opacity

out vec4 FragColor;

void main() {
    vec4 col = texture(uTex, vUV);

    // Discard near-black pixels to remove the PNG background.
    // Perceptual luminance weights: R=0.299, G=0.587, B=0.114
    float lum = dot(col.rgb, vec3(0.299, 0.587, 0.114));
    if (lum < 0.04) discard;

    float a = col.a * uAlpha;
    if (a < 0.01) discard;

    FragColor = vec4(col.rgb, a);
}
)GLSL";

    prog = buildProgram(VS, FS);

    // Cache uniform locations to avoid per-frame string lookups
    locPos    = glGetUniformLocation(prog, "uPos");
    locAngle  = glGetUniformLocation(prog, "uAngle");
    locScaleX = glGetUniformLocation(prog, "uScaleX");
    locScaleY = glGetUniformLocation(prog, "uScaleY");
    locHW     = glGetUniformLocation(prog, "uHalfW");
    locHH     = glGetUniformLocation(prog, "uHalfH");
    locTex    = glGetUniformLocation(prog, "uTex");
    locAlpha  = glGetUniformLocation(prog, "uAlpha");
}

// ─────────────────────────────────────────────────────────────
//  AlienTextureCache::cleanup
// ─────────────────────────────────────────────────────────────
void AlienTextureCache::cleanup() {
    if (normalTexID){ glDeleteTextures(1, &normalTexID); normalTexID = 0; }
    if (bossTexID)  { glDeleteTextures(1, &bossTexID);   bossTexID   = 0; }
    if (vao){ glDeleteVertexArrays(1, &vao); vao = 0; }
    if (vbo){ glDeleteBuffers(1,      &vbo); vbo = 0; }
    if (prog){ glDeleteProgram(prog);        prog = 0; }
}

// ─────────────────────────────────────────────────────────────
//  Factory methods
// ─────────────────────────────────────────────────────────────

Alien Alien::makeNormal() {
    Alien a;
    a.type = AlienType::NORMAL;
    a.hp   = 1;

    // Spawn on a random screen edge (outside the visible area)
    int edge = rand() % 4;
    switch(edge) {
        // Top edge: random X, just above the screen
        case 0: a.pos = {randRange(-HALF_W, HALF_W),  HALF_H + 20}; break;
        // Bottom edge
        case 1: a.pos = {randRange(-HALF_W, HALF_W), -HALF_H - 20}; break;
        // Right edge
        case 2: a.pos = { HALF_W + 20, randRange(-HALF_H, HALF_H)}; break;
        // Left edge
        case 3: a.pos = {-HALF_W - 20, randRange(-HALF_H, HALF_H)}; break;
    }

    // Head toward a random point near the screen centre
    float dx = randRange(-HALF_W * 0.4f, HALF_W * 0.4f) - a.pos.x;
    float dy = randRange(-HALF_H * 0.4f, HALF_H * 0.4f) - a.pos.y;
    float len = sqrtf(dx*dx + dy*dy);
    a.vel = {dx / len * ALIEN_SPEED, dy / len * ALIEN_SPEED};
    return a;
}

Alien Alien::makeBoss() {
    Alien a;
    a.type = AlienType::BOSS;
    a.hp   = BOSS_HP;

    // Spawn on a random edge, further out than normal aliens
    int edge = rand() % 4;
    switch(edge) {
        case 0: a.pos = {randRange(-HALF_W, HALF_W),  HALF_H + 40}; break;
        case 1: a.pos = {randRange(-HALF_W, HALF_W), -HALF_H - 40}; break;
        case 2: a.pos = { HALF_W + 40, randRange(-HALF_H, HALF_H)}; break;
        case 3: a.pos = {-HALF_W - 40, randRange(-HALF_H, HALF_H)}; break;
    }

    // Head straight for the screen centre
    float dx = -a.pos.x, dy = -a.pos.y;
    float len = sqrtf(dx*dx + dy*dy);
    a.vel = {dx / len * BOSS_SPEED, dy / len * BOSS_SPEED};
    return a;
}

// ─────────────────────────────────────────────────────────────
//  Per-frame logic
// ─────────────────────────────────────────────────────────────

void Alien::update(float dt) {
    if (!active) return;

    // Move in a straight line each frame (velocity × delta-time)
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    // Slowly spin the sprite for visual interest
    angle += 40.f * dt;

    // Boss: count down the shoot cooldown every frame
    if (type == AlienType::BOSS)
        shootTimer -= dt;

    // Deactivate if the alien has drifted far off-screen
    // (60px buffer avoids a visible pop-out at the edge)
    if (pos.x < -HALF_W - 60 || pos.x > HALF_W + 60 ||
        pos.y < -HALF_H - 60 || pos.y > HALF_H + 60)
        active = false;
}

void Alien::faceTarget(Vec2 target) {
    // Compute the unit vector from this alien toward the target.
    // Used by the boss to rotate toward the player each frame.
    float dx  = target.x - pos.x;
    float dy  = target.y - pos.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len > 0.1f)
        facingDir = {dx / len, dy / len};
}

Projectile Alien::fireAt(Vec2 target) const {
    // Create a projectile moving from this alien toward the target.
    Projectile p;
    float dx  = target.x - pos.x;
    float dy  = target.y - pos.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 0.1f) { p.active = false; return p; }
    p.pos = pos;
    p.vel = {dx / len * BOSS_BULLET_SPEED,
             dy / len * BOSS_BULLET_SPEED};
    return p;
}

float Alien::radius() const {
    // Collision circle radius: boss is twice the size of a normal alien
    return (type == AlienType::NORMAL) ? ALIEN_RADIUS : BOSS_RADIUS;
}

// ─────────────────────────────────────────────────────────────
//  Rendering
// ─────────────────────────────────────────────────────────────

void Alien::draw() const {
    if (!active) return;
    // Dispatch to the correct draw helper based on type
    if (type == AlienType::NORMAL) drawNormal();
    else                           drawBoss();
}

void Alien::drawNormal() const {
    // Normal alien: fixed display size (56 px), slow rotation.
    // The sprite rotates to face the direction of travel using
    // atan2 on the velocity vector, with -90° to align the
    // PNG nose (which points upward) with the +X movement reference.
    float facingAngle = atan2f(vel.y, vel.x) - (float)M_PI / 2.f;
    drawSprite(AlienTextureCache::normalTexID, 56.f, facingAngle);
}

void Alien::drawBoss() const {
    // Boss alien: larger display size (90 px), faces the player.
    // facingDir is updated each frame by Game::update → faceTarget().
    float facingAngle = atan2f(facingDir.y, facingDir.x)
                        - (float)M_PI / 2.f;
    drawSprite(AlienTextureCache::bossTexID, 90.f, facingAngle);

    // Draw the HP bar on top of the sprite
    drawHPBar();
}

// ─────────────────────────────────────────────────────────────
//  drawSprite
//  Internal helper – render one textured quad using the shared
//  AlienTextureCache shader program.
// ─────────────────────────────────────────────────────────────
void Alien::drawSprite(GLuint texID, float displaySize,
                        float angleRad, float alpha) const {
    // Guard: skip if the shared shader or this texture failed to load
    if (!AlienTextureCache::prog || !texID) return;

    glUseProgram(AlienTextureCache::prog);

    // Upload uniforms for this particular alien instance
    glUniform2f(AlienTextureCache::locPos,    pos.x, pos.y);
    glUniform1f(AlienTextureCache::locAngle,  angleRad);
    glUniform1f(AlienTextureCache::locScaleX, displaySize);  // square sprite
    glUniform1f(AlienTextureCache::locScaleY, displaySize);
    glUniform1f(AlienTextureCache::locHW,     HALF_W);
    glUniform1f(AlienTextureCache::locHH,     HALF_H);
    glUniform1f(AlienTextureCache::locAlpha,  alpha);
    glUniform1i(AlienTextureCache::locTex,    0);       // texture unit 0

    // Bind this alien's PNG texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Draw the 4-vertex quad (2 triangles) from the shared VAO
    glBindVertexArray(AlienTextureCache::vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

// ─────────────────────────────────────────────────────────────
//  drawHPBar
//  Drawn with legacy GL on top of the boss sprite.
//  Bar is positioned ABOVE the alien based on BOSS_RADIUS.
// ─────────────────────────────────────────────────────────────
void Alien::drawHPBar() const {
    float dmg = (float)hp / (float)BOSS_HP;  // 1.0 = full, 0.0 = dead

    // Bar dimensions and position
    float bw = BOSS_RADIUS * 3.5f;  // bar width scales with boss size
    float bh = 6.f;                 // bar height in pixels
    float bx = pos.x - bw / 2.f;   // bar left edge
    float by = pos.y + BOSS_RADIUS + 10.f;  // above the sprite

    // Dark grey background (empty HP)
    setColor(0.15f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(bx,      by);
    glVertex2f(bx + bw, by);
    glVertex2f(bx + bw, by + bh);
    glVertex2f(bx,      by + bh);
    glEnd();

    // HP fill: shifts from green → yellow → red as damage increases
    // dmg = 1.0 → green (0, 1, 0)
    // dmg = 0.5 → yellow (1, 1, 0)
    // dmg = 0.0 → red  (1, 0, 0)
    setColor(1.f - dmg, dmg, 0.f);
    glBegin(GL_QUADS);
    glVertex2f(bx,              by);
    glVertex2f(bx + bw * dmg,   by);
    glVertex2f(bx + bw * dmg,   by + bh);
    glVertex2f(bx,              by + bh);
    glEnd();

    // Thin cyan border around the bar
    setColor(0.f, 0.8f, 0.75f);
    glLineWidth(1.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx,      by);
    glVertex2f(bx + bw, by);
    glVertex2f(bx + bw, by + bh);
    glVertex2f(bx,      by + bh);
    glEnd();
    glLineWidth(1.f);
}
