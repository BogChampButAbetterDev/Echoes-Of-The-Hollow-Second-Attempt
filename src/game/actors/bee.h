#pragma once
#include "engine/AI_base.h"
#include "renderer/ren_queue.h"

#define ACTION_IDLE   0x01
#define ACTION_PATROL 0x02
#define ACTION_CHASE  0x04
#define ACTION_ATTACK 0x08

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
    void update(float delta, const std::vector<SDL_Rect>& solids, float px, float py, SDL_Rect mapBounds) override;
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

    float m_patrolTimer = 0.0f;
    float m_patrolDX    = 0.0f;
    float m_patrolDY    = 0.0f;
    // set by stateMachine when chasing
    float m_targetX     = 0.0f; 
    float m_targetY     = 0.0f;

    SDL_Rect getHitbox();

    void stateMachine(float px, float py);
    void matchAnimation();
    void addFrames(SDL_Renderer* ren);
    void move(float delta, const std::vector<SDL_Rect>& solids, SDL_Rect mapBounds);
};
