#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>
#include "ui_screen.h"
#include "font.h"
#include "globals/globals.h"

class MenuItem
{
public:
    explicit MenuItem(std::string label, std::function<void()> onSelect, bool closesMenu = true);
    explicit MenuItem(std::string label);

    void select() const { if (m_onSelect) m_onSelect(); }
    const std::string& label() const { return m_label; }
    bool closesMenu() const { return m_closesMenu; }

    bool hasCallback() const { return m_onSelect != nullptr; }

private:
    std::string m_label;
    std::function<void()> m_onSelect;
    bool m_closesMenu;
};

class Menu : public UIScreen
{
public:
    Menu(Font* font, std::vector<MenuItem> items);
    ~Menu();

    void handleInput(const Cursor& cursor) override;
    void render(SDL_Renderer* ren) override;
    bool wantsToClose() const override {return m_closed;}

    void setBG(SDL_Texture* tex) {m_bg = tex;}

private:
    void rebuildTextures(SDL_Renderer* ren);
    int hitTest(float cx, float cy) const; // returns index or -1

    SDL_Texture* m_bg = nullptr;

    Font* m_font = nullptr;
    std::vector<MenuItem> m_items;
    std::vector<SDL_Texture*> m_itemTextures;
    std::vector<SDL_Rect> m_itemRects;

    int m_hoveredIndex = -1;
    bool m_closed = false;
    bool m_texturesBuilt = false;

    bool m_prevInteract = false;
};
