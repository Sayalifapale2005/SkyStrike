#include <GL/freeglut.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

using namespace std;

// ============================================================
// WINDOW SETTINGS
// ============================================================

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// ============================================================
// GAME STATES
// ============================================================

enum GameState {
    START_SCREEN,
    PLAYING,
    PAUSED,
    GAME_OVER
};

GameState currentState = START_SCREEN;

// ============================================================
// GAME VARIABLES
// ============================================================

int score = 0;
int level = 1;
int lives = 3;

float playerHealth = 100.0f;
float enemyHealth = 100.0f;

// ============================================================
// PLAYER SETTINGS
// ============================================================

float playerX = 120.0f;
float playerY = WINDOW_HEIGHT / 2.0f;

float playerVelX = 0.0f;
float playerVelY = 0.0f;

const float PLAYER_SPEED = 7.0f;
const float PLAYER_WIDTH = 70.0f;
const float PLAYER_HEIGHT = 35.0f;

// ============================================================
// ENEMY SETTINGS
// ============================================================

float enemyX = 850.0f;
float enemyY = WINDOW_HEIGHT / 2.0f;

float enemySpeed = 2.0f;

const float ENEMY_WIDTH = 75.0f;
const float ENEMY_HEIGHT = 40.0f;

bool enemyActive = true;

float enemyFireTimer = 0.0f;
const float ENEMY_FIRE_INTERVAL = 80.0f;

// ============================================================
// BULLET
// ============================================================

struct Bullet {
    float x;
    float y;
    float vx;
    float vy;
    bool active;
    bool isPlayerBullet;
};

vector<Bullet> bullets;

const float BULLET_SPEED = 12.0f;
const float BULLET_SIZE = 8.0f;

// ============================================================
// OBSTACLE
// ============================================================

struct Obstacle {
    float x;
    float y;
    float size;
    float speed;
    bool active;
};

vector<Obstacle> obstacles;

// ============================================================
// EXPLOSION
// ============================================================

struct Explosion {
    float x;
    float y;
    float radius;
    float life;
    bool active;
};

vector<Explosion> explosions;

// ============================================================
// STARS
// ============================================================

struct Star {
    float x;
    float y;
    float speed;
    float size;
};

vector<Star> stars;

// ============================================================
// FIRING EFFECT
// ============================================================

float firingEffect = 0.0f;
int firingEffectTimer = 0;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

void initGame();
void resetGame();

void display();

void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);

void specialKeyboard(int key, int x, int y);
void specialKeyboardUp(int key, int x, int y);

void updateGame(int value);
void updateGameLogic();

void checkCollisions();

bool checkCollision(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
);

void spawnBullet(
    float x,
    float y,
    float vx,
    float vy,
    bool playerBullet
);

void spawnObstacle();
void spawnExplosion(float x, float y);

// Drawing functions
void drawStars();
void drawPlayer();
void drawEnemy();
void drawBullets();
void drawObstacles();
void drawExplosions();
void drawHUD();

void drawStartScreen();
void drawGameOverScreen();
void drawPauseScreen();

void reshape(int width, int height);

void drawText(
    float x,
    float y,
    const string& text,
    void* font = GLUT_BITMAP_HELVETICA_18
);

// ============================================================
// TEXT DRAWING
// ============================================================

