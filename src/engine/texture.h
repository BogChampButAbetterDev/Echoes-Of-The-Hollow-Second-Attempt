#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#ifdef __3DS__
    #include <citro2d.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

class Texture
{
public:
#ifndef __3DS__
    SDL_Texture* loadTex(SDL_Renderer* ren, const char* path);
    static void clearCache();
#else
    static C2D_SpriteSheet loadSheet(const char* path);
    static void clearSheetCache();
#endif

private:
    static std::unordered_map<std::string, SDL_Texture*> s_cache;
};
