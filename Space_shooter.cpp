// ============================================================
//  2D Space Shooter – Rage-Fang Edition
//  C++ / GLFW / GLEW / OpenGL 3.3 compatibility
//
//  Compile (MSYS2 MINGW64):
//    /c/msys64/mingw64/bin/g++ Space_shooter.cpp -o space_shooter.exe \
//      -lglew32 -lglfw3 -lopengl32 -lgdi32 -mconsole -std=c++17
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
//  Constants
// ─────────────────────────────────────────────
const int   WINDOW_W   = 1280;
const int   WINDOW_H   = 800;
const float HALF_W     = WINDOW_W / 2.0f;
const float HALF_H     = WINDOW_H / 2.0f;

const float PLAYER_SPEED     = 200.0f;
const float BULLET_SPEED     = 450.0f;
const float ALIEN_SPEED      = 80.0f;
const float BOSS_SPEED       = 45.0f;
const float BOSS_BULLET_SPEED = 220.0f;
const float BOSS_FIRE_RATE    = 2.2f;   // seconds between shots

const float PLAYER_RADIUS    = 16.0f;
const float ALIEN_RADIUS     = 14.0f;
const float BOSS_RADIUS      = 28.0f;
const float BULLET_RADIUS    = 6.0f;
const float SHIP_SCALE       = 38.0f;   // pixels per local unit

const int   BOSS_HP          = 5;
const int   BOSS_SPAWN_EVERY = 5;

// ─────────────────────────────────────────────
//  Math helpers
// ─────────────────────────────────────────────
struct Vec2 { float x, y; };

inline float dist2(Vec2 a, Vec2 b) {
    float dx=a.x-b.x, dy=a.y-b.y; return dx*dx+dy*dy;
}
inline bool circleCollide(Vec2 a,float ra,Vec2 b,float rb){
    float r=ra+rb; return dist2(a,b)<r*r;
}
inline float randRange(float lo,float hi){
    return lo+(hi-lo)*(float)rand()/RAND_MAX;
}

// ─────────────────────────────────────────────
//  Legacy GL helpers  (HUD / aliens / bullets)
// ─────────────────────────────────────────────
void setColor(float r,float g,float b,float a=1.f){glColor4f(r,g,b,a);}

void drawPoly(Vec2 c,float rad,int sides,float off=0){
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(c.x,c.y);
    for(int i=0;i<=sides;i++){
        float a=off+(float)i/sides*2.f*(float)M_PI;
        glVertex2f(c.x+cosf(a)*rad,c.y+sinf(a)*rad);
    }
    glEnd();
}
void drawSquare(Vec2 c,float h){
    glBegin(GL_QUADS);
    glVertex2f(c.x-h,c.y-h); glVertex2f(c.x+h,c.y-h);
    glVertex2f(c.x+h,c.y+h); glVertex2f(c.x-h,c.y+h);
    glEnd();
}

// ─────────────────────────────────────────────
//  Segment font  (7-segment style, Y-down glyph space)
// ─────────────────────────────────────────────
struct Seg{int x1,y1,x2,y2;};
void drawChar(float px,float py,float sc,char c){
    std::vector<Seg> s;
    switch(toupper(c)){
        case '0':s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0}};break;
        case '1':s={{2,0,2,6}};break;
        case '2':s={{0,0,4,0},{4,0,4,3},{4,3,0,3},{0,3,0,6},{0,6,4,6}};break;
        case '3':s={{0,0,4,0},{4,0,4,6},{0,6,4,6},{0,3,4,3}};break;
        case '4':s={{0,0,0,3},{0,3,4,3},{4,0,4,6}};break;
        case '5':s={{4,0,0,0},{0,0,0,3},{0,3,4,3},{4,3,4,6},{4,6,0,6}};break;
        case '6':s={{4,0,0,0},{0,0,0,6},{0,6,4,6},{4,6,4,3},{4,3,0,3}};break;
        case '7':s={{0,0,4,0},{4,0,4,6}};break;
        case '8':s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0},{0,3,4,3}};break;
        case '9':s={{0,3,4,3},{4,3,4,0},{4,0,0,0},{0,0,0,3},{4,3,4,6}};break;
        case 'R':s={{0,0,0,6},{0,0,4,0},{4,0,4,3},{4,3,0,3},{0,3,4,6}};break;
        case 'F':s={{0,0,0,6},{0,0,4,0},{0,3,3,3}};break;
        case '-':s={{1,3,3,3}};break;
        case 'L':s={{0,0,0,6},{0,6,4,6}};break;
        case 'E':s={{4,0,0,0},{0,0,0,6},{0,6,4,6},{0,3,3,3}};break;
        case 'S':s={{4,0,0,0},{0,0,0,3},{0,3,4,3},{4,3,4,6},{4,6,0,6}};break;
        case 'G':s={{4,0,0,0},{0,0,0,6},{0,6,4,6},{4,6,4,3},{2,3,4,3}};break;
        case 'A':s={{0,6,2,0},{2,0,4,6},{1,3,3,3}};break;
        case 'M':s={{0,6,0,0},{0,0,2,3},{2,3,4,0},{4,0,4,6}};break;
        case 'O':s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0}};break;
        case 'V':s={{0,0,2,6},{2,6,4,0}};break;
        case 'P':s={{0,0,0,6},{0,0,4,0},{4,0,4,3},{4,3,0,3}};break;
        case 'K':s={{0,0,0,6},{0,3,4,0},{0,3,4,6}};break;
        case 'X':s={{0,0,4,6},{4,0,0,6}};break;
        case 'I':s={{1,0,3,0},{2,0,2,6},{1,6,3,6}};break;
        case ':':s={{2,1,2,2},{2,4,2,5}};break;
        case 'C':s={{4,0,0,0},{0,0,0,6},{0,6,4,6}};break;
        case 'T':s={{0,0,4,0},{2,0,2,6}};break;
        case 'N':s={{0,6,0,0},{0,0,4,6},{4,6,4,0}};break;
        case 'B':s={{0,0,0,6},{0,6,3,6},{3,6,4,5},{4,5,3,3},{3,3,0,3},{3,3,4,2},{4,2,3,0},{3,0,0,0}};break;
        case 'D':s={{0,0,0,6},{0,6,3,6},{3,6,4,5},{4,5,4,1},{4,1,3,0},{3,0,0,0}};break;
        case 'H':s={{0,0,0,6},{4,0,4,6},{0,3,4,3}};break;
        case 'U':s={{0,0,0,6},{0,6,4,6},{4,6,4,0}};break;
        case 'W':s={{0,0,1,6},{1,6,2,3},{2,3,3,6},{3,6,4,0}};break;
        case 'Y':s={{0,0,2,3},{4,0,2,3},{2,3,2,6}};break;
        case 'Z':s={{0,0,4,0},{4,0,0,6},{0,6,4,6}};break;
        case 'J':s={{4,0,4,6},{4,6,0,6},{0,6,0,4}};break;
        case ' ':break;
        default:break;
    }
    glLineWidth(2.f);
    glBegin(GL_LINES);
    for(auto& g:s){
        glVertex2f(px+g.x1*sc,py+g.y1*sc);
        glVertex2f(px+g.x2*sc,py+g.y2*sc);
    }
    glEnd();
    glLineWidth(1.f);
}
void drawString(float x,float y,float sc,const std::string& txt){
    float cx=x;
    for(char c:txt){drawChar(cx,y,sc,c);cx+=6.f*sc;}
}