void drawText(float x, float y, const string& text, void* font) {
    glRasterPos2f(x, y);

    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

// ============================================================
// INITIALIZE GAME
// ============================================================

void initGame() {

    srand(static_cast<unsigned int>(time(nullptr)));

    stars.clear();

    // Create stars
    for (int i = 0; i < 100; i++) {

        Star star;

        star.x = static_cast<float>(rand() % WINDOW_WIDTH);
        star.y = static_cast<float>(rand() % WINDOW_HEIGHT);
        star.speed = 0.5f + static_cast<float>(rand() % 30) / 10.0f;
        star.size = 1.0f + static_cast<float>(rand() % 3);

        stars.push_back(star);
    }

    obstacles.clear();

    // Create obstacles
    for (int i = 0; i < 8; i++) {

        Obstacle obstacle;

        obstacle.x = 300.0f + static_cast<float>(rand() % 700);
        obstacle.y = 80.0f + static_cast<float>(rand() % 600);
        obstacle.size = 15.0f + static_cast<float>(rand() % 20);
        obstacle.speed = 2.0f + static_cast<float>(rand() % 20) / 10.0f;
        obstacle.active = true;

        obstacles.push_back(obstacle);
    }

    bullets.clear();
    explosions.clear();

    resetGame();
}

// ============================================================
// RESET GAME
// ============================================================

void resetGame() {

    score = 0;
    level = 1;
    lives = 3;

    playerHealth = 100.0f;
    enemyHealth = 100.0f;

    playerX = 120.0f;
    playerY = WINDOW_HEIGHT / 2.0f;

    playerVelX = 0.0f;
    playerVelY = 0.0f;

    enemyX = 850.0f;
    enemyY = WINDOW_HEIGHT / 2.0f;

    enemySpeed = 2.0f;

    enemyActive = true;

    enemyFireTimer = 0.0f;

    firingEffect = 0.0f;
    firingEffectTimer = 0;

    bullets.clear();
    explosions.clear();

    // Reset obstacles
    for (size_t i = 0; i < obstacles.size(); i++) {

        obstacles[i].x =
            300.0f + static_cast<float>(rand() % 700);

        obstacles[i].y =
            80.0f + static_cast<float>(rand() % 600);

        obstacles[i].speed =
            2.0f + static_cast<float>(rand() % 20) / 10.0f;

        obstacles[i].active = true;
    }
}

// ============================================================
// DRAW STARS
// ============================================================

void drawStars() {

    glPointSize(2.0f);

    glBegin(GL_POINTS);

    for (size_t i = 0; i < stars.size(); i++) {

        glColor3f(1.0f, 1.0f, 1.0f);

        glVertex2f(
            stars[i].x,
            stars[i].y
        );
    }

    glEnd();
}

// ============================================================
// DRAW PLAYER FIGHTER JET
// ============================================================

void drawPlayer() {

    glPushMatrix();

    glTranslatef(playerX, playerY, 0.0f);

    // Main body
    glColor3f(0.15f, 0.55f, 1.0f);

    glBegin(GL_POLYGON);

    glVertex2f(-35.0f, 0.0f);
    glVertex2f(-15.0f, 15.0f);
    glVertex2f(25.0f, 12.0f);
    glVertex2f(35.0f, 0.0f);
    glVertex2f(25.0f, -12.0f);
    glVertex2f(-15.0f, -15.0f);

    glEnd();

    // Top wing
    glColor3f(0.1f, 0.35f, 0.85f);

    glBegin(GL_TRIANGLES);

    glVertex2f(-5.0f, 5.0f);
    glVertex2f(10.0f, 40.0f);
    glVertex2f(20.0f, 8.0f);

    glEnd();

    // Bottom wing
    glBegin(GL_TRIANGLES);

    glVertex2f(-5.0f, -5.0f);
    glVertex2f(10.0f, -40.0f);
    glVertex2f(20.0f, -8.0f);

    glEnd();

    // Cockpit
    glColor3f(0.7f, 0.9f, 1.0f);

    glBegin(GL_POLYGON);

    glVertex2f(0.0f, 6.0f);
    glVertex2f(15.0f, 5.0f);
    glVertex2f(22.0f, 0.0f);
    glVertex2f(15.0f, -5.0f);
    glVertex2f(0.0f, -6.0f);

    glEnd();

    // Engine fire
    if (firingEffectTimer > 0) {

        glColor3f(1.0f, 0.5f, 0.0f);

        glBegin(GL_TRIANGLES);

        glVertex2f(-30.0f, 0.0f);
        glVertex2f(-50.0f, 8.0f);
        glVertex2f(-50.0f, -8.0f);

        glEnd();

        glColor3f(1.0f, 1.0f, 0.0f);

        glBegin(GL_TRIANGLES);

        glVertex2f(-32.0f, 0.0f);
        glVertex2f(-44.0f, 4.0f);
        glVertex2f(-44.0f, -4.0f);

        glEnd();
    }

    glPopMatrix();
}

// ============================================================
// DRAW ENEMY
// ============================================================

void drawEnemy() {

    if (!enemyActive)
        return;

    glPushMatrix();

    glTranslatef(enemyX, enemyY, 0.0f);

    // Enemy body
    glColor3f(0.9f, 0.15f, 0.15f);

    glBegin(GL_POLYGON);

    glVertex2f(35.0f, 0.0f);
    glVertex2f(15.0f, 18.0f);
    glVertex2f(-25.0f, 14.0f);
    glVertex2f(-35.0f, 0.0f);
    glVertex2f(-25.0f, -14.0f);
    glVertex2f(15.0f, -18.0f);

    glEnd();

    // Top wing
    glColor3f(0.65f, 0.05f, 0.05f);

    glBegin(GL_TRIANGLES);

    glVertex2f(5.0f, 5.0f);
    glVertex2f(-10.0f, 42.0f);
    glVertex2f(-20.0f, 8.0f);

    glEnd();

    // Bottom wing
    glBegin(GL_TRIANGLES);

    glVertex2f(5.0f, -5.0f);
    glVertex2f(-10.0f, -42.0f);
    glVertex2f(-20.0f, -8.0f);

    glEnd();

    // Cockpit
    glColor3f(1.0f, 0.7f, 0.7f);

    glBegin(GL_POLYGON);

    glVertex2f(0.0f, 7.0f);
    glVertex2f(-15.0f, 5.0f);
    glVertex2f(-22.0f, 0.0f);
    glVertex2f(-15.0f, -5.0f);
    glVertex2f(0.0f, -7.0f);

    glEnd();

    glPopMatrix();

    // Enemy health bar
    float barWidth = 80.0f;
    float healthWidth =
        barWidth * (enemyHealth / 100.0f);

    glColor3f(0.2f, 0.2f, 0.2f);

    glBegin(GL_QUADS);

    glVertex2f(
        enemyX - barWidth / 2,
        enemyY + 32
    );

    glVertex2f(
        enemyX + barWidth / 2,
        enemyY + 32
    );

    glVertex2f(
        enemyX + barWidth / 2,
        enemyY + 37
    );

    glVertex2f(
        enemyX - barWidth / 2,
        enemyY + 37
    );

    glEnd();

    if (enemyHealth > 0) {

        glColor3f(1.0f, 0.0f, 0.0f);

        glBegin(GL_QUADS);

        glVertex2f(
            enemyX - barWidth / 2,
            enemyY + 32
        );

        glVertex2f(
            enemyX - barWidth / 2 + healthWidth,
            enemyY + 32
        );

        glVertex2f(
            enemyX - barWidth / 2 + healthWidth,
            enemyY + 37
        );

        glVertex2f(
            enemyX - barWidth / 2,
            enemyY + 37
        );

        glEnd();
    }
}

// ============================================================
// DRAW BULLETS
// ============================================================

void drawBullets() {

    for (size_t i = 0; i < bullets.size(); i++) {

        if (!bullets[i].active)
            continue;

        if (bullets[i].isPlayerBullet) {

            glColor3f(0.0f, 1.0f, 1.0f);

        } else {

            glColor3f(1.0f, 0.2f, 0.2f);
        }

        glBegin(GL_QUADS);

        glVertex2f(
            bullets[i].x - 5,
            bullets[i].y - 3
        );

        glVertex2f(
            bullets[i].x + 5,
            bullets[i].y - 3
        );

        glVertex2f(
            bullets[i].x + 5,
            bullets[i].y + 3
        );

        glVertex2f(
            bullets[i].x - 5,
            bullets[i].y + 3
        );

        glEnd();
    }
}

// ============================================================
// DRAW OBSTACLES
// ============================================================

void drawObstacles() {

    for (size_t i = 0; i < obstacles.size(); i++) {

        if (!obstacles[i].active)
            continue;

        glPushMatrix();

        glTranslatef(
            obstacles[i].x,
            obstacles[i].y,
            0.0f
        );

        glColor3f(0.45f, 0.45f, 0.45f);

        glBegin(GL_POLYGON);

        for (int j = 0; j < 8; j++) {

            float angle =
                j * 3.14159f / 4.0f;

            float radius =
                obstacles[i].size;

            glVertex2f(
                cos(angle) * radius,
                sin(angle) * radius
            );
        }

        glEnd();

        glPopMatrix();
    }
}

// ============================================================
// DRAW EXPLOSIONS
// ============================================================

void drawExplosions() {

    for (size_t i = 0; i < explosions.size(); i++) {

        if (!explosions[i].active)
            continue;

        float alpha =
            explosions[i].life / 30.0f;

        glColor4f(
            1.0f,
            0.5f,
            0.0f,
            alpha
        );

        glBegin(GL_TRIANGLE_FAN);

        glVertex2f(
            explosions[i].x,
            explosions[i].y
        );

        for (int j = 0; j <= 20; j++) {

            float angle =
                j * 2.0f * 3.14159f / 20.0f;

            glVertex2f(
                explosions[i].x +
                    cos(angle) * explosions[i].radius,

                explosions[i].y +
                    sin(angle) * explosions[i].radius
            );
        }

        glEnd();
    }
}

// ============================================================
// DRAW HUD
// ============================================================

void drawHUD() {

    glColor3f(1.0f, 1.0f, 1.0f);

    string scoreText =
        "Score: " + to_string(score);

    string levelText =
        "Level: " + to_string(level);

    string livesText =
        "Lives: " + to_string(lives);

    string healthText =
        "Health: " +
        to_string(static_cast<int>(playerHealth));

    drawText(
        20,
        WINDOW_HEIGHT - 30,
        scoreText
    );

    drawText(
        180,
        WINDOW_HEIGHT - 30,
        levelText
    );

    drawText(
        300,
        WINDOW_HEIGHT - 30,
        livesText
    );

    drawText(
        420,
        WINDOW_HEIGHT - 30,
        healthText
    );

    // Player health bar
    float barX = 600.0f;
    float barY = WINDOW_HEIGHT - 35.0f;
    float barWidth = 250.0f;
    float barHeight = 18.0f;

    glColor3f(0.25f, 0.25f, 0.25f);

    glBegin(GL_QUADS);

    glVertex2f(barX, barY);
    glVertex2f(barX + barWidth, barY);
    glVertex2f(barX + barWidth, barY + barHeight);
    glVertex2f(barX, barY + barHeight);

    glEnd();

    if (playerHealth > 0) {

        float healthWidth =
            barWidth * (playerHealth / 100.0f);

        glColor3f(0.0f, 1.0f, 0.0f);

        glBegin(GL_QUADS);

        glVertex2f(barX, barY);

        glVertex2f(
            barX + healthWidth,
            barY
        );

        glVertex2f(
            barX + healthWidth,
            barY + barHeight
        );

        glVertex2f(
            barX,
            barY + barHeight
        );

        glEnd();
    }
}

// ============================================================
// START SCREEN
// ============================================================

void drawStartScreen() {

    glColor3f(0.0f, 0.15f, 0.35f);

    glBegin(GL_QUADS);

    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);

    glEnd();

    glColor3f(0.2f, 0.8f, 1.0f);

    drawText(
        330,
        550,
        "2D FIGHTER JET",
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    drawText(
        390,
        510,
        "SHOOTING GAME",
        GLUT_BITMAP_HELVETICA_18
    );

    glColor3f(1.0f, 1.0f, 1.0f);

    drawText(
        350,
        420,
        "Press SPACE to Start"
    );

    drawText(
        350,
        380,
        "W A S D / Arrow Keys - Move"
    );

    drawText(
        350,
        340,
        "SPACE - Shoot"
    );

    drawText(
        350,
        300,
        "P - Pause"
    );

    drawText(
        350,
        260,
        "ESC - Exit"
    );
}

// ============================================================
// GAME OVER SCREEN
// ============================================================

void drawGameOverScreen() {

    glColor4f(
        0.0f,
        0.0f,
        0.0f,
        0.7f
    );

    glBegin(GL_QUADS);

    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );
    glVertex2f(
        0,
        WINDOW_HEIGHT
    );

    glEnd();

    glColor3f(1.0f, 0.1f, 0.1f);

    drawText(
        410,
        480,
        "GAME OVER",
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    glColor3f(1.0f, 1.0f, 1.0f);

    drawText(
        420,
        420,
        "Score: " + to_string(score)
    );

    drawText(
        360,
        370,
        "Press SPACE to Restart"
    );

    drawText(
        420,
        330,
        "Press ESC to Exit"
    );
}

// ============================================================
// PAUSE SCREEN
// ============================================================

void drawPauseScreen() {

    glColor4f(
        0.0f,
        0.0f,
        0.0f,
        0.6f
    );

    glBegin(GL_QUADS);

    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );
    glVertex2f(
        0,
        WINDOW_HEIGHT
    );

    glEnd();

    glColor3f(1.0f, 1.0f, 0.0f);

    drawText(
        450,
        420,
        "PAUSED",
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    drawText(
        380,
        370,
        "Press P to Resume"
    );
}

// ============================================================
// COLLISION DETECTION
// ============================================================

bool checkCollision(
    float x1,
    float y1,
    float w1,
    float h1,
    float x2,
    float y2,
    float w2,
    float h2
) {

    return (
        x1 - w1 / 2 < x2 + w2 / 2 &&
        x1 + w1 / 2 > x2 - w2 / 2 &&
        y1 - h1 / 2 < y2 + h2 / 2 &&
        y1 + h1 / 2 > y2 - h2 / 2
    );
}

// ============================================================
// SPAWN BULLET
// ============================================================

void spawnBullet(
    float x,
    float y,
    float vx,
    float vy,
    bool playerBullet
) {

    Bullet bullet;

    bullet.x = x;
    bullet.y = y;
    bullet.vx = vx;
    bullet.vy = vy;
    bullet.active = true;
    bullet.isPlayerBullet = playerBullet;

    bullets.push_back(bullet);
}

// ============================================================
// SPAWN OBSTACLE
// ============================================================

void spawnObstacle() {

    for (size_t i = 0; i < obstacles.size(); i++) {

        if (!obstacles[i].active) {

            obstacles[i].x =
                WINDOW_WIDTH +
                static_cast<float>(rand() % 300);

            obstacles[i].y =
                70.0f +
                static_cast<float>(
                    rand() % (WINDOW_HEIGHT - 140)
                );

            obstacles[i].size =
                15.0f +
                static_cast<float>(rand() % 20);

            obstacles[i].speed =
                2.0f +
                static_cast<float>(rand() % 20) / 10.0f;

            obstacles[i].active = true;

            return;
        }
    }
}

// ============================================================
// SPAWN EXPLOSION
// ============================================================

void spawnExplosion(float x, float y) {

    Explosion explosion;

    explosion.x = x;
    explosion.y = y;
    explosion.radius = 10.0f;
    explosion.life = 30.0f;
    explosion.active = true;

    explosions.push_back(explosion);
}

// ============================================================
// COLLISION CHECKS
// ============================================================

void checkCollisions() {

    // --------------------------------------------------------
    // PLAYER BULLETS -> ENEMY
    // --------------------------------------------------------

    if (enemyActive) {

        for (size_t i = 0; i < bullets.size(); i++) {

            if (!bullets[i].active)
                continue;

            if (!bullets[i].isPlayerBullet)
                continue;

            if (checkCollision(
                    bullets[i].x,
                    bullets[i].y,
                    BULLET_SIZE,
                    BULLET_SIZE,
                    enemyX,
                    enemyY,
                    ENEMY_WIDTH,
                    ENEMY_HEIGHT
                )) {

                bullets[i].active = false;

                enemyHealth -= 10.0f;

                score += 10;

                spawnExplosion(
                    bullets[i].x,
                    bullets[i].y
                );

                if (enemyHealth <= 0.0f) {

                    enemyHealth = 0.0f;

                    enemyActive = false;

                    score += 100;

                    spawnExplosion(
                        enemyX,
                        enemyY
                    );
                }
            }
        }
    }

    // --------------------------------------------------------
    // ENEMY BULLETS -> PLAYER
    // --------------------------------------------------------

    for (size_t i = 0; i < bullets.size(); i++) {

        if (!bullets[i].active)
            continue;

        if (bullets[i].isPlayerBullet)
            continue;

        if (checkCollision(
                bullets[i].x,
                bullets[i].y,
                BULLET_SIZE,
                BULLET_SIZE,
                playerX,
                playerY,
                PLAYER_WIDTH,
                PLAYER_HEIGHT
            )) {

            bullets[i].active = false;

            playerHealth -= 10.0f;

            spawnExplosion(
                playerX,
                playerY
            );

            if (playerHealth <= 0.0f) {

                playerHealth = 0.0f;

                lives--;

                if (lives > 0) {

                    playerHealth = 100.0f;

                    playerX = 120.0f;

                    playerY =
                        WINDOW_HEIGHT / 2.0f;

                } else {

                    currentState = GAME_OVER;
                }
            }
        }
    }

    // --------------------------------------------------------
    // PLAYER -> OBSTACLE
    // --------------------------------------------------------

    for (size_t i = 0; i < obstacles.size(); i++) {

        if (!obstacles[i].active)
            continue;

        float size =
            obstacles[i].size * 2.0f;

        if (checkCollision(
                playerX,
                playerY,
                PLAYER_WIDTH,
                PLAYER_HEIGHT,
                obstacles[i].x,
                obstacles[i].y,
                size,
                size
            )) {

            obstacles[i].active = false;

            playerHealth -= 20.0f;

            spawnExplosion(
                obstacles[i].x,
                obstacles[i].y
            );

            if (playerHealth <= 0.0f) {

                playerHealth = 0.0f;

                lives--;

                if (lives > 0) {

                    playerHealth = 100.0f;

                    playerX = 120.0f;

                    playerY =
                        WINDOW_HEIGHT / 2.0f;

                } else {

                    currentState = GAME_OVER;
                }
            }
        }
    }

    // --------------------------------------------------------
    // PLAYER -> ENEMY
    // --------------------------------------------------------

    if (enemyActive) {

        if (checkCollision(
                playerX,
                playerY,
                PLAYER_WIDTH,
                PLAYER_HEIGHT,
                enemyX,
                enemyY,
                ENEMY_WIDTH,
                ENEMY_HEIGHT
            )) {

            enemyActive = false;

            playerHealth -= 30.0f;

            score += 20;

            spawnExplosion(
                enemyX,
                enemyY
            );

            if (playerHealth <= 0.0f) {

                playerHealth = 0.0f;

                lives--;

                if (lives > 0) {

                    playerHealth = 100.0f;

                    playerX = 120.0f;

                    playerY =
                        WINDOW_HEIGHT / 2.0f;

                } else {

                    currentState = GAME_OVER;
                }
            }
        }
    }
}

// ============================================================
// GAME LOGIC
// ============================================================

void updateGameLogic() {

    // --------------------------------------------------------
    // PLAYER MOVEMENT
    // --------------------------------------------------------

    playerX += playerVelX;
    playerY += playerVelY;

    // Keep player inside screen
    if (playerX < PLAYER_WIDTH / 2.0f)
        playerX = PLAYER_WIDTH / 2.0f;

    if (playerX >
        WINDOW_WIDTH - PLAYER_WIDTH / 2.0f)
        playerX =
            WINDOW_WIDTH -
            PLAYER_WIDTH / 2.0f;

    if (playerY < PLAYER_HEIGHT / 2.0f)
        playerY = PLAYER_HEIGHT / 2.0f;

    if (playerY >
        WINDOW_HEIGHT -
        PLAYER_HEIGHT / 2.0f)
        playerY =
            WINDOW_HEIGHT -
            PLAYER_HEIGHT / 2.0f;

    // --------------------------------------------------------
    // FIRING EFFECT
    // --------------------------------------------------------

    if (firingEffectTimer > 0) {

        firingEffectTimer--;

        firingEffect -= 0.5f;

        if (firingEffect < 0.0f)
            firingEffect = 0.0f;
    }

    // --------------------------------------------------------
    // BULLETS
    // --------------------------------------------------------

    for (size_t i = 0; i < bullets.size(); i++) {

        if (!bullets[i].active)
            continue;

        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;

        if (
            bullets[i].x < -50.0f ||
            bullets[i].x > WINDOW_WIDTH + 50.0f ||
            bullets[i].y < -50.0f ||
            bullets[i].y > WINDOW_HEIGHT + 50.0f
        ) {

            bullets[i].active = false;
        }
    }

    // --------------------------------------------------------
    // ENEMY MOVEMENT
    // --------------------------------------------------------

    if (enemyActive) {

        enemyY += enemySpeed;

        if (
            enemyY >
            WINDOW_HEIGHT - 60.0f
        ) {

            enemyY =
                WINDOW_HEIGHT - 60.0f;

            enemySpeed = -enemySpeed;
        }

        if (enemyY < 60.0f) {

            enemyY = 60.0f;

            enemySpeed = -enemySpeed;
        }

        // Enemy shooting
        enemyFireTimer++;

        float fireInterval =
            max(
                30.0f,
                ENEMY_FIRE_INTERVAL -
                level * 5.0f
            );

        if (enemyFireTimer >= fireInterval) {

            enemyFireTimer = 0.0f;

            spawnBullet(
                enemyX - 40.0f,
                enemyY,
                -BULLET_SPEED * 0.8f,
                0.0f,
                false
            );
        }
    }

    // --------------------------------------------------------
    // RESPAWN ENEMY
    // --------------------------------------------------------

    if (!enemyActive) {

        static int enemyRespawnTimer = 0;

        enemyRespawnTimer++;

        if (enemyRespawnTimer > 120) {

            enemyRespawnTimer = 0;

            enemyActive = true;

            enemyHealth = 100.0f;

            enemyX =
                WINDOW_WIDTH - 150.0f;

            enemyY =
                100.0f +
                static_cast<float>(
                    rand() % (WINDOW_HEIGHT - 200)
                );
        }
    }

    // --------------------------------------------------------
    // OBSTACLES
    // --------------------------------------------------------

    for (size_t i = 0; i < obstacles.size(); i++) {

        if (!obstacles[i].active)
            continue;

        obstacles[i].x -=
            obstacles[i].speed;

        if (obstacles[i].x <
            -obstacles[i].size * 2.0f) {

            obstacles[i].active = false;

            spawnObstacle();
        }
    }

    // --------------------------------------------------------
    // EXPLOSIONS
    // --------------------------------------------------------

    for (size_t i = 0; i < explosions.size(); i++) {

        if (!explosions[i].active)
            continue;

        explosions[i].radius += 2.0f;

        explosions[i].life -= 1.0f;

        if (explosions[i].life <= 0.0f) {

            explosions[i].active = false;
        }
    }

    // --------------------------------------------------------
    // STARS
    // --------------------------------------------------------

    for (size_t i = 0; i < stars.size(); i++) {

        stars[i].x -= stars[i].speed;

        if (stars[i].x < 0.0f) {

            stars[i].x =
                static_cast<float>(WINDOW_WIDTH);

            stars[i].y =
                static_cast<float>(
                    rand() % WINDOW_HEIGHT
                );
        }
    }

    // --------------------------------------------------------
    // COLLISIONS
    // --------------------------------------------------------

    checkCollisions();

    // --------------------------------------------------------
    // LEVEL
    // --------------------------------------------------------

    level = 1 + score / 100;

    if (level > 10)
        level = 10;
}

// ============================================================
// DISPLAY
// ============================================================

void display() {

    glClear(
        GL_COLOR_BUFFER_BIT
    );

    glLoadIdentity();

    if (currentState == START_SCREEN) {

        drawStartScreen();

        glutSwapBuffers();

        return;
    }

    // Background
    glColor3f(
        0.01f,
        0.02f,
        0.08f
    );

    glBegin(GL_QUADS);

    glVertex2f(0, 0);
    glVertex2f(
        WINDOW_WIDTH,
        0
    );

    glVertex2f(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    glVertex2f(
        0,
        WINDOW_HEIGHT
    );

    glEnd();

    // Game objects
    drawStars();

    drawObstacles();

    drawPlayer();

    drawEnemy();

    drawBullets();

    drawExplosions();

    drawHUD();

    if (currentState == PAUSED) {

        drawPauseScreen();
    }

    if (currentState == GAME_OVER) {

        drawGameOverScreen();
    }

    glutSwapBuffers();
}

// ============================================================
// KEYBOARD
// ============================================================

void keyboard(
    unsigned char key,
    int x,
    int y
) {

    switch (key) {

        // ----------------------------------------------------
        // SPACE
        // ----------------------------------------------------

        case ' ':

            if (
                currentState ==
                START_SCREEN
            ) {

                resetGame();

                currentState =
                    PLAYING;
            }

            else if (
                currentState ==
                GAME_OVER
            ) {

                resetGame();

                currentState =
                    PLAYING;
            }

            else if (
                currentState ==
                PLAYING
            ) {

                // Player shoots right
                spawnBullet(
                    playerX +
                        PLAYER_WIDTH / 2.0f,

                    playerY,

                    BULLET_SPEED,

                    0.0f,

                    true
                );

                firingEffectTimer = 8;

                firingEffect = 5.0f;
            }

            break;

        // ----------------------------------------------------
        // PAUSE
        // ----------------------------------------------------

        case 'p':
        case 'P':

            if (
                currentState ==
                PLAYING
            ) {

                currentState =
                    PAUSED;
            }

            else if (
                currentState ==
                PAUSED
            ) {

                currentState =
                    PLAYING;
            }

            break;

        // ----------------------------------------------------
        // UP
        // ----------------------------------------------------

        case 'w':
        case 'W':

            if (
                currentState ==
                PLAYING
            ) {

                playerVelY =
                    PLAYER_SPEED;
            }

            break;

        // ----------------------------------------------------
        // DOWN
        // ----------------------------------------------------

        case 's':
        case 'S':

            if (
                currentState ==
                PLAYING
            ) {

                playerVelY =
                    -PLAYER_SPEED;
            }

            break;

        // ----------------------------------------------------
        // LEFT
        // ----------------------------------------------------

        case 'a':
        case 'A':

            if (
                currentState ==
                PLAYING
            ) {

                playerVelX =
                    -PLAYER_SPEED;
            }

            break;

        // ----------------------------------------------------
        // RIGHT
        // ----------------------------------------------------

        case 'd':
        case 'D':

            if (
                currentState ==
                PLAYING
            ) {

                playerVelX =
                    PLAYER_SPEED;
            }

            break;

        // ----------------------------------------------------
        // ESC
        // ----------------------------------------------------

        case 27:

            exit(0);

            break;
    }
}

// ============================================================
// KEYBOARD RELEASE
// ============================================================

void keyboardUp(
    unsigned char key,
    int x,
    int y
) {

    switch (key) {

        case 'w':
        case 'W':
        case 's':
        case 'S':

            playerVelY = 0.0f;

            break;

        case 'a':
        case 'A':
        case 'd':
        case 'D':

            playerVelX = 0.0f;

            break;
    }
}

// ============================================================
// SPECIAL KEYBOARD - ARROW KEYS
// ============================================================

void specialKeyboard(
    int key,
    int x,
    int y
) {

    if (
        currentState !=
        PLAYING
    )
        return;

    switch (key) {

        case GLUT_KEY_UP:

            playerVelY =
                PLAYER_SPEED;

            break;

        case GLUT_KEY_DOWN:

            playerVelY =
                -PLAYER_SPEED;

            break;

        case GLUT_KEY_LEFT:

            playerVelX =
                -PLAYER_SPEED;

            break;

        case GLUT_KEY_RIGHT:

            playerVelX =
                PLAYER_SPEED;

            break;
    }
}

// ============================================================
// SPECIAL KEY RELEASE
// ============================================================

void specialKeyboardUp(
    int key,
    int x,
    int y
) {

    switch (key) {

        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:

            playerVelY = 0.0f;

            break;

        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:

            playerVelX = 0.0f;

            break;
    }
}

// ============================================================
// GAME TIMER
// ============================================================

void updateGame(int value) {

    if (
        currentState ==
        PLAYING
    ) {

        updateGameLogic();
    }

    glutPostRedisplay();

    glutTimerFunc(
        16,
        updateGame,
        0
    );
}

// ============================================================
// RESHAPE
// ============================================================

void reshape(
    int width,
    int height
) {

    glViewport(
        0,
        0,
        width,
        height
    );

    glMatrixMode(
        GL_PROJECTION
    );

    glLoadIdentity();

    glOrtho(
        0,
        WINDOW_WIDTH,
        0,
        WINDOW_HEIGHT,
        -1,
        1
    );

    glMatrixMode(
        GL_MODELVIEW
    );

    glLoadIdentity();
}

// ============================================================
// MAIN
// ============================================================

int main(
    int argc,
    char** argv
) {

    glutInit(
        &argc,
        argv
    );

    glutInitDisplayMode(
        GLUT_DOUBLE |
        GLUT_RGB |
        GLUT_ALPHA
    );

    glutInitWindowSize(
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    glutInitWindowPosition(
        100,
        50
    );

    glutCreateWindow(
        "2D Fighter Jet Shooting Game"
    );

    // Projection
    glMatrixMode(
        GL_PROJECTION
    );

    glLoadIdentity();

    glOrtho(
        0,
        WINDOW_WIDTH,
        0,
        WINDOW_HEIGHT,
        -1,
        1
    );

    // Model view
    glMatrixMode(
        GL_MODELVIEW
    );

    glLoadIdentity();

    // Transparency
    glEnable(GL_BLEND);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    // Initialize game
    initGame();

    // Callbacks
    glutDisplayFunc(display);

    glutKeyboardFunc(keyboard);

    glutKeyboardUpFunc(
        keyboardUp
    );

    glutSpecialFunc(
        specialKeyboard
    );

    glutSpecialUpFunc(
        specialKeyboardUp
    );

    glutReshapeFunc(
        reshape
    );

    // Timer
    glutTimerFunc(
        16,
        updateGame,
        0
    );

    // Start game loop
    glutMainLoop();

    return 0;
}