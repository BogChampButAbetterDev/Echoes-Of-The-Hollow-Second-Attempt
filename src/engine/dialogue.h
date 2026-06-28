#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "font.h"
#include "ui_screen.h"
#include "globals/globals.h"

class Dialogue
{
public:
    Dialogue(Font* font, SDL_Renderer* ren, const std::string& text);
    ~Dialogue();

    void advance(SDL_Renderer* ren);
    void render(SDL_Renderer* ren);
    bool wantsToClose() const { return m_closed; }
    bool isLastPage() const { return m_currentPage >= (int)m_pages.size() - 1; }

private:
    void rebuildTexture(SDL_Renderer* ren);
    Font* m_font;
    std::vector<std::string> m_pages;
    int m_currentPage = 0;
    SDL_Texture* m_dialogueTex = nullptr;
    bool m_closed = false;
};
