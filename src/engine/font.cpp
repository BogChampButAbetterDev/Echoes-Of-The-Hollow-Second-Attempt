#include <iostream>
#include "font.h"

void Font::init()
{
    TTF_Init();
}

void Font::quit()
{
    TTF_Quit();
}

Font::Font(const char *path, int size)
{
    m_font = TTF_OpenFont(path, size);
    if (!m_font)
    {
        std::cout << path << "\n";
        std::cout << "Failed to load font " << TTF_GetError() << "\n";
        exit(1); 
    }
}

Font::~Font()
{
    TTF_CloseFont(m_font);
}

SDL_Texture *Font::renderText(SDL_Renderer *ren, std::string &text, SDL_Color color, int wrapWidth)
{
    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(m_font, text.c_str(), color, wrapWidth);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    return tex;
}
