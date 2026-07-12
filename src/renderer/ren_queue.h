#pragma once
#include <vector>

#ifdef __3DS__
    #include <citro2d.h>
#else
    #include <SDL2/SDL.h>
#endif

struct RenderRequest
{   
    #ifdef __3DS__
        C2D_Image c2dImage;
    #else
        SDL_Texture* tex;
        SDL_Rect src;
    #endif
    int x, y, w, h;
    int z;
    bool flipH = false;
    Uint8 r = 255, g = 255, b = 255; // default no tint for sprites
};

extern std::vector <RenderRequest> r_queue;

#ifdef __3DS__
void submit(C2D_Image img, int x, int y, int w, int h);
#else
void submit(SDL_Texture* tex, SDL_Rect src, int x, int y, int w, int h, bool flipH=false, Uint8 r=255, Uint8 g=255, Uint8 b=255);
#endif
