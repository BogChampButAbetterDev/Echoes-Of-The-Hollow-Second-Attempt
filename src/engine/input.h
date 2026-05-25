#pragma once 
#include <SDL2/SDL.h>
#include <iostream>

struct Input
{
    bool w, a, s, d;
    bool interact; // space
    bool escape;
};

void pollInput(Input& input, bool& isRunning);
