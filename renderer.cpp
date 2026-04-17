// ============================================================
//  renderer.cpp  –  PNG-textured Rage-Fang player ship
// ============================================================
//
//  Pipeline overview:
//    1. stb_image loads the PNG from disk into CPU memory (RGBA).
//    2. glTexImage2D uploads the pixels to a GPU texture object.
//    3. A unit quad (-0.5..+0.5) is stored in a VAO/VBO.
//    4. Each frame the vertex shader:
//         a) scales the quad to the desired pixel size,
//         b) rotates it around the ship's centre using a 2-D
//            rotation matrix built from cos/sin of angleDeg,
//         c) translates to world position,
//         d) divides by HALF_W/HALF_H to produce NDC coords.
//    5. The fragment shader discards near-black pixels so the
//       PNG's black background becomes transparent.
// ============================================================

// Tell stb_image to compile its implementation in THIS .cpp file.
// Only define this in ONE translation unit.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "renderer.h"

// ─────────────────────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────────────────────
void RageFangRenderer::init(const char* pngPath) {

    // ── Step 1: load PNG from disk ────────────────────────
    // stbi_set_flip_vertically_on_load(true) because PNG stores
    // rows from top-to-bottom, but OpenGL expects bottom-to-top.
    stbi_set_flip_vertically_on_load(true);
    int channels = 0;
    unsigned char* data = stbi_load(
        pngPath,   // file path
        &texW,     // filled with image width  in pixels
        &texH,     // filled with image height in pixels
        &channels, // filled with original channel count
        4          // force 4 channels: R G B A
    );

    if (!data) {
        // Non-fatal: the game still runs, the ship just won't render.
        std::cerr << "[RageFangRenderer] Failed to load: " << pngPath
                  << "\n  Reason: " << stbi_failure_reason()
                  << "\n  Fix: place ship.png next to space_shooter.exe\n";
        return;
    }

    // ── Step 2: create and configure a GL texture object ──
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Use linear filtering so the ship looks smooth when scaled
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clamp to edge: pixels outside [0,1] UV use the border colour,
    // not a tiled repeat (prevents edge artefacts on the quad border)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload pixel data to GPU; GL_RGBA = 4 bytes per pixel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texW, texH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);  // CPU copy is no longer needed

    // ── Step 3: build a unit quad in a VAO/VBO ────────────
    // The quad occupies local space -0.5 to +0.5 on both axes.
    // Each vertex stores:  x, y,  u, v
    //   (x,y) = local position
    //   (u,v) = texture coordinate (0=left/bottom, 1=right/top)
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

    // Attribute 0: position (x, y) – stride = 4 floats, offset = 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: UV (u, v) – stride = 4 floats, offset = 2 floats
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // ── Step 4: compile GLSL shaders ──────────────────────

    // Vertex shader: scale → rotate → translate → NDC
    const char* VS = R"GLSL(
#version 330 core

layout(location = 0) in vec2 aPos;   // local quad vertex  (-0.5..+0.5)
layout(location = 1) in vec2 aUV;    // texture coordinate (0..1)

// Ship transform uniforms
uniform vec2  uPos;       // world-space centre of the ship
uniform float uAngle;     // rotation angle in radians (from atan2)
uniform float uScaleX;    // desired display width  in world pixels
uniform float uScaleY;    // desired display height in world pixels

// Projection uniforms (convert world pixels to NDC)
uniform float uHalfW;     // HALF_W = 640 for 1280-wide window
uniform float uHalfH;     // HALF_H = 400 for  800-high window

out vec2 vUV;             // pass UV to fragment shader

void main() {
    // 1. Scale the unit quad to the desired world-pixel size
    vec2 p = vec2(aPos.x * uScaleX,
                  aPos.y * uScaleY);

    // 2. Rotate around the ship's local origin
    //    Standard 2-D rotation matrix:
    //      | cos  -sin |   | x |
    //      | sin   cos | * | y |
    float c = cos(uAngle);
    float s = sin(uAngle);
    vec2 r = vec2(c * p.x - s * p.y,
                  s * p.x + c * p.y);

    // 3. Translate to world position
    vec2 w = r + uPos;

    // 4. Orthographic projection: world pixels → NDC [-1, +1]
    //    (replaces glOrtho for the modern-GL path)
    gl_Position = vec4(w.x / uHalfW,
                       w.y / uHalfH,
                       0.0, 1.0);

    vUV = aUV;
}
)GLSL";

    // Fragment shader: sample texture, discard black background
    const char* FS = R"GLSL(