// ─────────────────────────────────────────────
//  Modern GL – shader compiler
// ─────────────────────────────────────────────
static GLuint compileShader(GLenum type,const char* src){
    GLuint s=glCreateShader(type);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){char b[512];glGetShaderInfoLog(s,512,nullptr,b);std::cerr<<b<<"\n";}
    return s;
}
static GLuint buildProgram(const char* vs,const char* fs){
    GLuint v=compileShader(GL_VERTEX_SHADER,vs);
    GLuint f=compileShader(GL_FRAGMENT_SHADER,fs);
    GLuint p=glCreateProgram();
    glAttachShader(p,v);glAttachShader(p,f);
    glLinkProgram(p);
    glDeleteShader(v);glDeleteShader(f);
    return p;
}

// ─────────────────────────────────────────────
//  Rage-Fang Ship Geometry
//  Local space: nose = +Y, tail = -Y
//  1 unit ≈ SHIP_SCALE pixels in world space
// ─────────────────────────────────────────────
using Poly=std::vector<float>;   // flat {x0,y0, x1,y1, ...}

// Triangulate convex polygon as fan from vertex 0
static std::vector<float> triPoly(const Poly& p){
    std::vector<float> out;
    int n=(int)p.size()/2;
    for(int i=1;i<n-1;i++){
        out.push_back(p[0]);     out.push_back(p[1]);
        out.push_back(p[i*2]);   out.push_back(p[i*2+1]);
        out.push_back(p[i*2+2]); out.push_back(p[i*2+3]);
    }
    return out;
}

// ── Main fuselage ──────────────────────────────────────────
static const Poly FUSE={
     0.00f, 1.00f,
     0.09f, 0.88f,  0.17f, 0.68f,  0.22f, 0.38f,
     0.25f, 0.05f,  0.23f,-0.28f,  0.19f,-0.58f,
     0.13f,-0.80f,  0.07f,-0.98f,
    -0.07f,-0.98f, -0.13f,-0.80f, -0.19f,-0.58f,
    -0.23f,-0.28f, -0.25f, 0.05f, -0.22f, 0.38f,
    -0.17f, 0.68f, -0.09f, 0.88f,
};
// ── Wings ─────────────────────────────────────────────────
static const Poly L_WING={
    -0.22f, 0.28f, -0.76f,-0.20f, -0.83f,-0.47f, -0.24f,-0.54f,
};
static const Poly R_WING={
     0.22f, 0.28f,  0.76f,-0.20f,  0.83f,-0.47f,  0.24f,-0.54f,
};
// ── Orange leading edges ───────────────────────────────────
static const Poly L_EDGE={
    -0.22f, 0.28f, -0.76f,-0.20f, -0.78f,-0.30f, -0.24f, 0.18f,
};
static const Poly R_EDGE={
     0.22f, 0.28f,  0.76f,-0.20f,  0.78f,-0.30f,  0.24f, 0.18f,
};
// ── Wing tip fins ─────────────────────────────────────────
static const Poly L_FIN={
    -0.74f,-0.20f,-0.84f,-0.30f,-0.87f,-0.53f,-0.81f,-0.59f,-0.75f,-0.47f,
};
static const Poly R_FIN={
     0.74f,-0.20f, 0.84f,-0.30f, 0.87f,-0.53f, 0.81f,-0.59f, 0.75f,-0.47f,
};
// ── Wing detail panels ────────────────────────────────────
static const Poly L_WPNL={
    -0.23f, 0.10f,-0.57f,-0.18f,-0.62f,-0.38f,-0.26f,-0.30f,
};
static const Poly R_WPNL={
     0.23f, 0.10f, 0.57f,-0.18f, 0.62f,-0.38f, 0.26f,-0.30f,
};
// ── Cockpit canopy ────────────────────────────────────────
static const Poly COCKPIT={
     0.00f, 0.78f,
     0.11f, 0.62f,  0.13f, 0.32f,  0.05f, 0.18f,
    -0.05f, 0.18f, -0.13f, 0.32f, -0.11f, 0.62f,
};
// Inner cockpit (darker, creates faceted look)
static const Poly COCKPIT_IN={
     0.00f, 0.72f,
     0.08f, 0.58f,  0.09f, 0.34f,  0.00f, 0.22f,
    -0.09f, 0.34f, -0.08f, 0.58f,
};
// ── Cannon pylons ─────────────────────────────────────────
static const Poly L_PYLON={
    -0.29f, 0.22f,-0.50f, 0.10f,-0.52f,-0.16f,-0.31f,-0.20f,
};
static const Poly R_PYLON={
     0.29f, 0.22f, 0.50f, 0.10f, 0.52f,-0.16f, 0.31f,-0.20f,
};
// ── Cannon barrels (4 rectangles) ─────────────────────────
static const Poly L_CAN1={-0.35f,0.26f,-0.28f,0.26f,-0.28f,-0.32f,-0.35f,-0.32f};
static const Poly L_CAN2={-0.45f,0.21f,-0.38f,0.21f,-0.38f,-0.27f,-0.45f,-0.27f};
static const Poly R_CAN1={ 0.28f,0.26f, 0.35f,0.26f, 0.35f,-0.32f, 0.28f,-0.32f};
static const Poly R_CAN2={ 0.38f,0.21f, 0.45f,0.21f, 0.45f,-0.27f, 0.38f,-0.27f};
// ── Cannon muzzle rings (orange) ──────────────────────────
static const Poly L_MZ1={-0.36f,-0.28f,-0.27f,-0.28f,-0.27f,-0.38f,-0.36f,-0.38f};
static const Poly L_MZ2={-0.46f,-0.23f,-0.37f,-0.23f,-0.37f,-0.33f,-0.46f,-0.33f};
static const Poly R_MZ1={ 0.27f,-0.28f, 0.36f,-0.28f, 0.36f,-0.38f, 0.27f,-0.38f};
static const Poly R_MZ2={ 0.37f,-0.23f, 0.46f,-0.23f, 0.46f,-0.33f, 0.37f,-0.33f};
// ── Centre body panel ─────────────────────────────────────
static const Poly CENTER_PNL={
     0.06f, 0.42f, 0.08f,-0.10f, 0.05f,-0.62f,
     0.00f,-0.72f,-0.05f,-0.62f,-0.08f,-0.10f,-0.06f, 0.42f,
};
// ── Nose accent diamond ───────────────────────────────────
static const Poly NOSE_ACC={
     0.00f, 0.66f, 0.06f, 0.54f, 0.00f, 0.46f,-0.06f, 0.54f,
};
// ── RF-01 decal backgrounds ───────────────────────────────
static const Poly L_DECAL={-0.54f,-0.24f,-0.32f,-0.24f,-0.32f,-0.38f,-0.54f,-0.38f};
static const Poly R_DECAL={ 0.32f,-0.24f, 0.54f,-0.24f, 0.54f,-0.38f, 0.32f,-0.38f};

// ─────────────────────────────────────────────
//  Rage-Fang Renderer  (modern GL)
// ─────────────────────────────────────────────
struct Part{int first,count; float r,g,b,a; bool outline;};

