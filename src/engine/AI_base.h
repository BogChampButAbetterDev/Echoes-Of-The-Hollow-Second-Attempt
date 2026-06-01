#pragma once
#include "animation.h" // may as well leave this in so subclasses can access animation
#include "camera.h"
#include "globals/Actor.h"

struct AI
{
    u8 actionState; // things like patrolling or attacking
    u8 stateFlags;  // things like taking damage or being stunned

    float x, y;
    
    virtual void init(SDL_Renderer* ren) = 0;
    virtual void update(float delta) = 0;
    virtual void queueForRender(Camera& cam) = 0;
    virtual void onDamage(float amount) {}
    virtual void onDeath() {}

    virtual ~AI() {}
};
