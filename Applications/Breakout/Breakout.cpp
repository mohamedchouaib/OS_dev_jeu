#include "Breakout.h"
#include <drivers/vga.h>
#include <drivers/EcranBochs.h>
#include <drivers/PortSerie.h>
#include <sextant/sprite.h>

extern EcranBochs* globalVGA;
extern ui8_t palette_vga[256][3];

static const ui16_t SCREEN_WIDTH = 640;
static const ui16_t SCREEN_HEIGHT = 400;
static const ui8_t COLOR_PLAYER1 = 14;   // Yellow
static const ui8_t COLOR_PLAYER2 = 11;   // Cyan
static const ui8_t COLOR_BRICK_GREEN = 10;
static const ui8_t COLOR_BRICK_BLUE = 9;
static const ui8_t COLOR_BRICK_RED = 12;
static const ui8_t COLOR_POWERUP_ICON = 15;
static const int BALL_SPEED_PERIOD = 6;
static const int PADDLE_SPEED_PERIOD = 6;
static const int BASE_BALL_SPEED_Y = 2;
static const int MIN_BALL_SPEED_X = 1;
static const int MAX_BALL_SPEED_X = 3;
static const ui16_t BALL_DEFAULT_SIZE = 12;
static const ui16_t PADDLE1_Y = SCREEN_HEIGHT - 18;
static const ui16_t PADDLE2_Y = SCREEN_HEIGHT - 36;

struct Glyph { char c; unsigned char rows[5]; };

static const Glyph FONT_GLYPHS[] = {
    {'0', {0xE, 0xA, 0xA, 0xA, 0xE}}, // 1110 etc
    {'1', {0x4, 0xC, 0x4, 0x4, 0xE}},
    {'2', {0xE, 0x2, 0xE, 0x8, 0xE}},
    {'3', {0xE, 0x2, 0x6, 0x2, 0xE}},
    {'4', {0xA, 0xA, 0xE, 0x2, 0x2}},
    {'5', {0xE, 0x8, 0xE, 0x2, 0xE}},
    {'6', {0xE, 0x8, 0xE, 0xA, 0xE}},
    {'7', {0xE, 0x2, 0x4, 0x4, 0x4}},
    {'8', {0xE, 0xA, 0xE, 0xA, 0xE}},
    {'9', {0xE, 0xA, 0xE, 0x2, 0xE}},
    {'P', {0xE, 0xA, 0xE, 0x8, 0x8}},
    {'W', {0xA, 0xA, 0xA, 0xE, 0xA}},
    {'I', {0xE, 0x4, 0x4, 0x4, 0xE}},
    {'N', {0xA, 0xE, 0xE, 0xE, 0xA}},
    {'E', {0xE, 0x8, 0xE, 0x8, 0xE}},
    {'R', {0xE, 0xA, 0xE, 0xA, 0xA}},
    {' ', {0x0, 0x0, 0x0, 0x0, 0x0}},
    {':', {0x0, 0x4, 0x0, 0x4, 0x0}},
    {'-', {0x0, 0x0, 0xE, 0x0, 0x0}}
};

static const unsigned char* glyphFor(char c) {
    for (unsigned int i = 0; i < sizeof(FONT_GLYPHS)/sizeof(Glyph); ++i) {
        if (FONT_GLYPHS[i].c == c) {
            return FONT_GLYPHS[i].rows;
        }
    }
    return FONT_GLYPHS[sizeof(FONT_GLYPHS)/sizeof(Glyph) - 2].rows; // space
}

static void drawChar(EcranBochs* vga, int x, int y, char c, ui8_t color) {
    const unsigned char* glyph = glyphFor(c);
    for (int row = 0; row < 5; ++row) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < 4; ++col) {
            if (bits & (1 << (3 - col))) {
                vga->paint(x + col, y + row, color);
            }
        }
    }
}

static void drawText(EcranBochs* vga, int x, int y, const char* text, ui8_t color) {
    if (!text) return;
    int cursor = 0;
    while (text[cursor] != '\0') {
        drawChar(vga, x + cursor * 5, y, text[cursor], color);
        cursor++;
    }
}

static inline si32_t clampHorizontalSpeed(si32_t value) {
    if (value > MAX_BALL_SPEED_X) return MAX_BALL_SPEED_X;
    if (value < -MAX_BALL_SPEED_X) return -MAX_BALL_SPEED_X;
    if (value > 0 && value < MIN_BALL_SPEED_X) return MIN_BALL_SPEED_X;
    if (value < 0 && value > -MIN_BALL_SPEED_X) return -MIN_BALL_SPEED_X;
    return value;
}