class RageFangRenderer {
    GLuint vao=0,vbo=0,prog=0;
    std::vector<Part> parts;
    GLint locPos,locAngle,locScale,locOutSc,locColor,locHW,locHH;

public:
    void init(){
        // ── Toon shader ────────────────────────────────────
        const char* VS=R"GLSL(
#version 330 core
layout(location=0) in vec2 aPos;
uniform vec2  uPos;
uniform float uAngle;
uniform float uScale;
uniform float uOutSc;
uniform float uHalfW;
uniform float uHalfH;
void main(){
    vec2 p=aPos*uOutSc;
    float c=cos(uAngle),s=sin(uAngle);
    vec2 r=vec2(c*p.x-s*p.y, s*p.x+c*p.y);
    vec2 w=r*uScale+uPos;
    gl_Position=vec4(w.x/uHalfW, w.y/uHalfH, 0.0, 1.0);
}
)GLSL";
        const char* FS=R"GLSL(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main(){ FragColor=uColor; }
)GLSL";
        prog=buildProgram(VS,FS);
        locPos  =glGetUniformLocation(prog,"uPos");
        locAngle=glGetUniformLocation(prog,"uAngle");
        locScale=glGetUniformLocation(prog,"uScale");
        locOutSc=glGetUniformLocation(prog,"uOutSc");
        locColor=glGetUniformLocation(prog,"uColor");
        locHW   =glGetUniformLocation(prog,"uHalfW");
        locHH   =glGetUniformLocation(prog,"uHalfH");

        // ── Color palette ──────────────────────────────────
        //  DGREY  dark grey body  #2B2B2B
        //  MGREY  wing mid grey
        //  LGREY  cannon light grey
        //  WPNL   wing panel darker
        //  PANEL  body panel
        //  ORANGE wing leading edge / muzzle #FF5500
        //  CORG   cockpit warm orange
        //  CDRK   cockpit dark interior
        //  DACC   dark orange nose accent
        //  DECBG  decal plate
        static const float
            DGREY[4]={0.17f,0.17f,0.17f,1.f},
            MGREY[4]={0.26f,0.26f,0.29f,1.f},
            LGREY[4]={0.36f,0.36f,0.39f,1.f},
            WPNL [4]={0.21f,0.21f,0.23f,1.f},
            PANEL[4]={0.13f,0.13f,0.15f,1.f},
            ORANGE[4]={1.0f,0.33f,0.0f,1.f},
            CORG [4]={1.0f,0.52f,0.04f,1.f},
            CDRK [4]={0.04f,0.04f,0.06f,1.f},
            DACC [4]={0.55f,0.18f,0.0f,1.f},
            DECBG[4]={0.22f,0.22f,0.26f,1.f};

        // ── Part list  {polygon, color, include-outline} ───
        // Draw order: back → front
        struct PD{const Poly& p;const float* c;bool ol;};
        std::vector<PD> defs={
            {L_WING,   MGREY, true },
            {R_WING,   MGREY, true },
            {L_WPNL,   WPNL,  false},
            {R_WPNL,   WPNL,  false},
            {L_EDGE,   ORANGE,true },
            {R_EDGE,   ORANGE,true },
            {L_FIN,    DGREY, true },
            {R_FIN,    DGREY, true },
            {L_PYLON,  MGREY, true },
            {R_PYLON,  MGREY, true },
            {L_CAN1,   LGREY, true },
            {L_CAN2,   LGREY, true },
            {R_CAN1,   LGREY, true },
            {R_CAN2,   LGREY, true },
            {L_MZ1,    ORANGE,false},
            {L_MZ2,    ORANGE,false},
            {R_MZ1,    ORANGE,false},
            {R_MZ2,    ORANGE,false},

            {FUSE,     DGREY, true },
            {CENTER_PNL,PANEL,false},
            {COCKPIT,  CORG,  true },
            {COCKPIT_IN,CDRK, false},
            {NOSE_ACC, DACC,  false},
        };

        // ── Build flat VBO ─────────────────────────────────
        std::vector<float> buf;
        for(auto& d:defs){
            auto tris=triPoly(d.p);
            Part pt;
            pt.first=(int)buf.size()/2;
            pt.count=(int)tris.size()/2;
            pt.r=d.c[0];pt.g=d.c[1];pt.b=d.c[2];pt.a=d.c[3];
            pt.outline=d.ol;
            parts.push_back(pt);
            buf.insert(buf.end(),tris.begin(),tris.end());
        }

        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,buf.size()*sizeof(float),
                     buf.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    // Called BEFORE drawing the ship (engine glow goes behind)
    void drawGlow(Vec2 pos,float angleDeg,double t){
        float angle=(angleDeg-90.f)*(float)M_PI/180.f;
        float c=cosf(angle),s=sinf(angle);
        auto wp=[&](float lx,float ly)->Vec2{
            return {(c*lx-s*ly)*SHIP_SCALE+pos.x,
                    (s*lx+c*ly)*SHIP_SCALE+pos.y};
        };
        float f1=0.82f+0.18f*(float)sin(t*22.0);
        float f2=0.80f+0.20f*(float)sin(t*18.0+1.2f);

        // Main engine
        Vec2 me=wp(0.f,-0.98f);
        setColor(1.f,0.50f,0.f,0.85f); drawPoly(me,12.f*f1,10);
        setColor(1.f,0.85f,0.2f,0.75f);drawPoly(me, 6.f*f1,10);
        setColor(1.f,1.f,0.8f,0.70f); drawPoly(me, 2.5f,10);

        // Cannon exhausts (4 plumes)
        Vec2 e1=wp(-0.315f,-0.32f),e2=wp(-0.415f,-0.27f);
        Vec2 e3=wp( 0.315f,-0.32f),e4=wp( 0.415f,-0.27f);
        setColor(1.f,0.42f,0.f,0.80f);
        drawPoly(e1,6.f*f2,8); drawPoly(e3,6.f*f1,8);
        drawPoly(e2,5.f*f1,8); drawPoly(e4,5.f*f2,8);
        setColor(1.f,0.75f,0.1f,0.65f);
        drawPoly(e1,3.f*f2,8); drawPoly(e3,3.f*f1,8);
    }

    // Draw the toon ship
    void draw(Vec2 pos,float angleDeg,float invTimer,double t){
        if(!prog) return;
        // Invincibility blink
        if(invTimer>0.f&&(int)(invTimer*8)%2==0) return;

        float angle=(angleDeg-90.f)*(float)M_PI/180.f;

        // ── Modern GL toon passes ───────────────────────────
        glUseProgram(prog);
        glUniform2f(locPos,pos.x,pos.y);
        glUniform1f(locAngle,angle);
        glUniform1f(locScale,SHIP_SCALE);
        glUniform1f(locHW,HALF_W);
        glUniform1f(locHH,HALF_H);
        glBindVertexArray(vao);

        // Pass 1: thick black outline (scale up 1.13)
        glUniform1f(locOutSc,1.13f);
        glUniform4f(locColor,0.f,0.f,0.f,1.f);
        for(auto& p:parts)
            if(p.outline)
                glDrawArrays(GL_TRIANGLES,p.first,p.count);

        // Pass 2: flat colored fill
        glUniform1f(locOutSc,1.0f);
        for(auto& p:parts){
            glUniform4f(locColor,p.r,p.g,p.b,p.a);
            glDrawArrays(GL_TRIANGLES,p.first,p.count);
        }

        glBindVertexArray(0);
        glUseProgram(0);

        float S=SHIP_SCALE;

        // ── Legacy GL: cannon muzzle glow ───────────────────
        float ang=(angleDeg-90.f)*(float)M_PI/180.f;
        float cc=cosf(ang),ss=sinf(ang);
        auto wp=[&](float lx,float ly)->Vec2{
            return {(cc*lx-ss*ly)*S+pos.x,(ss*lx+cc*ly)*S+pos.y};
        };
        float g=0.80f+0.20f*(float)sin(t*25.0);
        setColor(1.f,0.55f,0.f,0.75f);
        drawPoly(wp(-0.315f,0.26f),4.5f*g,8);
        drawPoly(wp(-0.415f,0.21f),4.0f*g,8);
        drawPoly(wp( 0.315f,0.26f),4.5f*g,8);
        drawPoly(wp( 0.415f,0.21f),4.0f*g,8);
        setColor(1.f,0.9f,0.5f,0.60f);
        drawPoly(wp(-0.315f,0.26f),2.f,8);
        drawPoly(wp(-0.415f,0.21f),2.f,8);
        drawPoly(wp( 0.315f,0.26f),2.f,8);
        drawPoly(wp( 0.415f,0.21f),2.f,8);
    }

    void cleanup(){
        if(vao){glDeleteVertexArrays(1,&vao);vao=0;}
        if(vbo){glDeleteBuffers(1,&vbo);vbo=0;}
        if(prog){glDeleteProgram(prog);prog=0;}
    }
};

