#include "menus.h"
#include "game.h"

namespace Menus
{
    std::unique_ptr<Menu> mainMenuDEBUG(SDL_Renderer* ren, Game* game)
    {
        auto menu = std::make_unique<Menu>(game->getUI()->getFont(), std::vector<MenuItem>
        {
            MenuItem("Unnamed RPG game DEV"),
            MenuItem("Load Main Game", [game]() 
            { 
                game->loadScene("player_house.tmx", "default"); 
            }, true),
            MenuItem("Load Test Scene", [game]() 
            { 
                game->loadScene("testworld.tmx", "default"); 
            }, true),
            MenuItem("Nevermind. Get me tf out of here", [game]() 
            { 
                game->setRunState(false); 
            }, false),
            MenuItem("v0.1 BETA Debug")
        });

        menu->setBG(Texture::loadTex(ren, BG_PATH("main_menu.png")));
        return menu;
    }
}
