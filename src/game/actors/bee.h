#pragma once
#include "engine/AI_base.h"
#include "renderer/ren_queue.h"

#define ACTION_IDLE   0x01
#define ACTION_PATROL 0x02
#define ACTION_ATTACK 0x04

#define STATE_NONE    0x00
#define STATE_PAIN    0x01
#define STATE_DEATH   0x02

enum class DIR_STATE
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Bee : public AI
{
public:
    float health;

    Bee() {}
    Bee(SDL_Renderer* ren, v2 pos);

    void init(SDL_Renderer* ren) override;
    void update(float delta) override;
    void queueForRender(Camera& cam) override;
    void onDamage(float amount) override;
    void onDeath() override;

private:
    SDL_Renderer* m_ren;

    bool flipH; // true to move right, false to move left

    SDL_Rect m_src;

    Animation* currentAnim;
    Animation dIdle; // bees will not diddle players. *DOWN IDLE*
    Animation uIdle;
    Animation sIdle;
    Animation dWalk;
    Animation uWalk;
    Animation sWalk;
    Animation dDeath;
    Animation uDeath;
    Animation sDeath;

    DIR_STATE stat;

    void stateMachine();
    void matchAnimation();
    void addFrames(SDL_Renderer* ren);
    void move(float delta, const std::vector<SDL_Rect>& solids);
};
