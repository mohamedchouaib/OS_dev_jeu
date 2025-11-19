#ifndef BALL_H
#define BALL_H

#include <sextant/types.h>

enum BallOwner {
	OWNER_NONE = 0,
	OWNER_PLAYER1,
	OWNER_PLAYER2
};

class Ball {
private:
    si32_t x;
    si32_t y;
    si32_t vx;  // velocity x
    si32_t vy;  // velocity y
    ui16_t size;
    ui8_t color;
    bool active;
    BallOwner owner;

public:
    Ball();
    Ball(si32_t x, si32_t y, ui16_t size, ui8_t color, BallOwner owner = OWNER_NONE);
    
    void move();
    void setPosition(si32_t x, si32_t y);
    void setVelocity(si32_t vx, si32_t vy);
    void reverseX();
    void reverseY();
    void setActive(bool state);
    void setSize(ui16_t newSize);
    void setColor(ui8_t newColor);
    void setOwner(BallOwner newOwner);
    
    bool isActive() const;
    si32_t getX() const;
    si32_t getY() const;
    si32_t getVX() const;
    si32_t getVY() const;
    ui16_t getSize() const;
    ui8_t getColor() const;
    BallOwner getOwner() const;
    
    void draw();
};

#endif
