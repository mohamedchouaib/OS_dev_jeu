#ifndef BRICK_H
#define BRICK_H

#include <sextant/types.h>

class Brick {
private:
    ui16_t x;
    ui16_t y;
    ui16_t width;
    ui16_t height;
    ui8_t color;
    bool active;
    int hitPoints;
    bool extraBall;

public:
    Brick();
    Brick(ui16_t x, ui16_t y, ui16_t width, ui16_t height, ui8_t color, int hp = 1, bool powerup = false);
    
    void setPosition(ui16_t x, ui16_t y);
    void setActive(bool state);
    void setProperties(ui8_t newColor, int hp, bool powerup);
    bool takeHit();
    
    bool isActive() const;
    ui16_t getX() const;
    ui16_t getY() const;
    ui16_t getWidth() const;
    ui16_t getHeight() const;
    ui8_t getColor() const;
    int getHitPoints() const;
    bool hasExtraBall() const;
    
    void draw();
};

#endif
