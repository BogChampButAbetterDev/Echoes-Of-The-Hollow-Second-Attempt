#pragma once
#include <memory>
#include "engine/menu.h"
#include "globals/globals.h"

class Game;

namespace Menus
{
    std::unique_ptr<Menu> mainMenuDEBUG(SDL_Renderer* ren, Game* game);
}
