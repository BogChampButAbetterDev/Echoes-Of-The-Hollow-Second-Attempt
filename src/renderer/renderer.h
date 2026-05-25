#pragma once
#ifdef __3DS__
    #include <3ds.h>
    #include <citro2d.h>
#else
    #include <SDL2/SDL.h>
    #include <algorithm>
    #include "ren_queue.h"
    #include "engine/texture.h"
#endif

#include "globals/globals.h"

class Render
{
public:
#ifdef __3DS__
    C2D_RenderTarget* top    = nullptr;
    C2D_RenderTarget* bottom = nullptr;
#else
    SDL_Renderer* renderer = nullptr;
#endif
    Render() {}

#ifdef __3DS__
    Render(int dummy); 
#else
    explicit Render(SDL_Window* win);
#endif

    void beginFrame();
    void presentFrame();
    void renderFromQueue();

private:
#ifndef __3DS__
    SDL_Renderer* createRen(SDL_Window* win);
#endif
};
