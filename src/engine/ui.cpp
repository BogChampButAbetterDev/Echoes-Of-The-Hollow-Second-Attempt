#include "ui.h"
#include <sstream>

void UI::openDialogue(SDL_Renderer *ren, const std::string &text)
{
    m_dialogueOpen = true;
    m_currentPage = 0;
    m_pages.clear();

    // split to new page on "|"s
    std::stringstream ss(text);
    std::string page;
    while (std::getline(ss, page, '|'))
    {
        m_pages.push_back(page);
    }

    rebuildTexture(ren);
}

void UI::advance(SDL_Renderer* ren)
{
    m_currentPage++;
    if (m_currentPage >= (int)m_pages.size())
    {
        closeDialogue();
        return;
    }

    rebuildTexture(ren);
}

void UI::rebuildTexture(SDL_Renderer* ren)
{
    if (m_dialogueTex)
        SDL_DestroyTexture(m_dialogueTex);
    
    SDL_Color white = {255, 255, 255, 255};
    int wrapWidth = SCREEN_WIDTH - 60; // padding on both sides;
    m_dialogueTex = m_font->renderText(ren, m_pages[m_currentPage], white, wrapWidth);
}

void UI::closeDialogue()
{
    m_dialogueOpen = false;
}

void UI::render(SDL_Renderer *ren)
{
    if (m_dialogueOpen)
        renderDialogue(ren);
}

void UI::renderDialogue(SDL_Renderer *ren)
{
    SDL_Rect box = {20, SCREEN_HEIGHT - 120, SCREEN_WIDTH - 40, 150};
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderFillRect(ren, &box);

    if (m_dialogueTex)
    {
        int tw, th;
        SDL_QueryTexture(m_dialogueTex, nullptr, nullptr, &tw, &th);
        SDL_Rect dst = {box.x + 10, box.y + 10, tw, th};
        SDL_RenderCopy(ren, m_dialogueTex, nullptr, &dst);
    }
}

UI::UiButton::UiButton(const char* texPath, SDL_Renderer* ren, int x, int y, int w, int h)
{
    m_tex = Texture::loadTex(ren, texPath);
    m_rect = 
    {
        x, y, w, h
    };
}

bool UI::UiButton::isClicked(float cX, float cY)
{
    return false;
}
