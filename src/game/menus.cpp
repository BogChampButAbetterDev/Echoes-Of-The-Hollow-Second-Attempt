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
            MenuItem("Scene Selector", [game, ren]()
            {
                game->getUI()->openMenu(DEBUG_SceneSelect(ren, game));
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

    std::unique_ptr<Menu> DEBUG_SceneSelect(SDL_Renderer* ren, Game* game)
    {
        auto menu = std::make_unique<Menu>(game->getUI()->getFont(), std::vector<MenuItem>
        {
            MenuItem("SCENE SELECT"),
            MenuItem("House", [game]()
            {
                game->loadScene("player_house.tmx", "default");
            }, true),
            MenuItem("Starting area from house", [game]()
            {
                game->loadScene("castle_town.tmx", "from_house");
            }, true),
            MenuItem("Sword cave", [game]()
            {
                game->loadScene("sword_cave.tmx", "default");
            }, true),
            MenuItem("Starting area from sword cave", [game]()
            {
                game->loadScene("castle_town.tmx", "from_sword_cave");
            }, true),
            MenuItem("Back to main menu", [game]()
            {
                game->quitToMenu();
            }, true),
        });

        menu->setBG(Texture::loadTex(ren, BG_PATH("pause_menu.png")));
        return menu;
    }

    std::unique_ptr<Menu> pauseMenu(SDL_Renderer* ren, Game* game)
    {
        auto menu = std::make_unique<Menu>(game->getUI()->getFont(), std::vector<MenuItem>
        {
            MenuItem("Pause"),
            MenuItem("Resume", [game]()
            {
                // only needs to close the menu so I don't need do anything
                // other then set closes_menu to true which will handle closing
                // pasue menu safely, automatically
            }, /*closes_menu=*/true),
            MenuItem("Quit to Menu", [game]()
            {
                game->quitToMenu();
            }, true)
        });

        menu->setBG(Texture::loadTex(ren, BG_PATH("pause_menu.png")));
        return menu;
    }
}