// ─────────────────────────────────────────────
//  Projectile
// ─────────────────────────────────────────────
struct Projectile{
    Vec2 pos,vel;
    bool active=true;
    void update(float dt){
        pos.x+=vel.x*dt; pos.y+=vel.y*dt;
        if(pos.x<-HALF_W||pos.x>HALF_W||pos.y<-HALF_H||pos.y>HALF_H)
            active=false;
    }
    void draw()const{
        if(!active)return;
        setColor(1.f,0.92f,0.3f);
        drawPoly(pos,BULLET_RADIUS,8);
        setColor(1.f,0.55f,0.f,0.5f);
        drawPoly(pos,BULLET_RADIUS+3.f,8);
    }
};

// ─────────────────────────────────────────────
//  Alien
// ─────────────────────────────────────────────
enum class AlienType{NORMAL,BOSS};

struct Alien{
    Vec2 pos,vel;
    int hp;
    AlienType type;
    bool  active=true;
    float angle=0.f;
    float shootTimer=0.f;  // boss shoot cooldown
    Vec2  facingDir={0.f,-1.f}; // direction boss faces

    static Alien makeNormal(){
        Alien a; a.type=AlienType::NORMAL; a.hp=1;
        int e=rand()%4;
        switch(e){
            case 0:a.pos={randRange(-HALF_W,HALF_W), HALF_H+20};break;
            case 1:a.pos={randRange(-HALF_W,HALF_W),-HALF_H-20};break;
            case 2:a.pos={ HALF_W+20,randRange(-HALF_H,HALF_H)};break;
            case 3:a.pos={-HALF_W-20,randRange(-HALF_H,HALF_H)};break;
        }
        float dx=randRange(-HALF_W*.4f,HALF_W*.4f)-a.pos.x;
        float dy=randRange(-HALF_H*.4f,HALF_H*.4f)-a.pos.y;
        float len=sqrtf(dx*dx+dy*dy);
        a.vel={dx/len*ALIEN_SPEED,dy/len*ALIEN_SPEED};
        return a;
    }
    static Alien makeBoss(){
        Alien a; a.type=AlienType::BOSS; a.hp=BOSS_HP;
        int e=rand()%4;
        switch(e){
            case 0:a.pos={randRange(-HALF_W,HALF_W), HALF_H+40};break;
            case 1:a.pos={randRange(-HALF_W,HALF_W),-HALF_H-40};break;
            case 2:a.pos={ HALF_W+40,randRange(-HALF_H,HALF_H)};break;
            case 3:a.pos={-HALF_W-40,randRange(-HALF_H,HALF_H)};break;
        }
        float dx=-a.pos.x,dy=-a.pos.y;
        float len=sqrtf(dx*dx+dy*dy);
        a.vel={dx/len*BOSS_SPEED,dy/len*BOSS_SPEED};
        return a;
    }
    // For normal aliens only (boss facing updated in Game::update)
    void update(float dt){
        if(!active)return;
        pos.x+=vel.x*dt; pos.y+=vel.y*dt;
        angle+=60.f*dt;
        if(type==AlienType::BOSS) shootTimer-=dt;
        if(pos.x<-HALF_W-60||pos.x>HALF_W+60||
           pos.y<-HALF_H-60||pos.y>HALF_H+60)
            active=false;
    }
    void faceTarget(Vec2 target){
        float dx=target.x-pos.x, dy=target.y-pos.y;
        float len=sqrtf(dx*dx+dy*dy);
        if(len>0.1f){ facingDir={dx/len,dy/len}; }
    }
    Projectile fireAt(Vec2 target)const{
        Projectile p;
        float dx=target.x-pos.x, dy=target.y-pos.y;
        float len=sqrtf(dx*dx+dy*dy);
        if(len<0.1f){p.active=false;return p;}
        p.pos=pos;
        // Aim slightly ahead of player for difficulty
        p.vel={dx/len*BOSS_BULLET_SPEED, dy/len*BOSS_BULLET_SPEED};
        return p;
    }
    void draw()const{
        if(!active)return;
        if(type==AlienType::NORMAL){
            drawNormalAlien();
        } else {
            drawBoss();
        }
    }

