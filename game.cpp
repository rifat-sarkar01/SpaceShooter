// ============================================================
//  game.cpp  –  Game loop, update, render, callbacks
// ============================================================
#include "game.h"

// ─────────────────────────────────────────────
//  Init
// ─────────────────────────────────────────────
bool Game::init(){
    if(!glfwInit()){ std::cerr << "GLFW init failed\n"; return false; }

    glfwWindowHint(GLFW_RESIZABLE,            GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    window = glfwCreateWindow(WINDOW_W, WINDOW_H,
                              "Space Shooter - Rage-Fang",
                              nullptr, nullptr);
    if(!window){ glfwTerminate(); return false; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        std::cerr << "GLEW init failed\n";
        return false;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window,         keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window,   cursorCallback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ship.init();               // load ship.png, build quad VAO
    AlienTextureCache::init(); // load normal_alien.png + boss_alien.png
    srand((unsigned)time(nullptr));
    return true;
}

// ─────────────────────────────────────────────
//  Orthographic projection
// ─────────────────────────────────────────────
void Game::setupProjection(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-HALF_W, HALF_W, -HALF_H, HALF_H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ─────────────────────────────────────────────
//  Main loop
// ─────────────────────────────────────────────
void Game::run(){
    if(!init()) return;
    setupProjection();

    double prev = glfwGetTime();
    while(!glfwWindowShouldClose(window)){
        double now = glfwGetTime();
        float  dt  = (float)(now - prev);
        prev = now;
        if(dt > 0.05f) dt = 0.05f;   // cap delta-time

        glfwPollEvents();

        if(!gameOver){ update(dt); render(); }
        else          { renderGameOver();     }

        glfwSwapBuffers(window);
    }

    ship.cleanup();               // free ship GPU resources
    AlienTextureCache::cleanup(); // free alien GPU resources
    glfwDestroyWindow(window);
    glfwTerminate();
}

// ─────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────
void Game::update(float dt){
    gameTime += dt;

    // Player
    player.aimAt(mouseX, mouseY);
    player.update(dt);

    // Player bullets
    for(auto& b : bullets) b.update(dt);
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [](const Projectile& p){ return !p.active; }),
        bullets.end());

    // Aliens – update + boss AI
    for(auto& a : aliens){
        a.update(dt);
        if(a.type == AlienType::BOSS && a.active){
            a.faceTarget(player.pos);
            if(a.shootTimer <= 0.f){
                a.shootTimer = BOSS_FIRE_RATE;
                enemyBullets.push_back(a.fireAt(player.pos));
            }
        }
    }
    aliens.erase(
        std::remove_if(aliens.begin(), aliens.end(),
            [](const Alien& a){ return !a.active; }),
        aliens.end());

    // Enemy bullets
    for(auto& b : enemyBullets) b.update(dt);
    enemyBullets.erase(
        std::remove_if(enemyBullets.begin(), enemyBullets.end(),
            [](const Projectile& p){ return !p.active; }),
        enemyBullets.end());

    // Spawn normal aliens (difficulty scales with score)
    spawnTimer -= dt;
    if(spawnTimer <= 0){
        spawnTimer = std::max(0.6f, spawnInterval - score * 0.003f);
        aliens.push_back(Alien::makeNormal());
    }

    // ── Collision: player bullet ↔ alien ─────────────────
    for(auto& b : bullets){
        if(!b.active) continue;
        for(auto& a : aliens){
            if(!a.active) continue;
            if(circleCollide(b.pos, BULLET_RADIUS, a.pos, a.radius())){
                b.active = false;
                a.hp--;
                if(a.hp <= 0){
                    a.active = false;
                    if(a.type == AlienType::NORMAL){
                        score += 10;
                        normalKills++;
                        if(normalKills % BOSS_SPAWN_EVERY == 0)
                            aliens.push_back(Alien::makeBoss());
                    } else {
                        score += 50;   // boss kill bonus
                    }
                }
                break;
            }
        }
    }

    // ── Collision: alien body ↔ player ───────────────────
    if(player.invTimer <= 0){
        for(auto& a : aliens){
            if(!a.active) continue;
            if(circleCollide(player.pos, PLAYER_RADIUS,
                             a.pos, a.radius())){
                player.lives--;
                player.invTimer = 2.5f;
                a.active = false;
                if(player.lives <= 0) gameOver = true;
                break;
            }
        }

        // ── Collision: enemy bullet ↔ player ─────────────
        for(auto& b : enemyBullets){
            if(!b.active) continue;
            if(circleCollide(player.pos, PLAYER_RADIUS,
                             b.pos, BULLET_RADIUS)){
                b.active = false;
                player.lives--;
                player.invTimer = 2.5f;
                if(player.lives <= 0) gameOver = true;
                break;
            }
        }
    }
}

// ─────────────────────────────────────────────
//  Render
// ─────────────────────────────────────────────
void Game::render(){
    glClearColor(0.03f, 0.03f, 0.10f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawStarfield();

    for(auto& a : aliens)  a.draw();
    for(auto& b : bullets) b.draw();

    // Enemy bullets – cyan bolts
    for(auto& b : enemyBullets){
        if(!b.active) continue;
        setColor(0.f, 0.9f, 0.85f, 0.9f);
        drawPoly(b.pos, BULLET_RADIUS, 8);
        setColor(0.6f, 1.f, 1.f, 0.45f);
        drawPoly(b.pos, BULLET_RADIUS + 4.f, 8);
    }

    // Draw the player ship (PNG-textured quad)
    ship.draw(player.pos, player.angle, player.invTimer, gameTime);

    drawHUD();
}

// ─────────────────────────────────────────────
//  Starfield
// ─────────────────────────────────────────────
void Game::drawStarfield(){
    static std::vector<Vec2> stars;
    if(stars.empty()){
        srand(42);
        for(int i = 0; i < 120; i++)
            stars.push_back({randRange(-HALF_W, HALF_W),
                             randRange(-HALF_H, HALF_H)});
        srand((unsigned)time(nullptr));
    }
    glPointSize(2.f);
    glBegin(GL_POINTS);
    for(auto& s : stars){
        float b = 0.4f + 0.4f * (s.x / HALF_W + 1.f) * 0.5f;
        glColor3f(b, b, b);
        glVertex2f(s.x, s.y);
    }
    glEnd();
    glPointSize(1.f);
}

// ─────────────────────────────────────────────
//  HUD
// ─────────────────────────────────────────────
void Game::drawHUD(){
    float sx = -HALF_W + 16, sy = HALF_H - 30;

    // Segment font uses Y-down coords; flip Y so text is upright
    glPushMatrix();
    glScalef(1.f, -1.f, 1.f);
    setColor(0.5f, 0.5f, 0.6f);
    drawString(sx, -(sy + 18.f), 2.f, "SCORE");
    setColor(0.95f, 0.95f, 0.95f);
    drawString(sx, -sy, 3.f, std::to_string(score));
    glPopMatrix();

    // Lives – small orange triangles (normal world space)
    for(int i = 0; i < player.lives; i++){
        Vec2 lp = {HALF_W - 18.f - i*22.f, HALF_H - 22.f};
        setColor(1.f, 0.33f, 0.f);
        float a = 90.f * (float)M_PI / 180.f;
        glBegin(GL_TRIANGLES);
        for(float da : {0.f, 2.3f, -2.3f})
            glVertex2f(lp.x + cosf(a+da)*9.f,
                       lp.y + sinf(a+da)*9.f);
        glEnd();
    }
}

// ─────────────────────────────────────────────
//  Game Over screen
// ─────────────────────────────────────────────
void Game::renderGameOver(){
    glClearColor(0.03f, 0.03f, 0.10f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawStarfield();

    glPushMatrix();
    glScalef(1.f, -1.f, 1.f);

    setColor(1.f, 0.33f, 0.f);
    drawString(-80, -30, 5.f, "GAME OVER");

    setColor(0.85f, 0.85f, 0.85f);
    std::string fs = "SCORE  " + std::to_string(score);
    drawString(-(float)fs.size() * 6, 20, 3.f, fs);

    setColor(0.5f, 0.8f, 1.f);
    drawString(-90, 70, 2.5f, "PRESS R TO RESTART");

    glPopMatrix();
}

// ─────────────────────────────────────────────
//  Restart
// ─────────────────────────────────────────────
void Game::restart(){
    player       = Player{};
    aliens.clear();
    bullets.clear();
    enemyBullets.clear();
    score        = 0;
    normalKills  = 0;
    spawnTimer   = 0.f;
    spawnInterval= 2.5f;
    gameOver     = false;
    gameTime     = 0.0;
}

// ─────────────────────────────────────────────
//  GLFW Callbacks
// ─────────────────────────────────────────────
void Game::keyCallback(GLFWwindow* w, int key, int /*scan*/,
                       int action, int /*mods*/)
{
    Game* g = static_cast<Game*>(glfwGetWindowUserPointer(w));
    bool  dn = (action != GLFW_RELEASE);

    if(key == GLFW_KEY_ESCAPE && dn)
        glfwSetWindowShouldClose(w, GLFW_TRUE);

    if(g->gameOver){
        if(key == GLFW_KEY_R && dn) g->restart();
        return;
    }

    switch(key){
        case GLFW_KEY_W: g->player.moveUp    = dn; break;
        case GLFW_KEY_S: g->player.moveDown  = dn; break;
        case GLFW_KEY_A: g->player.moveLeft  = dn; break;
        case GLFW_KEY_D: g->player.moveRight = dn; break;
    }
}

void Game::mouseButtonCallback(GLFWwindow* w, int button,
                                int action, int /*mods*/)
{
    Game* g = static_cast<Game*>(glfwGetWindowUserPointer(w));
    if(g->gameOver) return;
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        g->bullets.push_back(g->player.fire());
}

void Game::cursorCallback(GLFWwindow* w, double xpos, double ypos){
    Game* g = static_cast<Game*>(glfwGetWindowUserPointer(w));
    g->mouseX = (float)xpos - HALF_W;
    g->mouseY = HALF_H - (float)ypos;   // flip Y (GL is Y-up)
}
