#ifndef BREAKOUT_INPUT_H
#define BREAKOUT_INPUT_H

#include "Breakout.h"

class BreakoutInput {
private:
    Breakout *game;
    
public:
    BreakoutInput(Breakout *breakoutGame);
    void handleKeyPress(unsigned char scancode);
    void handleKeyRelease(unsigned char scancode);
};

#endif