    void drawNormalAlien()const{
        float R = ALIEN_RADIUS;
        float px = pos.x, py = pos.y;
        // alien faces direction of movement (or rotation angle for spin)
        float fa = atan2f(vel.y, vel.x) - (float)M_PI/2.f;
        float fc = cosf(fa), fs = sinf(fa);
        // rotate local→world
        auto rv=[&](float lx,float ly){ glVertex2f(px+fc*lx-fs*ly, py+fs*lx+fc*ly); };
        auto rp=[&](float lx,float ly)->Vec2{ return {px+fc*lx-fs*ly, py+fs*lx+fc*ly}; };

        // ── Drop shadow ──────────────────────────────────────
        setColor(0.f,0.f,0.f,0.22f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px+3.f,py-3.f);
        for(int i=0;i<=14;i++){
            float a=(float)i/14*2.f*(float)M_PI;
            glVertex2f(px+3.f+cosf(a)*R*1.5f, py-3.f+sinf(a)*R*0.85f);
        }
        glEnd();

        // ── Back swept fins (top and bottom) ─────────────────
        // Top fin – black outline first
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv( 0.f,  R*0.05f);
        rv(-R*0.35f, R*1.05f); rv( R*0.05f, R*1.45f);
        rv( R*0.65f, R*1.10f); rv( R*0.5f,  R*0.05f);
        glEnd();
        setColor(0.72f,0.22f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv( 0.f,  R*0.05f);
        rv(-R*0.30f, R*1.00f); rv( R*0.05f, R*1.38f);
        rv( R*0.60f, R*1.05f); rv( R*0.45f,  R*0.05f);
        glEnd();
        // Fin highlight
        setColor(0.88f,0.36f,0.08f);
        glBegin(GL_TRIANGLES);
        rv( 0.1f, R*0.2f); rv( R*0.45f, R*1.05f); rv( R*0.1f, R*0.8f);
        glEnd();

        // Bottom fin – outline
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv( 0.f, -R*0.05f);
        rv(-R*0.35f,-R*1.05f); rv( R*0.05f,-R*1.45f);
        rv( R*0.65f,-R*1.10f); rv( R*0.5f, -R*0.05f);
        glEnd();
        setColor(0.72f,0.22f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv( 0.f, -R*0.05f);
        rv(-R*0.30f,-R*1.00f); rv( R*0.05f,-R*1.38f);
        rv( R*0.60f,-R*1.05f); rv( R*0.45f,-R*0.05f);
        glEnd();
        setColor(0.88f,0.36f,0.08f);
        glBegin(GL_TRIANGLES);
        rv( 0.1f,-R*0.2f); rv( R*0.45f,-R*1.05f); rv( R*0.1f,-R*0.8f);
        glEnd();

        // ── Main body outline (ellipsoid) ─────────────────────
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0,0);
        for(int i=0;i<=20;i++){
            float a=(float)i/20*2.f*(float)M_PI;
            rv(cosf(a)*R*1.52f, sinf(a)*R*1.02f);
        }
        glEnd();

        // ── Carapace plates ───────────────────────────────────
        // Base terracotta/dark-orange
        setColor(0.62f,0.18f,0.05f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0,0);
        for(int i=0;i<=20;i++){
            float a=(float)i/20*2.f*(float)M_PI;
            rv(cosf(a)*R*1.45f, sinf(a)*R*0.95f);
        }
        glEnd();

        // Mid plate highlight
        setColor(0.82f,0.30f,0.07f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0.f,0.f);
        rv(-R*0.55f, R*0.55f); rv( R*0.25f, R*0.65f);
        rv( R*0.90f, R*0.30f); rv( R*1.05f, 0.f);
        rv( R*0.90f,-R*0.30f); rv( R*0.25f,-R*0.65f);
        rv(-R*0.55f,-R*0.55f); rv(-R*1.00f, 0.f);
        rv(-R*0.55f, R*0.55f);
        glEnd();

        // Front panel (left side – brighter)
        setColor(0.90f,0.38f,0.10f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-R*0.5f, 0.f);
        rv(-R*1.1f, R*0.35f); rv(-R*0.8f, R*0.65f);
        rv(-R*0.2f, R*0.60f); rv(-R*0.1f, 0.f);
        rv(-R*0.2f,-R*0.60f); rv(-R*0.8f,-R*0.65f);
        rv(-R*1.1f,-R*0.35f);
        glEnd();

        // Right rear plate
        setColor(0.70f,0.24f,0.06f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.5f,0.f);
        rv(R*0.2f, R*0.55f); rv(R*0.85f, R*0.45f);
        rv(R*1.10f, 0.f);    rv(R*0.85f,-R*0.45f);
        rv(R*0.2f,-R*0.55f);
        glEnd();

        // ── Plate crack / panel lines ─────────────────────────
        setColor(0.20f,0.06f,0.01f);
        glLineWidth(1.8f);
        glBegin(GL_LINES);
        // Diagonal spine
        rv(-R*0.6f, R*0.5f);  rv( R*0.6f,-R*0.5f);
        rv(-R*0.6f,-R*0.5f);  rv( R*0.6f, R*0.5f);
        // Vertical split
        rv( 0.f, R*0.85f);    rv( 0.f,-R*0.85f);
        // Horizontal belt
        rv(-R*1.1f, 0.f);     rv( R*1.1f, 0.f);
        // Front segment
        rv(-R*0.8f, R*0.4f);  rv(-R*0.4f,-R*0.4f);
        rv(-R*0.8f,-R*0.4f);  rv(-R*0.4f, R*0.4f);
        glEnd();
        glLineWidth(1.f);

        // ── Eye cluster (4 cyan orbs, diamond pattern) ────────
        // Panel background for eye cluster
        setColor(0.30f,0.08f,0.02f);
        Vec2 ec = rp(-R*0.55f, 0.f); // cluster centre
        drawPoly(ec, R*0.52f, 12);

        float er = R*0.13f; // orb radius
        // 4 orbs in diamond: top, bottom, left, right
        float offsets[4][2] = {{0.f,R*0.22f},{0.f,-R*0.22f},{-R*0.22f,0.f},{R*0.22f,0.f}};
        for(auto& o : offsets){
            Vec2 op = rp(-R*0.55f+o[0], o[1]);
            setColor(0.f,0.f,0.f);           drawPoly(op,er*1.25f,10);
            setColor(0.08f,0.72f,0.72f,0.9f);drawPoly(op,er,10);
            setColor(0.55f,0.97f,0.97f,0.8f);drawPoly(op,er*0.55f,10);
            setColor(0.90f,1.f,1.f,0.7f);    drawPoly(op,er*0.25f,10);
        }

        // ── Cockpit teardrop (elongated eye, center-top) ──────
        // Outline
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.10f, R*0.28f);
        // teardrop: blend oval and pointed tip
        for(int i=0;i<=16;i++){
            float t=(float)i/16*2.f*(float)M_PI;
            float ex=cosf(t)*R*0.42f*(1.f+0.28f*cosf(t));
            float ey=sinf(t)*R*0.17f;
            rv(R*0.10f+ex, R*0.28f+ey);
        }
        glEnd();
        // Dark teal fill
        setColor(0.04f,0.38f,0.36f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.10f, R*0.28f);
        for(int i=0;i<=16;i++){
            float t=(float)i/16*2.f*(float)M_PI;
            float ex=cosf(t)*R*0.38f*(1.f+0.25f*cosf(t));
            float ey=sinf(t)*R*0.14f;
            rv(R*0.10f+ex, R*0.28f+ey);
        }
        glEnd();
        // Bright cyan glow core
        setColor(0.18f,0.88f,0.85f,0.85f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.10f, R*0.28f);
        for(int i=0;i<=14;i++){
            float t=(float)i/14*2.f*(float)M_PI;
            float ex=cosf(t)*R*0.22f*(1.f+0.20f*cosf(t));
            float ey=sinf(t)*R*0.08f;
            rv(R*0.10f+ex, R*0.28f+ey);
        }
        glEnd();
        // Inner white highlight
        setColor(0.80f,1.f,0.98f,0.70f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.05f, R*0.30f);
        for(int i=0;i<=10;i++){
            float t=(float)i/10*2.f*(float)M_PI;
            rv(R*0.05f+cosf(t)*R*0.10f, R*0.30f+sinf(t)*R*0.045f);
        }
        glEnd();
    }
    // Helper: draw a filled quad
    static void quad(float x0,float y0,float x1,float y1,
                     float x2,float y2,float x3,float y3){
        glBegin(GL_QUADS);
        glVertex2f(x0,y0);glVertex2f(x1,y1);
        glVertex2f(x2,y2);glVertex2f(x3,y3);
        glEnd();
    }
    // Helper: draw a filled triangle
    static void tri(float x0,float y0,float x1,float y1,float x2,float y2){
        glBegin(GL_TRIANGLES);
        glVertex2f(x0,y0);glVertex2f(x1,y1);glVertex2f(x2,y2);
        glEnd();
    }

    void drawBoss()const{
        float R=BOSS_RADIUS;
        float dmg=(float)hp/BOSS_HP;
        float px=pos.x, py=pos.y;

        // Facing angle (boss rotates toward player)
        float fa=atan2f(facingDir.y,facingDir.x)-(float)M_PI/2.f;
        float fc=cosf(fa), fs=sinf(fa);
        // Rotate a local point around boss center
        auto R2=[&](float lx,float ly)->std::pair<float,float>{
            return {px+fc*lx-fs*ly, py+fs*lx+fc*ly};
        };
        auto rv=[&](float lx,float ly){
            auto [wx,wy]=R2(lx,ly); glVertex2f(wx,wy);
        };

        // ── Drop shadow (offset bottom-right) ──────────────
        float sx=4.f,sy=-4.f;
        setColor(0.f,0.f,0.f,0.25f);
        auto rs=[&](float lx,float ly){
            auto [wx,wy]=R2(lx,ly); glVertex2f(wx+sx,wy+sy);
        };
        glBegin(GL_TRIANGLE_FAN);
        rs(0,0);
        for(int i=0;i<=12;i++){
            float a=(float)i/12*2.f*(float)M_PI;
            rs(cosf(a)*R*1.3f,sinf(a)*R*1.3f);
        }
        glEnd();

        // ── Swept back wings (dark red) ─────────────────────
        float wo=R*0.3f; // wing overlap with body
        // Left wing
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-wo,-R*0.1f); rv(-R*1.6f,R*0.5f); rv(-R*1.8f,-R*0.2f);
        rv(-R*1.3f,-R*0.8f); rv(-wo,-R*0.5f);
        glEnd();
        setColor(0.50f,0.08f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-wo,-R*0.1f); rv(-R*1.5f, R*0.4f); rv(-R*1.7f,-R*0.15f);
        rv(-R*1.2f,-R*0.75f); rv(-wo,-R*0.45f);
        glEnd();
        // Wing accent (lighter orange stripe)
        setColor(0.80f,0.28f,0.05f);
        glBegin(GL_TRIANGLES);
        rv(-R*0.5f,R*0.3f); rv(-R*1.4f,R*0.35f); rv(-R*1.5f,-R*0.1f);
        rv(-R*0.5f,R*0.3f); rv(-R*1.5f,-R*0.1f); rv(-R*0.6f,-R*0.3f);
        glEnd();

