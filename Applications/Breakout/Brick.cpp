#include "Brick.h"
#include <drivers/vga.h>

Brick::Brick() : x(0), y(0), width(40), height(10), color(1), active(true), hitPoints(1), extraBall(false) {}

Brick::Brick(ui16_t x, ui16_t y, ui16_t width, ui16_t height, ui8_t color, int hp, bool powerup)
    : x(x), y(y), width(width), height(height), color(color), active(true), hitPoints(hp), extraBall(powerup) {}

void Brick::setPosition(ui16_t newX, ui16_t newY) {
    x = newX;
    y = newY;
}

void Brick::setActive(bool state) {
    active = state;
}

void Brick::setProperties(ui8_t newColor, int hp, bool powerup) {
    color = newColor;
    hitPoints = hp;
    extraBall = powerup;
}

bool Brick::takeHit() {
    if (!active) return false;
    if (hitPoints > 0) {
        hitPoints--;
    }
    if (hitPoints <= 0) {
        active = false;
        return true;
    }
    return false;
}

bool Brick::isActive() const {
    return active;
}

ui16_t Brick::getX() const {
    return x;
}

ui16_t Brick::getY() const {
    return y;
}

ui16_t Brick::getWidth() const {
    return width;
}

ui16_t Brick::getHeight() const {
    return height;
}

ui8_t Brick::getColor() const {
    return color;
}

int Brick::getHitPoints() const {
    return hitPoints;
}

bool Brick::hasExtraBall() const {
    return extraBall;
}

void Brick::draw() {
    if (active) {
        plot_square(x, y, width, color);
        // Draw a simple rectangle by plotting multiple vertical lines
        for (ui16_t i = 0; i < height; i++) {
            plot_square(x, y + i, width, color);
        }
    }
}
