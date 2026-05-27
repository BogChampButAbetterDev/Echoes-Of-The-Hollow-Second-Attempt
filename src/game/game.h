#include <iostream>
#include "renderer/renderer.h"
#include "engine/tilemap.h"
#include "engine/scene.h"
#include "engine/font.h"
#include "engine/ui.h"
#include "actors/player.h"

class Game
{
public:
    Game();
    void mainLoop();
    void end();

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

    Scene* m_currentScene;
    std::unordered_map<std::string, Scene> m_scenes;

    Camera m_cam;
    Player m_player;

    Font* m_font;
    UI m_ui;

    void init();
    void checkInteraction();
    void checkContact();

    void loadScene(const std::string& mapId, const std::string& spawnName);
    void checkDoorTransitions();

    enum class FadeState 
    {
        NONE,
        FADING_OUT,
        FADING_IN
    };
    FadeState m_fadestat = FadeState::NONE;
    int m_fadeAlpha = 0;
    float fadeSpeed = 200.0f;
    int fadeDirection = 0;

    bool shouldLoadScene = false;
    std::string m_pendingMap;
    std::string m_pendingSpawn;

    void startTransition(const std::string& mapId, const std::string& spawnName);
    void updateFade();

    SDL_Texture* m_dialogueTex;
};
