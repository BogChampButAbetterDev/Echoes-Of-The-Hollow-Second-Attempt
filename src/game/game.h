#include <iostream>
#include "renderer/renderer.h"
#include "engine/tilemap.h"
#include "engine/scene.h"
#include "engine/scenemanager.h"
#include "scenes/allScenes.h"
#include "engine/font.h"
#include "engine/ui.h"
#include "engine/ui_cursor.h"
#include "engine/story_state.h"
#include "actors/player.h"
#include "actors/bee.h"
#include "menus.h"
#include "inventory_manager.h"

class Game
{
public:
    Game();
    void mainLoop();
    void end();

    void loadScene(const std::string& mapId, const std::string& spawnName);

    UI* getUI() {return m_ui;}
    void setRunState(bool val) {running = val;}

    void quitToMenu();

private:
    SDL_Window* createWin();

    SDL_Window* m_win;
    Render m_ren;

    bool running;
    Uint64 lastTime;
    double delta;
    int m_fps;
    float m_fpsTimer;
    int m_frameCount;

    SceneManager m_sm;
    Scene* m_currentScene;
    AllScenes m_allScenes;
    std::unordered_map<std::string, Scene> m_scenes;

    StoryState m_story;

    Inventory m_inventory;

    Camera m_cam;
    Player m_player;

    Font* m_font;
    UI* m_ui = nullptr;
    Cursor* m_cursor;

    void init();
    void checkInteraction();
    void checkContact();
    void checkFlagDoors(); // doors that can be opened by use of flags rather than buttons

    void checkDoorTransitions();

    float fadeSpeed = 200.0f;
    int fadeDirection = 0;

    void updateFade();

    SDL_Texture* m_dialogueTex;
};