static void intToString(int value, char* buffer, int bufferSize) {
    if (!buffer || bufferSize <= 1) return;
    int i = 0;
    bool neg = value < 0;
    int tmp = neg ? -value : value;
    if (tmp == 0 && i < bufferSize - 1) {
        buffer[i++] = '0';
    }
    while (tmp > 0 && i < bufferSize - 1) {
        buffer[i++] = '0' + (tmp % 10);
        tmp /= 10;
    }
    if (neg && i < bufferSize - 1) {
        buffer[i++] = '-';
    }
    for (int j = 0; j < i / 2; ++j) {
        char c = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = c;
    }
    buffer[i] = '\0';
}

static void drawScoreLine(EcranBochs* vga, int x, int y, const char* label, int score, ui8_t color) {
    char buf[8];
    intToString(score, buf, sizeof(buf));
    drawText(vga, x, y, label, color);
    drawText(vga, x + 10, y, buf, color);
}

static void writeInt(PortSerie &ps, int value) {
    char buf[16];
    int i = 0;
    bool neg = value < 0;
    int tmp = neg ? -value : value;
    if (tmp == 0) {
        buf[i++] = '0';
    }
    while (tmp > 0 && i < 15) {
        buf[i++] = '0' + (tmp % 10);
        tmp /= 10;
    }
    if (neg) buf[i++] = '-';
    for (int j = 0; j < i / 2; ++j) {
        char c = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = c;
    }
    buf[i] = '\0';
    ps.ecrireMot(buf);
}

Breakout::Breakout() : score1(0), score2(0), activeBalls(2), gameRunning(true),
                       lives1(PLAYER_LIVES), lives2(PLAYER_LIVES), tickCounter(0),
                       keyLeft1(false), keyRight1(false), keyLeft2(false), keyRight2(false) {
    
    // Initialize paddles on the same (bottom) side
    paddle1 = Paddle(280, PADDLE1_Y, 60, 8, 15);  // Player 1 closest to the edge
    paddle2 = Paddle(280, PADDLE2_Y, 60, 8, 11);  // Player 2 just above
    paddle1.setSpeed(3);
    paddle2.setSpeed(3);
    paddle1.setBounds(8, SCREEN_WIDTH - paddle1.getWidth() - 8);
    paddle2.setBounds(8, SCREEN_WIDTH - paddle2.getWidth() - 8);
    
    // Player 1 ball (moves upward)
    balls[0] = Ball(320, paddle1.getY() - BALL_DEFAULT_SIZE, BALL_DEFAULT_SIZE, COLOR_PLAYER1, OWNER_PLAYER1);
    balls[0].setVelocity(clampHorizontalSpeed(1), -BASE_BALL_SPEED_Y);

    // Player 2 ball (also moves upward from second paddle)
    balls[1] = Ball(320, paddle2.getY() - BALL_DEFAULT_SIZE, BALL_DEFAULT_SIZE, COLOR_PLAYER2, OWNER_PLAYER2);
    balls[1].setVelocity(clampHorizontalSpeed(-1), -BASE_BALL_SPEED_Y);

    // Other balls inactive
    for (int i = 2; i < MAX_BALLS; i++) {
        balls[i].setActive(false);
    }
}

void Breakout::init() {
    initBricks();
}

void Breakout::initBricks() {
    const int brickWidth = 50;
    const int brickHeight = 12;
    const int spacingX = 3;
    const int spacingY = 4;
    const int startX = 10;
    const int startY = 20; // keep bricks close to the top edge

    const ui8_t rowColors[BRICKS_ROWS] = {
        COLOR_BRICK_RED,
        COLOR_BRICK_BLUE,
        COLOR_BRICK_GREEN,
        COLOR_BRICK_GREEN,
        COLOR_BRICK_GREEN
    };
    const int rowHP[BRICKS_ROWS] = {3, 2, 1, 1, 1};
    
    int idx = 0;
    for (int row = 0; row < BRICKS_ROWS; row++) {
        for (int col = 0; col < BRICKS_COLS; col++) {
            int x = startX + col * (brickWidth + spacingX);
            int y = startY + row * (brickHeight + spacingY);
            bool powerup = false;
            bricks[idx] = Brick(x, y, brickWidth, brickHeight,
                                rowColors[row], rowHP[row], powerup);
            bricks[idx].setActive(true);
            idx++;
        }
    }
}