        // Right wing (mirror)
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(wo,-R*0.1f); rv(R*1.6f,R*0.5f); rv(R*1.8f,-R*0.2f);
        rv(R*1.3f,-R*0.8f); rv(wo,-R*0.5f);
        glEnd();
        setColor(0.50f,0.08f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(wo,-R*0.1f); rv(R*1.5f, R*0.4f); rv(R*1.7f,-R*0.15f);
        rv(R*1.2f,-R*0.75f); rv(wo,-R*0.45f);
        glEnd();
        setColor(0.80f,0.28f,0.05f);
        glBegin(GL_TRIANGLES);
        rv(R*0.5f,R*0.3f); rv(R*1.4f,R*0.35f); rv(R*1.5f,-R*0.1f);
        rv(R*0.5f,R*0.3f); rv(R*1.5f,-R*0.1f); rv(R*0.6f,-R*0.3f);
        glEnd();

        // ── Claw appendages (4 claws) ───────────────────────
        // Front-left claw
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-R*0.4f,R*0.7f); rv(-R*1.0f,R*1.4f); rv(-R*1.3f,R*1.1f);
        rv(-R*1.15f,R*0.9f); rv(-R*0.6f,R*0.5f);
        glEnd();
        setColor(0.72f,0.18f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-R*0.4f,R*0.7f); rv(-R*0.95f,R*1.3f); rv(-R*1.2f,R*1.05f);
        rv(-R*1.05f,R*0.85f); rv(-R*0.55f,R*0.5f);
        glEnd();
        // Claw tips (sharp black)
        setColor(0.08f,0.08f,0.08f);
        glBegin(GL_TRIANGLES);
        rv(-R*1.0f,R*1.4f); rv(-R*1.35f,R*1.6f); rv(-R*1.3f,R*1.1f);
        rv(-R*0.95f,R*1.3f); rv(-R*1.1f,R*1.55f); rv(-R*1.2f,R*1.05f);
        glEnd();

