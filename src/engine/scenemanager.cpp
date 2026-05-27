#include "game/game.h"
#include "scenemanager.h"

SceneManager::SceneManager(SDL_Renderer *ren)
{
    renderer = ren;
    state = State::NONE;
    alpha = 0.0f;
    speed = 300.0f;
}

void SceneManager::update(float dt, Game &game)
{
    switch (state)
    {
        case State::NONE:
            return;

        case State::FADE_OUT:
            alpha += speed * dt;

            if (alpha >= 255)
            {
                alpha = 255;
                state = State::LOAD;
            }
            break;

        case State::LOAD:
            game.loadScene(pendingMap, pendingSpawn);
            state = State::FADE_IN;
            break;

        case State::FADE_IN:
            alpha -= speed * dt;

            if (alpha <= 0)
            {
                alpha = 0;
                state = State::NONE;
            }
            break;
    }
}

void SceneManager::render(SDL_Renderer* ren)
{
    // skip if no scene change
    if (state == State::NONE)
        return;

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, (Uint8)alpha);

    SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(ren, &full);
}

void SceneManager::requestScene(const std::string &map, const std::string &spawn)
{
    // ignore if already transitioning
    if (state != State::NONE)
        return;

    pendingMap = map;
    pendingSpawn = spawn;

    alpha = 0.0f;
    state = State::FADE_OUT;
}

bool SceneManager::isTransitioning() const
{
    return state != State::NONE;
}
