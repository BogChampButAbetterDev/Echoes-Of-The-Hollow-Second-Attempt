#include "menu.h"

MenuItem::MenuItem(std::string label, std::function<void()> onSelect, bool closesMenu)
    : m_label(std::move(label)), m_onSelect(std::move(onSelect)), m_closesMenu(closesMenu) {}

Menu::Menu(Font* font, std::vector<MenuItem> items)
: m_font(font), m_items(std::move(items))
{
}

MenuItem::MenuItem(std::string label)
: m_label(std::move(label)), m_onSelect(nullptr), m_closesMenu(false) 
{

}

Menu::~Menu()
{
    for (auto* tex : m_itemTextures)
        if (tex) SDL_DestroyTexture(tex);
}

void Menu::rebuildTextures(SDL_Renderer* ren) {
    m_itemTextures.clear();
    m_itemRects.clear();

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255}; 
    int outlineSize = 2;              

    SDL_Rect box = {SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 100, 300, 200};
    int y = box.y + 20;

    for (auto& item : m_items)
    {
        std::string label = item.label();

        TTF_SetFontOutline(m_font->getFont(), outlineSize);
        SDL_Texture* outlineTex = m_font->renderText(ren, label, black, 0);
        m_itemTextures.push_back(outlineTex);

        int otw, oth;
        SDL_QueryTexture(outlineTex, nullptr, nullptr, &otw, &oth);
        m_itemRects.push_back({ box.x + 40, y, otw, oth });

        TTF_SetFontOutline(m_font->getFont(), 0); // Reset outline to 0
        SDL_Texture* innerTex = m_font->renderText(ren, label, white, 0);
        m_itemTextures.push_back(innerTex);

        int itw, ith;
        SDL_QueryTexture(innerTex, nullptr, nullptr, &itw, &ith);
        
        m_itemRects.push_back({ box.x + 40 + outlineSize, y + outlineSize, itw, ith });

        y += oth + 10;
    }

    m_texturesBuilt = true;
}

int Menu::hitTest(float cx, float cy) const
{
    // since im retarded, this can be called before render() and seg fault
    // that's what the guard is for
    if (m_itemRects.empty()) return -1;

    SDL_Point p = { (int)cx, (int)cy };
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].hasCallback() && SDL_PointInRect(&p, &m_itemRects[i * 2]))
            return (int)i;
    }
    return -1;
}

void Menu::handleInput(const Cursor& cursor)
{
    m_hoveredIndex = hitTest(cursor.getPos().x, cursor.getPos().y);

    bool confirmPressed = cursor.input.interact && !m_prevInteract;
    m_prevInteract = cursor.input.interact;

    if (confirmPressed && m_hoveredIndex != -1)
    {
        const MenuItem& item = m_items[m_hoveredIndex];
        if (!item.hasCallback()) return;
        if (item.closesMenu())
            m_closed = true;
        item.select();
    }
}

void Menu::render(SDL_Renderer* ren)
{
    if (!m_texturesBuilt)
        rebuildTextures(ren);

    if (m_bg)
        SDL_RenderCopy(ren, m_bg, nullptr, nullptr);

    for (size_t i = 0; i < m_itemTextures.size(); ++i)
    {
        SDL_Texture* tex = m_itemTextures[i];
        if (!tex) continue;

        SDL_Rect dst = m_itemRects[i];
        SDL_RenderCopy(ren, tex, nullptr, &dst);

        if ((int)(i / 2) == m_hoveredIndex)
        {
            SDL_Rect highlight = m_itemRects[i];
            highlight.x -= 5; highlight.w += 10;
            SDL_SetRenderDrawColor(ren, 255, 255, 0, 80);
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(ren, &highlight);
        }
    }
}
