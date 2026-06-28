#pragma once 
#include <SDL2/SDL.h>
#include <iostream>

struct Input
{
    bool w, a, s, d;
    bool attack;
    bool interact; // space
    bool escape;

    bool start;

    // raw analog | Normalized -1.0 to 1.0
    float axisX = 0.0f;
    float axisY = 0.0f;

    // set true when USING_CONTOLLER and no controller is connected
    bool noControllerWarning = false;

    SDL_GameController* controller = nullptr;
};

void pollInput(Input& input, bool& isRunning);