        // Front-right claw
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.4f,R*0.7f); rv(R*1.0f,R*1.4f); rv(R*1.3f,R*1.1f);
        rv(R*1.15f,R*0.9f); rv(R*0.6f,R*0.5f);
        glEnd();
        setColor(0.72f,0.18f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.4f,R*0.7f); rv(R*0.95f,R*1.3f); rv(R*1.2f,R*1.05f);
        rv(R*1.05f,R*0.85f); rv(R*0.55f,R*0.5f);
        glEnd();
        setColor(0.08f,0.08f,0.08f);
        glBegin(GL_TRIANGLES);
        rv(R*1.0f,R*1.4f); rv(R*1.35f,R*1.6f); rv(R*1.3f,R*1.1f);
        rv(R*0.95f,R*1.3f); rv(R*1.1f,R*1.55f); rv(R*1.2f,R*1.05f);
        glEnd();

        // Rear-left claw
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-R*0.5f,-R*0.8f); rv(-R*0.9f,-R*1.35f); rv(-R*1.15f,-R*1.0f);
        rv(-R*0.7f,-R*0.6f);
        glEnd();
        setColor(0.72f,0.18f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(-R*0.5f,-R*0.8f); rv(-R*0.85f,-R*1.3f); rv(-R*1.1f,-R*0.95f);
        rv(-R*0.65f,-R*0.6f);
        glEnd();
        setColor(0.08f,0.08f,0.08f);
        glBegin(GL_TRIANGLES);
        rv(-R*0.9f,-R*1.35f); rv(-R*1.0f,-R*1.6f); rv(-R*1.15f,-R*1.0f);
        glEnd();

        // Rear-right claw
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.5f,-R*0.8f); rv(R*0.9f,-R*1.35f); rv(R*1.15f,-R*1.0f);
        rv(R*0.7f,-R*0.6f);
        glEnd();
        setColor(0.72f,0.18f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(R*0.5f,-R*0.8f); rv(R*0.85f,-R*1.3f); rv(R*1.1f,-R*0.95f);
        rv(R*0.65f,-R*0.6f);
        glEnd();
        setColor(0.08f,0.08f,0.08f);
        glBegin(GL_TRIANGLES);
        rv(R*0.5f,-R*0.8f); rv(R*1.0f,-R*1.6f); rv(R*1.15f,-R*1.0f);
        glEnd();

        // ── Main body outline ───────────────────────────────
        setColor(0.f,0.f,0.f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0,0);
        for(int i=0;i<=16;i++){
            float a=(float)i/16*2.f*(float)M_PI;
            rv(cosf(a)*R*1.02f,sinf(a)*R*1.02f);
        }
        glEnd();

        // ── Main body (diamond/hexagonal) ──────────────────
        // Base dark red
        setColor(0.55f,0.10f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0,0);
        float bvx[]={0,R*0.5f,R*0.85f,R*0.7f,R*0.4f,0,-R*0.4f,-R*0.7f,-R*0.85f,-R*0.5f,0};
        float bvy[]={R,R*0.6f,0,-R*0.6f,-R*0.9f,-R*0.7f,-R*0.9f,-R*0.6f,0,R*0.6f,R};
        for(int i=0;i<=10;i++) rv(bvx[i],bvy[i]);
        glEnd();
        // Mid orange highlight
        setColor(0.80f,0.22f,0.04f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0,0);
        float mvx[]={0,R*0.35f,R*0.6f,R*0.5f,R*0.25f,0,-R*0.25f,-R*0.5f,-R*0.6f,-R*0.35f,0};
        float mvy[]={R*0.7f,R*0.45f,0,-R*0.4f,-R*0.65f,-R*0.5f,-R*0.65f,-R*0.4f,0,R*0.45f,R*0.7f};
        for(int i=0;i<=10;i++) rv(mvx[i],mvy[i]);
        glEnd();
        // Bright centre spine
        setColor(0.90f,0.35f,0.08f);
        glBegin(GL_TRIANGLE_FAN);
        rv(0,0);
        rv(-R*0.12f,R*0.65f); rv(R*0.12f,R*0.65f);
        rv(R*0.18f,0); rv(R*0.12f,-R*0.6f);
        rv(-R*0.12f,-R*0.6f); rv(-R*0.18f,0);
        rv(-R*0.12f,R*0.65f);
        glEnd();

        // ── Armour panel lines ──────────────────────────────
        setColor(0.25f,0.05f,0.02f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        // Diagonal panel cuts
        rv(-R*0.5f,R*0.5f); rv(-R*0.1f,0.f);
        rv( R*0.5f,R*0.5f); rv( R*0.1f,0.f);
        rv(-R*0.5f,-R*0.4f); rv(-R*0.1f,0.f);
        rv( R*0.5f,-R*0.4f); rv( R*0.1f,0.f);
        rv(-R*0.7f,0.f); rv(-R*0.2f,R*0.5f);
        rv( R*0.7f,0.f); rv( R*0.2f,R*0.5f);
        glEnd();
        glLineWidth(1.f);

        // ── Cyan thruster ports (rear) ──────────────────────
        float thrGlow=0.7f+0.3f*(float)sin(angle*0.3f);
        // Left thruster
        setColor(0.f,0.f,0.f);
        drawPoly({px+fc*(-R*0.45f)-fs*(-R*0.75f),
                  py+fs*(-R*0.45f)+fc*(-R*0.75f)},R*0.18f,8);
        setColor(0.0f,0.9f,0.85f,0.9f);
        drawPoly({px+fc*(-R*0.45f)-fs*(-R*0.75f),
                  py+fs*(-R*0.45f)+fc*(-R*0.75f)},R*0.14f*thrGlow,8);
        setColor(0.6f,1.f,1.f,0.6f);
        drawPoly({px+fc*(-R*0.45f)-fs*(-R*0.75f),
                  py+fs*(-R*0.45f)+fc*(-R*0.75f)},R*0.07f,8);
        // Right thruster
        setColor(0.f,0.f,0.f);
        drawPoly({px+fc*(R*0.45f)-fs*(-R*0.75f),
                  py+fs*(R*0.45f)+fc*(-R*0.75f)},R*0.18f,8);
        setColor(0.0f,0.9f,0.85f,0.9f);
        drawPoly({px+fc*(R*0.45f)-fs*(-R*0.75f),
                  py+fs*(R*0.45f)+fc*(-R*0.75f)},R*0.14f*thrGlow,8);
        setColor(0.6f,1.f,1.f,0.6f);
        drawPoly({px+fc*(R*0.45f)-fs*(-R*0.75f),
                  py+fs*(R*0.45f)+fc*(-R*0.75f)},R*0.07f,8);

        // ── Cyan thruster exhaust plumes ────────────────────
        setColor(0.0f,0.85f,0.80f,0.55f*thrGlow);
        drawPoly({px+fc*(-R*0.45f)-fs*(-R*0.95f),
                  py+fs*(-R*0.45f)+fc*(-R*0.95f)},R*0.25f*thrGlow,8);
        drawPoly({px+fc*(R*0.45f)-fs*(-R*0.95f),
                  py+fs*(R*0.45f)+fc*(-R*0.95f)},R*0.25f*thrGlow,8);

        // ── Cyan eye sockets ────────────────────────────────
        // Eye socket outlines
        setColor(0.f,0.f,0.f);
        drawPoly({px+fc*(-R*0.32f)-fs*(R*0.22f),
                  py+fs*(-R*0.32f)+fc*(R*0.22f)},R*0.22f,10);
        drawPoly({px+fc*(R*0.32f)-fs*(R*0.22f),
                  py+fs*(R*0.32f)+fc*(R*0.22f)},R*0.22f,10);
        // Eye glow rings
        float eyeGlow=0.75f+0.25f*(float)sin(angle*0.5f+1.f);
        setColor(0.0f,0.85f,0.80f,0.85f);
        drawPoly({px+fc*(-R*0.32f)-fs*(R*0.22f),
                  py+fs*(-R*0.32f)+fc*(R*0.22f)},R*0.18f*eyeGlow,10);
        drawPoly({px+fc*(R*0.32f)-fs*(R*0.22f),
                  py+fs*(R*0.32f)+fc*(R*0.22f)},R*0.18f*eyeGlow,10);
        // Eye inner whites
        setColor(0.75f,1.f,0.98f,0.90f);
        drawPoly({px+fc*(-R*0.32f)-fs*(R*0.22f),
                  py+fs*(-R*0.32f)+fc*(R*0.22f)},R*0.09f,10);
        drawPoly({px+fc*(R*0.32f)-fs*(R*0.22f),
                  py+fs*(R*0.32f)+fc*(R*0.22f)},R*0.09f,10);

        // ── Centre reactor orb ──────────────────────────────
        setColor(0.f,0.f,0.f);
        drawPoly({px,py},R*0.17f,12);
        setColor(0.0f,0.75f,0.70f,0.80f);
        drawPoly({px,py},R*0.14f,12);
        setColor(0.7f,1.f,0.98f,0.70f);
        drawPoly({px,py},R*0.07f,12);

        // ── HP bar ──────────────────────────────────────────
        float bw=R*3.5f,bh=6.f;
        float bx=px-bw/2, by=py+R*1.5f;
        setColor(0.15f,0.15f,0.15f);
        glBegin(GL_QUADS);
        glVertex2f(bx,by);     glVertex2f(bx+bw,by);
        glVertex2f(bx+bw,by+bh); glVertex2f(bx,by+bh);
        glEnd();
        // Color shifts red→yellow as damage increases
        setColor(dmg,0.9f*(1.f-dmg*0.5f),0.f);
        glBegin(GL_QUADS);
        glVertex2f(bx,by);           glVertex2f(bx+bw*dmg,by);
        glVertex2f(bx+bw*dmg,by+bh); glVertex2f(bx,by+bh);
        glEnd();
        // Bar border
        setColor(0.f,0.7f,0.65f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(bx,by);     glVertex2f(bx+bw,by);
        glVertex2f(bx+bw,by+bh); glVertex2f(bx,by+bh);
        glEnd();
        glLineWidth(1.f);
    }
    float radius()const{return type==AlienType::NORMAL?ALIEN_RADIUS:BOSS_RADIUS;}
};

// ─────────────────────────────────────────────
//  Player  (data only – drawing via RageFangRenderer)
// ─────────────────────────────────────────────
struct Player{
    Vec2  pos={0,0};
    float angle=90.f;
    int   lives=3;
    float invTimer=0.f;
    bool  moveUp=false,moveDown=false,moveLeft=false,moveRight=false;

    void update(float dt){
        float dx=0,dy=0;
        if(moveUp)   dy+=1;
        if(moveDown) dy-=1;
        if(moveRight)dx+=1;
        if(moveLeft) dx-=1;
        float len=sqrtf(dx*dx+dy*dy);
        if(len>0){dx/=len;dy/=len;}
        pos.x+=dx*PLAYER_SPEED*dt;
        pos.y+=dy*PLAYER_SPEED*dt;
        pos.x=std::max(-HALF_W+PLAYER_RADIUS,std::min(HALF_W-PLAYER_RADIUS,pos.x));
        pos.y=std::max(-HALF_H+PLAYER_RADIUS,std::min(HALF_H-PLAYER_RADIUS,pos.y));
        if(invTimer>0)invTimer-=dt;
    }
    void aimAt(float mx,float my){
        float dx=mx-pos.x,dy=my-pos.y;
        angle=atan2f(dy,dx)*180.f/(float)M_PI;
    }
    Projectile fire()const{
        Projectile p;
        float a=angle*(float)M_PI/180.f;
        p.pos=pos;
        p.vel={cosf(a)*BULLET_SPEED,sinf(a)*BULLET_SPEED};
        return p;
    }
    bool isAlive()const{return lives>0;}
};

// ─────────────────────────────────────────────
//  Game
// ─────────────────────────────────────────────
struct Game{
    GLFWwindow*             window=nullptr;
    Player                  player;
    std::vector<Alien>      aliens;
    std::vector<Projectile> bullets;
    std::vector<Projectile> enemyBullets; // boss shots
    RageFangRenderer        ship;

    int   score=0,normalKills=0;
    float spawnTimer=0,spawnInterval=2.5f;
    bool  gameOver=false;
    float mouseX=0,mouseY=0;
    double gameTime=0.0;

    // ── Init ──────────────────────────────────────────────
    bool init(){
        if(!glfwInit()){std::cerr<<"GLFW fail\n";return false;}
        glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
        // Request compatibility profile so legacy GL still works
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
        glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_COMPAT_PROFILE);
        window=glfwCreateWindow(WINDOW_W,WINDOW_H,
                                "Space Shooter – Rage-Fang",
                                nullptr,nullptr);
        if(!window){glfwTerminate();return false;}
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glewExperimental=GL_TRUE;
        if(glewInit()!=GLEW_OK){std::cerr<<"GLEW fail\n";return false;}

        glfwSetWindowUserPointer(window,this);
        glfwSetKeyCallback(window,keyCallback);
        glfwSetMouseButtonCallback(window,mouseButtonCallback);
        glfwSetCursorPosCallback(window,cursorCallback);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        ship.init();
        srand((unsigned)time(nullptr));
        return true;
    }

    void setupProjection(){
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-HALF_W,HALF_W,-HALF_H,HALF_H,-1,1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    // ── Main loop ─────────────────────────────────────────
    void run(){
        if(!init())return;
        setupProjection();
        double prev=glfwGetTime();
        while(!glfwWindowShouldClose(window)){
            double now=glfwGetTime();
            float dt=(float)(now-prev); prev=now;
            if(dt>0.05f)dt=0.05f;
            glfwPollEvents();
            if(!gameOver){ update(dt); render(); }
            else renderGameOver();
            glfwSwapBuffers(window);
        }
        ship.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    // ── Update ────────────────────────────────────────────
    void update(float dt){
        gameTime+=dt;
        player.aimAt(mouseX,mouseY);
        player.update(dt);

        for(auto& b:bullets)b.update(dt);
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),
            [](const Projectile& p){return !p.active;}),bullets.end());

        for(auto& a:aliens){
            a.update(dt);
            if(a.type==AlienType::BOSS && a.active){
                a.faceTarget(player.pos);
                if(a.shootTimer<=0.f){
                    a.shootTimer=BOSS_FIRE_RATE;
                    enemyBullets.push_back(a.fireAt(player.pos));
                }
            }
        }
        aliens.erase(std::remove_if(aliens.begin(),aliens.end(),
            [](const Alien& a){return !a.active;}),aliens.end());
        // Enemy bullets
        for(auto& b:enemyBullets)b.update(dt);
        enemyBullets.erase(std::remove_if(enemyBullets.begin(),enemyBullets.end(),
            [](const Projectile& p){return !p.active;}),enemyBullets.end());

        spawnTimer-=dt;
        if(spawnTimer<=0){
            spawnTimer=std::max(0.6f,spawnInterval-score*0.003f);
            aliens.push_back(Alien::makeNormal());
        }

        // Bullet ↔ alien
        for(auto& b:bullets){
            if(!b.active)continue;
            for(auto& a:aliens){
                if(!a.active)continue;
                if(circleCollide(b.pos,BULLET_RADIUS,a.pos,a.radius())){
                    b.active=false; a.hp--;
                    if(a.hp<=0){
                        a.active=false;
                        if(a.type==AlienType::NORMAL){
                            score+=10; normalKills++;
                            if(normalKills%BOSS_SPAWN_EVERY==0)
                                aliens.push_back(Alien::makeBoss());
                        } else { score+=50; }
                    }
                    break;
                }
            }
        }

        // Alien ↔ player
        if(player.invTimer<=0){
            for(auto& a:aliens){
                if(!a.active)continue;
                if(circleCollide(player.pos,PLAYER_RADIUS,a.pos,a.radius())){
                    player.lives--; player.invTimer=2.5f;
                    a.active=false;
                    if(player.lives<=0)gameOver=true;
                    break;
                }
            }
            // Enemy bullet ↔ player
            for(auto& b:enemyBullets){
                if(!b.active)continue;
                if(circleCollide(player.pos,PLAYER_RADIUS,b.pos,BULLET_RADIUS)){
                    b.active=false;
                    player.lives--; player.invTimer=2.5f;
                    if(player.lives<=0)gameOver=true;
                    break;
                }
            }
        }
    }

    // ── Render ────────────────────────────────────────────
    void render(){
        glClearColor(0.03f,0.03f,0.10f,1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawStarfield();
        for(auto& a:aliens) a.draw();
        for(auto& b:bullets)b.draw();
        // Enemy bullets – cyan bolts
        for(auto& b:enemyBullets){
            if(!b.active)continue;
            setColor(0.f,0.9f,0.85f,0.9f);
            drawPoly(b.pos,BULLET_RADIUS,8);
            setColor(0.6f,1.f,1.f,0.45f);
            drawPoly(b.pos,BULLET_RADIUS+4.f,8);
        }
        // Engine glow goes behind ship body
        ship.drawGlow(player.pos,player.angle,gameTime);
        // Toon ship on top
        ship.draw(player.pos,player.angle,player.invTimer,gameTime);
        drawHUD();
    }

    void drawStarfield(){
        static std::vector<Vec2> stars;
        if(stars.empty()){
            srand(42);
            for(int i=0;i<120;i++)
                stars.push_back({randRange(-HALF_W,HALF_W),
                                 randRange(-HALF_H,HALF_H)});
            srand((unsigned)time(nullptr));
        }
        glPointSize(2.f);
        glBegin(GL_POINTS);
        for(auto& s:stars){
            float b=0.4f+0.4f*(s.x/HALF_W+1.f)*0.5f;
            glColor3f(b,b,b);
            glVertex2f(s.x,s.y);
        }
        glEnd();
        glPointSize(1.f);
    }

    void drawHUD(){
        float sx=-HALF_W+16, sy=HALF_H-30;
        // Segment font uses Y-down coords; flip Y axis for correct rendering
        glPushMatrix();
        glScalef(1.f,-1.f,1.f);
        setColor(0.5f,0.5f,0.6f);
        drawString(sx,-(sy+18.f),2.f,"SCORE");
        setColor(0.95f,0.95f,0.95f);
        drawString(sx,-sy,3.f,std::to_string(score));
        glPopMatrix();
        // Lives (small ship silhouettes) – drawn in normal world space
        for(int i=0;i<player.lives;i++){
            Vec2 lp={HALF_W-18.f-i*22.f, HALF_H-22.f};
            setColor(1.f,0.33f,0.f);
            float a=90.f*(float)M_PI/180.f;
            glBegin(GL_TRIANGLES);
            for(float da:{0.f,2.3f,-2.3f})
                glVertex2f(lp.x+cosf(a+da)*9.f,lp.y+sinf(a+da)*9.f);
            glEnd();
        }
    }

    void renderGameOver(){
        glClearColor(0.03f,0.03f,0.10f,1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawStarfield();
        // Flip Y for segment font
        glPushMatrix();
        glScalef(1.f,-1.f,1.f);
        setColor(1.f,0.33f,0.f);
        drawString(-80,-30,5.f,"GAME OVER");
        setColor(0.85f,0.85f,0.85f);
        std::string fs="SCORE  "+std::to_string(score);
        drawString(-(float)fs.size()*6,20,3.f,fs);
        setColor(0.5f,0.8f,1.f);
        drawString(-90,70,2.5f,"PRESS R TO RESTART");
        glPopMatrix();
    }

    void restart(){
        player=Player{};
        aliens.clear(); bullets.clear();
        score=0; normalKills=0;
        spawnTimer=0; spawnInterval=2.5f;
        gameOver=false; gameTime=0.0;
        enemyBullets.clear();
    }

    // ── Callbacks ─────────────────────────────────────────
    static void keyCallback(GLFWwindow* w,int key,int,int action,int){
        Game* g=static_cast<Game*>(glfwGetWindowUserPointer(w));
        bool dn=(action!=GLFW_RELEASE);
        if(key==GLFW_KEY_ESCAPE&&dn)glfwSetWindowShouldClose(w,GLFW_TRUE);
        if(g->gameOver){if(key==GLFW_KEY_R&&dn)g->restart();return;}
        switch(key){
            case GLFW_KEY_W:g->player.moveUp   =dn;break;
            case GLFW_KEY_S:g->player.moveDown =dn;break;
            case GLFW_KEY_A:g->player.moveLeft =dn;break;
            case GLFW_KEY_D:g->player.moveRight=dn;break;
        }
    }
    static void mouseButtonCallback(GLFWwindow* w,int btn,int action,int){
        Game* g=static_cast<Game*>(glfwGetWindowUserPointer(w));
        if(g->gameOver)return;
        if(btn==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS)
            g->bullets.push_back(g->player.fire());
    }
    static void cursorCallback(GLFWwindow* w,double xp,double yp){
        Game* g=static_cast<Game*>(glfwGetWindowUserPointer(w));
        g->mouseX=(float)xp-HALF_W;
        g->mouseY=HALF_H-(float)yp;
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main(){
    Game game;
    game.run();
    return 0;
}