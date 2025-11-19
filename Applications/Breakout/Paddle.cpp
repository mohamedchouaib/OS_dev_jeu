#include "Paddle.h"
#include <drivers/vga.h>

Paddle::Paddle() : x(280), y(380), width(60), height(10), color(15), speed(4), minX(0), maxX(580) {}

Paddle::Paddle(ui16_t x, ui16_t y, ui16_t width, ui16_t height, ui8_t color)
    : x(x), y(y), width(width), height(height), color(color), speed(4), minX(0), maxX(580) {}

void Paddle::moveLeft() {
    if (x > minX) {
        x = (x > minX + speed) ? x - speed : minX;
    }
}

void Paddle::moveRight() {
    if (x < maxX) {
        x = (x + speed < maxX) ? x + speed : maxX;
    }
}

void Paddle::setPosition(ui16_t newX, ui16_t newY) {
    x = newX;
    y = newY;
}

void Paddle::setBounds(ui16_t min, ui16_t max) {
    minX = min;
    maxX = max;
}

void Paddle::setSpeed(ui16_t newSpeed) {
    speed = newSpeed;
}

ui16_t Paddle::getSpeed() const {
    return speed;
}

ui16_t Paddle::getX() const {
    return x;
}

ui16_t Paddle::getY() const {
    return y;
}

ui16_t Paddle::getWidth() const {
    return width;
}

ui16_t Paddle::getHeight() const {
    return height;
}

ui8_t Paddle::getColor() const {
    return color;
}

void Paddle::draw() {
    // Draw paddle as a filled rectangle
    for (ui16_t i = 0; i < height; i++) {
        plot_square(x, y + i, width, color);
    }
}
