#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include "animation.h"
#include "renderer/ren_queue.h"
#include "input.h"
#include "globals/globals.h"
#include "globals/Actor.h"

class Cursor
{
public:
    Cursor(SDL_Renderer* ren)
    {
        addFrames(ren);
        currentAnim = &def;
        m_rect = currentAnim->getCurrentFrame();

        x = SCREEN_WIDTH  / 2;
        y = SCREEN_HEIGHT / 2;
    }

    void update(float dt);
    void render(SDL_Renderer* ren);

    v2 getPos() const {v2 pos; pos.x = x; pos.y = y; return pos;}

    Input input;

private:
    void move(float dt);
    void addFrames(SDL_Renderer* ren);

    float x, y;

    SDL_Rect m_rect;

    Animation def; // default
    Animation click;

    Animation* currentAnim;
};
