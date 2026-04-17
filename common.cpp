// ============================================================
//  common.cpp  –  Shader compiler + segment font
// ============================================================
#include "common.h"

// ─────────────────────────────────────────────
//  Shader helpers
// ─────────────────────────────────────────────
GLuint compileShader(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok){ char b[512]; glGetShaderInfoLog(s,512,nullptr,b); std::cerr<<b<<"\n"; }
    return s;
}

GLuint buildProgram(const char* vs, const char* fs){
    GLuint v = compileShader(GL_VERTEX_SHADER,   vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ─────────────────────────────────────────────
//  Segment font  (Y-down glyph coords 0-4 x 0-6)
// ─────────────────────────────────────────────
struct Seg { int x1,y1,x2,y2; };

void drawChar(float px, float py, float sc, char c){
    std::vector<Seg> s;
    switch(toupper(c)){
        case '0': s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0}}; break;
        case '1': s={{2,0,2,6}}; break;
        case '2': s={{0,0,4,0},{4,0,4,3},{4,3,0,3},{0,3,0,6},{0,6,4,6}}; break;
        case '3': s={{0,0,4,0},{4,0,4,6},{0,6,4,6},{0,3,4,3}}; break;
        case '4': s={{0,0,0,3},{0,3,4,3},{4,0,4,6}}; break;
        case '5': s={{4,0,0,0},{0,0,0,3},{0,3,4,3},{4,3,4,6},{4,6,0,6}}; break;
        case '6': s={{4,0,0,0},{0,0,0,6},{0,6,4,6},{4,6,4,3},{4,3,0,3}}; break;
        case '7': s={{0,0,4,0},{4,0,4,6}}; break;
        case '8': s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0},{0,3,4,3}}; break;
        case '9': s={{0,3,4,3},{4,3,4,0},{4,0,0,0},{0,0,0,3},{4,3,4,6}}; break;
        case 'A': s={{0,6,2,0},{2,0,4,6},{1,3,3,3}}; break;
        case 'B': s={{0,0,0,6},{0,6,3,6},{3,6,4,5},{4,5,3,3},{3,3,0,3},{3,3,4,2},{4,2,3,0},{3,0,0,0}}; break;
        case 'C': s={{4,0,0,0},{0,0,0,6},{0,6,4,6}}; break;
        case 'D': s={{0,0,0,6},{0,6,3,6},{3,6,4,5},{4,5,4,1},{4,1,3,0},{3,0,0,0}}; break;
        case 'E': s={{4,0,0,0},{0,0,0,6},{0,6,4,6},{0,3,3,3}}; break;
        case 'F': s={{0,0,0,6},{0,0,4,0},{0,3,3,3}}; break;
        case 'G': s={{4,0,0,0},{0,0,0,6},{0,6,4,6},{4,6,4,3},{2,3,4,3}}; break;
        case 'H': s={{0,0,0,6},{4,0,4,6},{0,3,4,3}}; break;
        case 'I': s={{1,0,3,0},{2,0,2,6},{1,6,3,6}}; break;
        case 'J': s={{4,0,4,6},{4,6,0,6},{0,6,0,4}}; break;
        case 'K': s={{0,0,0,6},{0,3,4,0},{0,3,4,6}}; break;
        case 'L': s={{0,0,0,6},{0,6,4,6}}; break;
        case 'M': s={{0,6,0,0},{0,0,2,3},{2,3,4,0},{4,0,4,6}}; break;
        case 'N': s={{0,6,0,0},{0,0,4,6},{4,6,4,0}}; break;
        case 'O': s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0}}; break;
        case 'P': s={{0,0,0,6},{0,0,4,0},{4,0,4,3},{4,3,0,3}}; break;
        case 'Q': s={{0,0,4,0},{4,0,4,6},{4,6,0,6},{0,6,0,0},{2,4,4,6}}; break;
        case 'R': s={{0,0,0,6},{0,0,4,0},{4,0,4,3},{4,3,0,3},{0,3,4,6}}; break;
        case 'S': s={{4,0,0,0},{0,0,0,3},{0,3,4,3},{4,3,4,6},{4,6,0,6}}; break;
        case 'T': s={{0,0,4,0},{2,0,2,6}}; break;
        case 'U': s={{0,0,0,6},{0,6,4,6},{4,6,4,0}}; break;
        case 'V': s={{0,0,2,6},{2,6,4,0}}; break;
        case 'W': s={{0,0,1,6},{1,6,2,3},{2,3,3,6},{3,6,4,0}}; break;
        case 'X': s={{0,0,4,6},{4,0,0,6}}; break;
        case 'Y': s={{0,0,2,3},{4,0,2,3},{2,3,2,6}}; break;
        case 'Z': s={{0,0,4,0},{4,0,0,6},{0,6,4,6}}; break;
        case '-': s={{1,3,3,3}}; break;
        case ':': s={{2,1,2,2},{2,4,2,5}}; break;
        case '.': s={{2,5,2,6}}; break;
        case ' ': break;
        default:  break;
    }
    glLineWidth(2.f);
    glBegin(GL_LINES);
    for(auto& g : s){
        glVertex2f(px + g.x1*sc, py + g.y1*sc);
        glVertex2f(px + g.x2*sc, py + g.y2*sc);
    }
    glEnd();
    glLineWidth(1.f);
}

void drawString(float x, float y, float sc, const std::string& txt){
    float cx = x;
    for(char c : txt){ drawChar(cx, y, sc, c); cx += 6.f * sc; }
}
