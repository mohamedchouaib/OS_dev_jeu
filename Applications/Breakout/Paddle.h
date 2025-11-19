#ifndef PADDLE_H
#define PADDLE_H

#include <sextant/types.h>

class Paddle {
private:
    ui16_t x;
    ui16_t y;
    ui16_t width;
    ui16_t height;
    ui8_t color;
    ui16_t speed;
    ui16_t minX;
    ui16_t maxX;

public:
    Paddle();
    Paddle(ui16_t x, ui16_t y, ui16_t width, ui16_t height, ui8_t color);
    
    void moveLeft();
    void moveRight();
    void setPosition(ui16_t x, ui16_t y);
    void setBounds(ui16_t min, ui16_t max);
    void setSpeed(ui16_t newSpeed);
    ui16_t getSpeed() const;
    
    ui16_t getX() const;
    ui16_t getY() const;
    ui16_t getWidth() const;
    ui16_t getHeight() const;
    ui8_t getColor() const;
    
    void draw();
};

#endif
