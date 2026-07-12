#pragma once
#include "animation.h" // may as well leave this in so subclasses can access animation
#include "camera.h"
#include "globals/Actor.h"
#include "game/actors/player.h"

struct AI
{
    u8 actionState; // things like patrolling or attacking
    u8 stateFlags;  // things like taking damage or being stunned

    float x, y;

    float health;
    
    virtual void init(SDL_Renderer* ren) = 0;
    // px and py can be left 0 if NPC has no use for them
    virtual void update(float delta, const std::vector<SDL_Rect>& solids, float px, float py, SDL_Rect mapBounds, Player& player) = 0;
    virtual void queueForRender(Camera& cam) = 0;
    virtual void onDamage(float amount) {}
    virtual void onDeath() {}

    virtual SDL_Rect getHitbox() = 0;

    virtual ~AI() {}
};
