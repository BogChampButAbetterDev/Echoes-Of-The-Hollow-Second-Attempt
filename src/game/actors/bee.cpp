#include "bee.h"

Bee::Bee(SDL_Renderer* ren, v2 pos) 
{
    x = pos.x; y = pos.y;
}

void Bee::init(SDL_Renderer* ren)
{
    flipH = false;
    health = 100;
    actionState = ACTION_IDLE;
    stateFlags = STATE_NONE;
    stat = DIR_STATE::LEFT;
    addFrames(ren);
    currentAnim = &sIdle;
}

void Bee::update(float delta)
{
    m_src = currentAnim->getCurrentFrame();
}

void Bee::queueForRender(Camera& cam)
{
    submit(currentAnim->getTexture(), m_src, 
       cam.toScrX(x * MAP_RENDER_SCALE), 
       cam.toScrY(y * MAP_RENDER_SCALE), 
       m_src.w * SPRITE_RENDER_SCALE, 
       m_src.h * SPRITE_RENDER_SCALE,
       flipH);
}

void Bee::onDamage(float amount)
{
    
}

void Bee::onDeath() 
{
    
}

void Bee::stateMachine()
{
    
}

void Bee::matchAnimation()
{
    
}

void Bee::move(float delta, const std::vector<SDL_Rect>& solids)
{
    
}

void Bee::addFrames(SDL_Renderer* ren)
{
    dIdle.load(ren, ENEMY_PATH("Bee/D_Walk.png"));
    dIdle.setFrameTime(0.0f);
    dIdle.addFrame({14, 21, 20, 14});

    uIdle.load(ren, ENEMY_PATH("Bee/U_Walk.png"));
    uIdle.setFrameTime(0.0f);
    uIdle.addFrame({14, 21, 20, 14});

    sIdle.load(ren, ENEMY_PATH("Bee/S_Walk.png"));
    sIdle.setFrameTime(0.0f);
    sIdle.addFrame({16, 15, 28, 21});

    dWalk.load(ren, ENEMY_PATH("Bee/D_Walk.png"));
    dWalk.setFrameTime(0.07f);
    dWalk.addFrame({14, 21, 20, 14});
    dWalk.addFrame({63, 19, 18, 15});
    dWalk.addFrame({109, 20, 22, 14});
    dWalk.addFrame({159, 17, 18, 17});
    dWalk.addFrame({206, 20, 20, 15});
    dWalk.addFrame({253, 21, 22, 14});

    uWalk.load(ren, ENEMY_PATH("Bee/U_Walk.png"));
    uWalk.setFrameTime(0.07f);
    uWalk.addFrame({14, 21, 20, 14});
    uWalk.addFrame({63, 19, 18, 15});
    uWalk.addFrame({109, 20, 22, 14});
    uWalk.addFrame({159, 17, 18, 17});
    uWalk.addFrame({206, 20, 20, 15});
    uWalk.addFrame({253, 21, 22, 14});

    sWalk.load(ren, ENEMY_PATH("Bee/S_Walk.png"));
    sWalk.setFrameTime(0.07f);
    sWalk.addFrame({16, 15, 18, 21});
    sWalk.addFrame({64, 18, 17, 18});
    sWalk.addFrame({112, 14, 18, 21});
    sWalk.addFrame({160, 16, 17, 19});
    sWalk.addFrame({208, 15, 18, 20});
    sWalk.addFrame({256, 18, 17, 18});

    dDeath.load(ren, ENEMY_PATH("Bee/D_Death.png"));
    dDeath.setLooping(false);
    dDeath.setFrameTime(0.07f);
    dDeath.addFrame({14, 20, 20, 15});
    dDeath.addFrame({63, 19, 18, 16});
    dDeath.addFrame({112, 18, 16, 17});
    dDeath.addFrame({159, 16, 18, 21});
    dDeath.addFrame({206, 14, 21, 24});
    dDeath.addFrame({253, 12, 23, 28});
    dDeath.addFrame({0, 0, 4, 4});

    uDeath.load(ren, ENEMY_PATH("Bee/U_Death.png"));
    uDeath.setLooping(false);
    uDeath.setFrameTime(0.07f);
    uDeath.addFrame({14, 21, 20, 14});
    uDeath.addFrame({63, 20, 18, 15});
    uDeath.addFrame({112, 18, 16, 18});
    uDeath.addFrame({160, 14, 16, 24});
    uDeath.addFrame({205, 13, 19, 27});
    uDeath.addFrame({252, 12, 21, 29});
    uDeath.addFrame({0, 0, 4, 4});

    sDeath.load(ren, ENEMY_PATH("Bee/S_Death.png"));
    sDeath.setLooping(false);
    sDeath.setFrameTime(0.07f);
    sDeath.addFrame({16, 18, 16, 18});
    sDeath.addFrame({64, 16, 17, 21});
    sDeath.addFrame({111, 15, 19, 23});
    sDeath.addFrame({155, 14, 25, 23});
    sDeath.addFrame({201, 13, 28, 26});
    sDeath.addFrame({248, 15, 30, 25});
    sDeath.addFrame({0, 0, 4, 4});
}
