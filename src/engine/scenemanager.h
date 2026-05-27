#pragma once
#include <string>
#include <SDL2/SDL.h>

class Game;

class SceneManager
{
public:
    SceneManager(SDL_Renderer* ren);

    void update(float dt, Game& game);
    void render(SDL_Renderer* ren);

    void requestScene(const std::string& map, const std::string& spawn);

    bool isTransitioning() const;

private:
    SDL_Renderer* renderer;

    enum class State { NONE, FADE_OUT, LOAD, FADE_IN };
    State state = State::NONE;

    float alpha = 0;
    float speed = 300.0f;

    std::string pendingMap;
    std::string pendingSpawn;

    bool loadQueued = false;
};
