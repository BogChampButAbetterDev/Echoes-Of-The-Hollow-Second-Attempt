#include "ui.h"

void UI::openDialogue(SDL_Renderer* ren, const std::string& text)
{
    m_dialogue = std::make_unique<Dialogue>(m_font, ren, text);
}

void UI::advance(SDL_Renderer* ren)
{
    if (!m_dialogue) return;
    m_dialogue->advance(ren);
    if (m_dialogue->wantsToClose())
        m_dialogue.reset();
}

void UI::openMenu(std::unique_ptr<Menu> menu) 
{ 
    pushScreen(std::move(menu)); 
}

void UI::handleInput(const Cursor& cursor)
{
    if (m_menuStack.empty()) return;
    m_menuStack.back()->handleInput(cursor);
    if (m_menuStack.back()->wantsToClose())
        popScreen();
}

void UI::showAreaLabel(SDL_Renderer* ren, const std::string& text)
{
    std::string t = text;
    if (m_AL_active) 
    { 
        SDL_DestroyTexture(m_al.tex); 
        SDL_DestroyTexture(m_al.outlineTex);
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};

    TTF_SetFontOutline(m_font->getFont(), 2);
    m_al.outlineTex = m_font->renderText(ren, t, black);
    
    TTF_SetFontOutline(m_font->getFont(), 0);
    m_al.tex = m_font->renderText(ren, t, white, 0);

    SDL_QueryTexture(m_al.tex, nullptr, nullptr, &m_al.w, &m_al.h);
    m_al.timer = 0.f;
    m_AL_active = true;
}

void UI::update(float dt)
{
    if (!m_menuStack.empty())
        m_menuStack.back()->update(dt);
    
    if (m_AL_active)
    {
        m_al.timer += dt;
        if (m_al.timer >= m_al.duration)
        {
            SDL_DestroyTexture(m_al.tex);
            SDL_DestroyTexture(m_al.outlineTex);
            m_al.tex = nullptr;
            m_AL_active = false;
        }
    }
}

void UI::render(SDL_Renderer* ren)
{
    for (auto& screen : m_menuStack)
        screen->render(ren);

    if (m_dialogue)
        m_dialogue->render(ren);
    
    if (m_AL_active && m_al.tex)
    {
        float t = m_al.timer;
        float alpha = 1.0f;
        if (t < m_al.fadeIn)
            alpha = t / m_al.fadeIn;
        else if (t > m_al.duration - m_al.fadeOut)
            alpha = (m_al.duration - t) / m_al.fadeOut;

        u8 a = (u8)(alpha * 255);
        SDL_Rect dst = {SCREEN_WIDTH/2 - m_al.w/2, 60, m_al.w, m_al.h};

        SDL_SetTextureAlphaMod(m_al.outlineTex, a);
        SDL_RenderCopy(ren, m_al.outlineTex, nullptr, &dst);

        SDL_Rect innerDst = {dst.x + 2, dst.y + 2, m_al.w, m_al.h};
        SDL_SetTextureAlphaMod(m_al.tex, a);
        SDL_RenderCopy(ren, m_al.tex, nullptr, &innerDst);
    }
}
