#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <sextant/Activite/Threads.h>
#include <sextant/Synchronisation/Mutex/Mutex.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"

#define MAX_BALLS 6
#define BRICKS_ROWS 5
#define MAX_BRICKS (BRICKS_ROWS * BRICKS_COLS)
#define BRICKS_COLS 12
#define PLAYER_LIVES 3

class Breakout : public Threads {
private:
    Ball balls[MAX_BALLS];
    Paddle paddle1;  // Player 1 (bottom-most)
    Paddle paddle2;  // Player 2 (just above player 1)
    Brick bricks[MAX_BRICKS];
    
    Mutex gameMutex;
    
    int score1;
    int score2;
    int activeBalls;
    bool gameRunning;
    int lives1;
    int lives2;
    int tickCounter;
    
    // Keyboard state
    bool keyLeft1;
    bool keyRight1;
    bool keyLeft2;
    bool keyRight2;
    
    void initBricks();
    void checkCollisions();
    bool checkBallPaddleCollision(Ball &ball, Paddle &paddle);
    void checkBallBrickCollision(Ball &ball);
    void checkBallWallCollision(Ball &ball);
    void handleBallMiss(Ball &ball, BallOwner owner);
    void attachOwnerToBall(Ball &ball, BallOwner owner);
    void applyPaddleBounce(Ball &ball, const Paddle &paddle);
    void spawnBallForPlayer(BallOwner owner, si32_t x, si32_t y, si32_t vx, si32_t vy);
    void updateScoresForBrick(BallOwner owner);
    bool penalizePlayer(BallOwner owner);
    bool allBricksCleared() const;
    void checkVictoryConditions();
    
public:
    Breakout();
    
    void init();
    void run();
    void render();
    
    void setKeyLeft1(bool state);
    void setKeyRight1(bool state);
    void setKeyLeft2(bool state);
    void setKeyRight2(bool state);
    
    bool getKeyLeft1() const { return keyLeft1; }
    bool getKeyRight1() const { return keyRight1; }
    bool getKeyLeft2() const { return keyLeft2; }
    bool getKeyRight2() const { return keyRight2; }
    
    Ball* getBalls() { return balls; }
    Paddle* getPaddle1() { return &paddle1; }
    Paddle* getPaddle2() { return &paddle2; }
    
    void updateLogic();  // Update game state without rendering
    
    bool isGameRunning() const;
};

#endif