void Breakout::setKeyLeft1(bool state) {
    keyLeft1 = state;
}

void Breakout::setKeyRight1(bool state) {
    keyRight1 = state;
}

void Breakout::setKeyLeft2(bool state) {
    keyLeft2 = state;
}

void Breakout::setKeyRight2(bool state) {
    keyRight2 = state;
}

bool Breakout::isGameRunning() const {
    return gameRunning;
}

void Breakout::checkBallWallCollision(Ball &ball) {
    if (!ball.isActive()) return;
    
    // Left and right walls
    if (ball.getX() <= 0) {
        ball.setPosition(0, ball.getY());
        ball.setVelocity(clampHorizontalSpeed(-ball.getVX()), ball.getVY());
    } else if (ball.getX() >= SCREEN_WIDTH - ball.getSize()) {
        ball.setPosition(SCREEN_WIDTH - ball.getSize(), ball.getY());
        ball.setVelocity(clampHorizontalSpeed(-ball.getVX()), ball.getVY());
    }
    
    // Top wall simply bounces
    if (ball.getY() <= 0) {
        ball.reverseY();
        return;
    }
    
    // Bottom wall means both players missed (penalize owner)
    if (ball.getY() >= SCREEN_HEIGHT - ball.getSize()) {
        handleBallMiss(ball, ball.getOwner());
        return;
    }
}

bool Breakout::checkBallPaddleCollision(Ball &ball, Paddle &paddle) {
    if (!ball.isActive()) return false;
    
    int ballX = ball.getX();
    int ballY = ball.getY();
    int ballSize = ball.getSize();
    
    int paddleX = paddle.getX();
    int paddleY = paddle.getY();
    int paddleW = paddle.getWidth();
    int paddleH = paddle.getHeight();
    
    // Simple AABB collision detection
    if (ballX + ballSize >= paddleX && ballX <= paddleX + paddleW &&
        ballY + ballSize >= paddleY && ballY <= paddleY + paddleH) {
        attachOwnerToBall(ball, (&paddle == &paddle1) ? OWNER_PLAYER1 : OWNER_PLAYER2);
        applyPaddleBounce(ball, paddle);
        return true;
    }
    
    return false;
}

void Breakout::checkBallBrickCollision(Ball &ball) {
    if (!ball.isActive()) return;
    
    int ballX = ball.getX();
    int ballY = ball.getY();
    int ballSize = ball.getSize();
    
    for (int i = 0; i < MAX_BRICKS; i++) {
        if (!bricks[i].isActive()) continue;
        
        int brickX = bricks[i].getX();
        int brickY = bricks[i].getY();
        int brickW = bricks[i].getWidth();
        int brickH = bricks[i].getHeight();
        
        // AABB collision detection
        if (ballX + ballSize >= brickX && ballX <= brickX + brickW &&
            ballY + ballSize >= brickY && ballY <= brickY + brickH) {
            ball.reverseY();
            bool destroyed = bricks[i].takeHit();
            if (destroyed) {
                updateScoresForBrick(ball.getOwner());
                checkVictoryConditions();
            }
            return; // Only collide with one brick per frame
        }
    }
}

void Breakout::checkCollisions() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].isActive()) continue;
        
        checkBallWallCollision(balls[i]);
        checkBallPaddleCollision(balls[i], paddle1);
        checkBallPaddleCollision(balls[i], paddle2);
        checkBallBrickCollision(balls[i]);
    }
}

void Breakout::attachOwnerToBall(Ball &ball, BallOwner owner) {
    if (owner == OWNER_NONE) return;
    ball.setOwner(owner);
    ball.setColor(owner == OWNER_PLAYER1 ? COLOR_PLAYER1 : COLOR_PLAYER2);
    si32_t vx = ball.getVX();
    if (vx == 0) {
        vx = (owner == OWNER_PLAYER1) ? MIN_BALL_SPEED_X : -MIN_BALL_SPEED_X;
    }
    ball.setVelocity(clampHorizontalSpeed(vx), -BASE_BALL_SPEED_Y);
}

void Breakout::applyPaddleBounce(Ball &ball, const Paddle &paddle) {
    int paddleCenter = paddle.getX() + paddle.getWidth() / 2;
    int ballCenter = ball.getX() + ball.getSize() / 2;
    int offset = ballCenter - paddleCenter;
    int halfWidth = paddle.getWidth() / 2;
    si32_t newVX = (offset * MAX_BALL_SPEED_X) / (halfWidth ? halfWidth : 1);
    ball.setVelocity(clampHorizontalSpeed(newVX), -BASE_BALL_SPEED_Y);
}

