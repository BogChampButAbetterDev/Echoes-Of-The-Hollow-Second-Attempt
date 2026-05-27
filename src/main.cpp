#include <iostream>
#define SDL_MAIN_HANDLED

#include "game/game.h"

int main()
{
    Game game;
    game.mainLoop();
    game.end();
}
