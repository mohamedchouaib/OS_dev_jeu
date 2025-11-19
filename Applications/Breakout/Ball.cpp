#include "Ball.h"
#include <drivers/vga.h>

Ball::Ball()
    : x(320), y(200), vx(1), vy(-2), size(12), color(14), active(true), owner(OWNER_NONE) {}

Ball::Ball(si32_t x, si32_t y, ui16_t size, ui8_t color, BallOwner owner)
    : x(x), y(y), vx(1), vy(-2), size(size), color(color), active(true), owner(owner) {}

void Ball::move() {
    if (active) {
        x += vx;
        y += vy;
    }
}

void Ball::setPosition(si32_t newX, si32_t newY) {
    x = newX;
    y = newY;
}

void Ball::setVelocity(si32_t newVx, si32_t newVy) {
    vx = newVx;
    vy = newVy;
}

void Ball::reverseX() {
    vx = -vx;
}

void Ball::reverseY() {
    vy = -vy;
}

void Ball::setActive(bool state) {
    active = state;
}

void Ball::setSize(ui16_t newSize) {
    size = newSize;
}

void Ball::setColor(ui8_t newColor) {
    color = newColor;
}

void Ball::setOwner(BallOwner newOwner) {
    owner = newOwner;
}

bool Ball::isActive() const {
    return active;
}

si32_t Ball::getX() const {
    return x;
}

si32_t Ball::getY() const {
    return y;
}

si32_t Ball::getVX() const {
    return vx;
}

si32_t Ball::getVY() const {
    return vy;
}

ui16_t Ball::getSize() const {
    return size;
}

ui8_t Ball::getColor() const {
    return color;
}

BallOwner Ball::getOwner() const {
    return owner;
}

void Ball::draw() {
    if (active) {
        plot_square(x, y, size, color);
    }
}