void Breakout::handleBallMiss(Ball &ball, BallOwner owner) {
    if (owner == OWNER_NONE) {
        owner = OWNER_PLAYER1;
    }
    bool stillAlive = penalizePlayer(owner);
    if (!stillAlive || !gameRunning) {
        ball.setActive(false);
        return;
    }
    attachOwnerToBall(ball, owner);
    ball.setSize(BALL_DEFAULT_SIZE);
    const Paddle& paddle = (owner == OWNER_PLAYER1) ? paddle1 : paddle2;
    si32_t startX = paddle.getX() + paddle.getWidth() / 2 - ball.getSize() / 2;
    si32_t startY = paddle.getY() - ball.getSize() - 2;
    si32_t startVX = (owner == OWNER_PLAYER1 ? MIN_BALL_SPEED_X : -MIN_BALL_SPEED_X);
    si32_t startVY = -BASE_BALL_SPEED_Y;
    ball.setPosition(startX, startY);
    ball.setVelocity(startVX, startVY);
    ball.setActive(true);
}

void Breakout::spawnBallForPlayer(BallOwner owner, si32_t x, si32_t y, si32_t vx, si32_t vy) {
    for (int i = 0; i < MAX_BALLS; ++i) {
        if (!balls[i].isActive()) {
            balls[i].setActive(true);
            balls[i].setSize(BALL_DEFAULT_SIZE);
            balls[i].setPosition(x, y);
            balls[i].setVelocity(clampHorizontalSpeed(vx), vy);
            attachOwnerToBall(balls[i], owner);
            return;
        }
    }
}

void Breakout::updateScoresForBrick(BallOwner owner) {
    if (owner == OWNER_NONE) return;
    PortSerie ps;
    if (owner == OWNER_PLAYER1) {
        score1 += 1;
        ps.ecrireMot("Player 1 +1 (brique). Score=");
        writeInt(ps, score1);
        ps.ecrireMot("\n");
    } else {
        score2 += 1;
        ps.ecrireMot("Player 2 +1 (brique). Score=");
        writeInt(ps, score2);
        ps.ecrireMot("\n");
    }
}

bool Breakout::penalizePlayer(BallOwner owner) {
    if (owner == OWNER_NONE) return true;
    PortSerie ps;
    if (owner == OWNER_PLAYER1) {
        lives1--;
        score1 -= 1;
        ps.ecrireMot("Player 1 -1 vie (reste ");
        writeInt(ps, lives1);
        ps.ecrireMot(") score=");
        writeInt(ps, score1);
        ps.ecrireMot("\n");
        checkVictoryConditions();
        return lives1 > 0;
    } else {
        lives2--;
        score2 -= 1;
        ps.ecrireMot("Player 2 -1 vie (reste ");
        writeInt(ps, lives2);
        ps.ecrireMot(") score=");
        writeInt(ps, score2);
        ps.ecrireMot("\n");
        checkVictoryConditions();
        return lives2 > 0;
    }
}

bool Breakout::allBricksCleared() const {
    for (int i = 0; i < MAX_BRICKS; ++i) {
        if (bricks[i].isActive()) {
            return false;
        }
    }
    return true;
}

void Breakout::checkVictoryConditions() {
    if (!gameRunning) return;
    if (lives1 <= 0 || lives2 <= 0 || allBricksCleared()) {
        gameRunning = false;
        if (globalVGA) {
            render();
            globalVGA->swapBuffer();
        }
    }
}

