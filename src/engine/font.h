#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class Font
{
public:
    static void init();
    static void quit();

    Font(const char* path, int size);
    ~Font();

    SDL_Texture* renderText(SDL_Renderer* ren, std::string& text, SDL_Color color, int wrapWidth = 0);

private:
    TTF_Font* m_font;
};
