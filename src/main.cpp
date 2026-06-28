#include <iostream>
#define SDL_MAIN_HANDLED
#undef main

#include "game/game.h"

int main()
{
    Game game;
    game.mainLoop();
    game.end();
    return 0;
}