void Breakout::render() {
    if (!globalVGA) return;
    
    // Clear screen
    globalVGA->clear(0);
    
    // Draw paddles
    for (int i = 0; i < paddle1.getHeight(); i++) {
        for (int j = 0; j < paddle1.getWidth(); j++) {
            ui8_t col = (i == 0) ? COLOR_POWERUP_ICON : (i == paddle1.getHeight() - 1 ? 0 : (char)paddle1.getColor());
            globalVGA->paint(paddle1.getX() + j, paddle1.getY() + i, col);
        }
    }
    for (int i = 0; i < paddle2.getHeight(); i++) {
        for (int j = 0; j < paddle2.getWidth(); j++) {
            ui8_t col = (i == 0) ? COLOR_POWERUP_ICON : (i == paddle2.getHeight() - 1 ? 0 : (char)paddle2.getColor());
            globalVGA->paint(paddle2.getX() + j, paddle2.getY() + i, col);
        }
    }
    
    // Draw bricks
    for (int i = 0; i < MAX_BRICKS; i++) {
        if (bricks[i].isActive()) {
            ui8_t col = bricks[i].getColor();
            for (int y = 0; y < bricks[i].getHeight(); y++) {
                for (int x = 0; x < bricks[i].getWidth(); x++) {
                    ui8_t pixelColor = (char)col;
                    if (y == 0) {
                        pixelColor = COLOR_POWERUP_ICON;
                    } else if (y == bricks[i].getHeight() - 1 || x == 0 || x == bricks[i].getWidth() - 1) {
                        pixelColor = 0;
                    }
                    globalVGA->paint(bricks[i].getX() + x, bricks[i].getY() + y, pixelColor);
                }
            }
        }
    }

    // Draw balls
    for (int i = 0; i < MAX_BALLS; ++i) {
        if (!balls[i].isActive()) continue;
        int size = balls[i].getSize();
        int radius = size / 2;
        int cx = balls[i].getX() + radius;
        int cy = balls[i].getY() + radius;
        ui8_t fillColor = (char)balls[i].getColor();
        for (int dy = -radius; dy < radius + (size % 2); ++dy) {
            for (int dx = -radius; dx < radius + (size % 2); ++dx) {
                if (dx * dx + dy * dy <= radius * radius) {
                    ui8_t pixel = (dy < 0 && dx * dx + dy * dy <= (radius - 1) * (radius - 1)) ? COLOR_POWERUP_ICON : fillColor;
                    globalVGA->paint(cx + dx, cy + dy, pixel);
                }
            }
        }
    }

    // Draw lives indicators (simple bars)
    for (int i = 0; i < lives1; ++i) {
        for (int dy = 0; dy < 6; ++dy) {
            for (int dx = 0; dx < 10; ++dx) {
                globalVGA->paint(10 + i * 12 + dx, SCREEN_HEIGHT - 15 + dy, COLOR_PLAYER1);
            }
        }
    }
    for (int i = 0; i < lives2; ++i) {
        for (int dy = 0; dy < 6; ++dy) {
            for (int dx = 0; dx < 10; ++dx) {
                globalVGA->paint(10 + i * 12 + dx, 5 + dy, COLOR_PLAYER2);
            }
        }
    }

    if (!gameRunning) {
        ui8_t panelColor = (score1 == score2) ? 8 : (score1 > score2 ? COLOR_PLAYER1 : COLOR_PLAYER2);
        for (int y = SCREEN_HEIGHT / 2 - 40; y < SCREEN_HEIGHT / 2 + 40; ++y) {
            for (int x = SCREEN_WIDTH / 2 - 80; x < SCREEN_WIDTH / 2 + 80; ++x) {
                globalVGA->paint(x, y, panelColor);
            }
        }
        drawScoreLine(globalVGA, SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 25, "P1:", score1, COLOR_PLAYER1);
        drawScoreLine(globalVGA, SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 10, "P2:", score2, COLOR_PLAYER2);
        const char* winnerText = (score1 == score2) ? "WIN -" : (score1 > score2 ? "WIN P1" : "WIN P2");
        drawText(globalVGA, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 10, winnerText, COLOR_POWERUP_ICON);
    }
}

void Breakout::updateLogic() {
    // Lock game state
    gameMutex.lock();

    if (!gameRunning) {
        gameMutex.unlock();
        return;
    }
    
    bool shouldMovePaddles = (tickCounter % PADDLE_SPEED_PERIOD) == 0;
    if (shouldMovePaddles) {
        if (keyLeft1) {
            paddle1.moveLeft();
        }
        if (keyRight1) {
            paddle1.moveRight();
        }
        
        if (keyLeft2) {
            paddle2.moveLeft();
        }
        if (keyRight2) {
            paddle2.moveRight();
        }
    }
    
    tickCounter++;
    bool shouldMove = (tickCounter % BALL_SPEED_PERIOD) == 0;
    // Move balls
    if (shouldMove) {
        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].isActive()) {
                balls[i].move();
            }
        }
    }
    
    // Check collisions
    checkCollisions();
    
    // Unlock game state
    gameMutex.unlock();
}

void Breakout::run() {
    while (gameRunning) {
        updateLogic();
        
        // Yield to other threads
        Yield();
    }
}
