#pragma once 
#include <SDL2/SDL.h>
#include "ui_cursor.h"

class UIScreen
{
public:
    virtual ~UIScreen() = default;
    virtual void handleInput(const Cursor& cursor) = 0;
    virtual void update(float dt) {}
    virtual void render(SDL_Renderer* ren) = 0;
    virtual bool wantsToClose() const = 0;
};
