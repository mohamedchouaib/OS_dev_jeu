#include "BreakoutInput.h"
#include <drivers/PortSerie.h>

// Scancodes for keyboard (AZERTY French layout)
#define SCANCODE_LEFT_ARROW  75    // Extended scancode for left arrow
#define SCANCODE_RIGHT_ARROW 77    // Extended scancode for right arrow
#define SCANCODE_Q          30     // Q key (AZERTY)
#define SCANCODE_D          32     // D key (AZERTY)

BreakoutInput::BreakoutInput(Breakout *breakoutGame) : game(breakoutGame) {}

void BreakoutInput::handleKeyPress(unsigned char scancode) {
    if (game == nullptr) return;
    
    PortSerie ps;
    
    switch (scancode) {
        // Player 1 controls (Arrow keys)
        case SCANCODE_LEFT_ARROW:
            ps.ecrireMot("Left arrow pressed\n");
            game->setKeyLeft1(true);
            break;
        case SCANCODE_RIGHT_ARROW:
            ps.ecrireMot("Right arrow pressed\n");
            game->setKeyRight1(true);
            break;
            
        // Player 2 controls (Q/D keys)
        case SCANCODE_Q:
            ps.ecrireMot("Q pressed\n");
            game->setKeyLeft2(true);
            break;
        case SCANCODE_D:
            ps.ecrireMot("D pressed\n");
            game->setKeyRight2(true);
            break;
    }
}

void BreakoutInput::handleKeyRelease(unsigned char scancode) {
    if (game == nullptr) return;
    
    switch (scancode) {
        // Player 1 controls (Arrow keys)
        case SCANCODE_LEFT_ARROW:
            game->setKeyLeft1(false);
            break;
        case SCANCODE_RIGHT_ARROW:
            game->setKeyRight1(false);
            break;
            
        // Player 2 controls (Q/D keys)
        case SCANCODE_Q:
            game->setKeyLeft2(false);
            break;
        case SCANCODE_D:
            game->setKeyRight2(false);
            break;
    }
}