#version 330 core

in  vec2 vUV;

uniform sampler2D uTex;    // the ship PNG  (bound to unit 0)
uniform float     uAlpha;  // global opacity (1.0 normal, 0.0 invisible)

out vec4 FragColor;

void main() {
    vec4 col = texture(uTex, vUV);

    // ── Background removal ───────────────────────────────
    // The PNG has a black background.  Compute perceptual
    // luminance (human eyes are most sensitive to green).
    float lum = dot(col.rgb, vec3(0.299, 0.587, 0.114));

    // Any pixel darker than 4% brightness is treated as background
    // and thrown away so the starfield shows through.
    if (lum < 0.04) discard;

    // Also respect the PNG's own alpha channel (if present)
    float a = col.a * uAlpha;
    if (a < 0.01) discard;

    FragColor = vec4(col.rgb, a);
}
)GLSL";

    // Link vertex + fragment shaders into a program
    prog = buildProgram(VS, FS);

    // Cache all uniform locations so we avoid string lookups each frame
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
//  draw  –  render the textured quad for this frame
// ─────────────────────────────────────────────────────────────
void RageFangRenderer::draw(Vec2 pos, float angleDeg,
                             float invTimer, double /*t*/) {

    // Safety: skip if init() failed (missing PNG or GL error)
    if (!prog || !texID) return;

    // ── Invincibility blink ───────────────────────────────
    // Toggle visibility 8 times per second while invTimer > 0.
    // ( (int)(invTimer * 8) % 2 ) alternates 0 / 1 at 8 Hz.
    if (invTimer > 0.f && (int)(invTimer * 8) % 2 == 0) return;

    // ── Rotation angle ────────────────────────────────────
    // angleDeg comes from atan2(mouseY - shipY, mouseX - shipX),
    // giving the angle of the vector from ship to cursor.
    // The PNG nose points to the RIGHT (+X direction in image space),
    // which matches atan2's reference axis, so no offset is needed.
    float angle = angleDeg * (float)M_PI / 180.f;

    // ── Display size ──────────────────────────────────────
    // Scale so the ship is 120 world-pixels wide.
    // Height is derived from the PNG's aspect ratio to avoid stretch.
    float dispW = 120.f;
    float dispH = (texH > 0)
                  ? dispW * (float)texH / (float)texW
                  : 120.f;

    // ── Upload uniforms and draw ──────────────────────────
    glUseProgram(prog);

    glUniform2f(locPos,    pos.x, pos.y);   // world position
    glUniform1f(locAngle,  angle);           // rotation (radians)
    glUniform1f(locScaleX, dispW);           // quad width  in px
    glUniform1f(locScaleY, dispH);           // quad height in px
    glUniform1f(locHW,     HALF_W);          // ortho half-width
    glUniform1f(locHH,     HALF_H);          // ortho half-height
    glUniform1f(locAlpha,  1.0f);            // fully opaque
    glUniform1i(locTex,    0);               // texture unit 0

    // Bind the ship texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Draw the 4-vertex quad as a triangle fan (2 triangles)
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    // Clean up state (good practice to unbind after use)
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

// ─────────────────────────────────────────────────────────────
//  cleanup  –  release all GPU resources
// ─────────────────────────────────────────────────────────────
void RageFangRenderer::cleanup() {
    if (texID) { glDeleteTextures(1,     &texID); texID = 0; }
    if (vao)   { glDeleteVertexArrays(1, &vao);   vao   = 0; }
    if (vbo)   { glDeleteBuffers(1,      &vbo);   vbo   = 0; }
    if (prog)  { glDeleteProgram(prog);            prog  = 0; }
}
