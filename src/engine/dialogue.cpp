#include "dialogue.h"
#include <sstream>

Dialogue::Dialogue(Font* font, SDL_Renderer* ren, const std::string& text)
    : m_font(font)
{
    std::stringstream ss(text);
    std::string page;
    while (std::getline(ss, page, '|'))
        m_pages.push_back(page);

    rebuildTexture(ren);
}

Dialogue::~Dialogue()
{
    if (m_dialogueTex)
        SDL_DestroyTexture(m_dialogueTex);
}

void Dialogue::advance(SDL_Renderer* ren)
{
    m_currentPage++;
    if (m_currentPage >= (int)m_pages.size())
    {
        m_closed = true;
        return;
    }
    rebuildTexture(ren);
}

void Dialogue::rebuildTexture(SDL_Renderer* ren)
{
    if (m_dialogueTex)
        SDL_DestroyTexture(m_dialogueTex);

    SDL_Color white = {255, 255, 255, 255};
    int wrapWidth = SCREEN_WIDTH - 60;
    m_dialogueTex = m_font->renderText(ren, m_pages[m_currentPage], white, wrapWidth);
}

void Dialogue::render(SDL_Renderer* ren)
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
