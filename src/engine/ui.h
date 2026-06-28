#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include <vector>
#include "font.h"
#include "input.h"
#include "ui_screen.h"
#include "dialogue.h"
#include "menu.h"

class UI
{
public:
    UI(Font* font) : m_font(font) {}

    void openDialogue(SDL_Renderer* ren, const std::string& text);
    void advance(SDL_Renderer* ren);
    bool isDialogueOpen() const { return m_dialogue != nullptr; }

    void openMenu(std::unique_ptr<Menu> menu) ;
    void handleInput(const Cursor& cursor);
    bool hasOpenScreen() const { return !m_menuStack.empty(); }

    void update(float dt);
    void render(SDL_Renderer* ren);

    void pushScreen(std::unique_ptr<UIScreen> screen) { m_menuStack.push_back(std::move(screen)); }
    void popScreen() { if (!m_menuStack.empty()) m_menuStack.pop_back(); }

    Font* getFont() const {return m_font;}

    void showAreaLabel(SDL_Renderer* ren, const std::string& text);

private:
    struct areaLabel
    {
        std::string text;
        float timer = 0.0f;
        const float duration = 2.0f;
        // fade in for 0.5 seconds
        const float fadeIn = 0.5f; 
        // 1 second buffer
        // fade out for another 0.5 seconds
        const float fadeOut = 0.5f;
        SDL_Texture* tex = nullptr;
        SDL_Texture* outlineTex = nullptr;
        int w = 0; int h = 0;
    };

    Font* m_font;
    std::unique_ptr<Dialogue> m_dialogue;
    std::vector<std::unique_ptr<UIScreen>> m_menuStack;

    areaLabel m_al;
    bool m_AL_active = false;
};
