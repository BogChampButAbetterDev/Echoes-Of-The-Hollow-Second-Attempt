#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "font.h"
#include "globals/globals.h"

class UI
{
public:
    UI() : m_font(nullptr), m_dialogueOpen(false), m_dialogueTex(nullptr), m_currentPage(0) {}
    UI(Font* font) : m_font(font), m_dialogueOpen(false), m_dialogueTex(nullptr), m_currentPage(0) {}

    void openDialogue(SDL_Renderer* ren, const std::string& text);
    void advance(SDL_Renderer* ren);
    void closeDialogue();
    bool isDialogueOpen() const { return m_dialogueOpen; }
    bool isLastPage() const { return m_currentPage >= (int)m_pages.size() - 1; }

    void render(SDL_Renderer* ren);

private:
    Font* m_font;
    bool m_dialogueOpen;
    std::vector<std::string> m_pages;
    SDL_Texture* m_dialogueTex;
    int m_currentPage;

    void rebuildTexture(SDL_Renderer* ren);
    void renderDialogue(SDL_Renderer* ren);
};
