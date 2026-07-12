#include "bee.h"

Bee::Bee(SDL_Renderer* ren, v2 pos) 
{
    x = pos.x; y = pos.y;
}

void Bee::init(SDL_Renderer* ren)
{
    flipH = false;
    health = 30;
    actionState = ACTION_IDLE;
    stateFlags = STATE_NONE;
    stat = DIR_STATE::LEFT;
    addFrames(ren);
    currentAnim = &sIdle;
    m_src = currentAnim->getCurrentFrame();
}

void Bee::update(float delta, const std::vector<SDL_Rect>& solids, float px, float py, SDL_Rect mapBounds, Player& player)
{
    if (stateFlags & STATE_DEATH) 
    {
        m_deathTimer -= delta;
        matchAnimation();
        m_src = currentAnim->getCurrentFrame();
        currentAnim->update(delta);
        return;
    }

    m_iframeTimer -= delta;
    m_isRedTimer -= delta;
    if (m_iframeTimer < 0.0f) m_iframeTimer = 0.0f;
    if (m_isRedTimer < 0.0f) m_isRedTimer = 0.0f;

    m_src = currentAnim->getCurrentFrame();
    requestTexTint();

    stateMachine(px, py, player);
    move(delta, solids, mapBounds, px, py);

    m_src = currentAnim->getCurrentFrame();
    matchAnimation();
    currentAnim->update(delta);
}

void Bee::queueForRender(Camera& cam)
{
    submit(currentAnim->getTexture(), m_src, 
       cam.toScrX(x * MAP_RENDER_SCALE), 
       cam.toScrY(y * MAP_RENDER_SCALE), 
       m_src.w * SPRITE_RENDER_SCALE, 
       m_src.h * SPRITE_RENDER_SCALE,
       flipH,
       m_tintR, m_tintG, m_tintB);
}

void Bee::move(float delta, const std::vector<SDL_Rect>& solids, SDL_Rect mapBounds, float px, float py)
{
    float speed = 20.0f * MAP_RENDER_SCALE;

    if (m_knockbackTimer > 0.0f)
    {
        dx = m_knockbackX * delta;
        dy = m_knockbackY * delta;

        m_knockbackTimer -= delta;
        if (m_knockbackTimer <= 0.0f)
            stateFlags &= ~STATE_PAIN;
    }
    else if (actionState == ACTION_PATROL)
    {
        m_patrolTimer -= delta;
        if (m_patrolTimer <= 0.0f)
        {
            float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
            m_patrolDX = std::cos(angle);
            m_patrolDY = std::sin(angle);
            m_patrolTimer = 0.5f + ((float)rand() / RAND_MAX) * 1.5f;
        }
        dx = m_patrolDX * speed * delta;
        dy = m_patrolDY * speed * delta;
    }
    else if (actionState == ACTION_CHASE)
    {
        m_patrolTimer = 0.0f;
        float diffX = m_targetX - x;
        float diffY = m_targetY - y;
        float dist  = std::sqrt(diffX * diffX + diffY * diffY);
        if (dist > 0.0f)
        {
            dx = (diffX / dist) * speed * delta;
            dy = (diffY / dist) * speed * delta;
        }
    }

    if (std::abs(dx) >= std::abs(dy))
        stat = (dx > 0) ? DIR_STATE::RIGHT : DIR_STATE::LEFT;
    else
        stat = (dy > 0) ? DIR_STATE::DOWN : DIR_STATE::UP;

    float oldX = x;
    x += dx;
    SDL_Rect rx = getHitbox();
    for (const SDL_Rect& tile : solids)
        if (SDL_HasIntersection(&rx, &tile)) { x = oldX; m_patrolDX = -m_patrolDX; break; }

    float oldY = y;
    y += dy;
    SDL_Rect ry = getHitbox();
    for (const SDL_Rect& tile : solids)
        if (SDL_HasIntersection(&ry, &tile)) { y = oldY; m_patrolDY = -m_patrolDY; break; }

    float maxX = (mapBounds.w / MAP_RENDER_SCALE) - m_src.w;
    float maxY = (mapBounds.h / MAP_RENDER_SCALE) - m_src.h;
    x = std::max(0.0f, std::min(x, maxX));
    y = std::max(0.0f, std::min(y, maxY));
}

void Bee::requestTexTint()
{
    if (m_isRedTimer != 0.0f) { m_tintR = 255; m_tintG = 0; m_tintB = 0; }
    else { m_tintR = 255; m_tintG = 255; m_tintB = 255; }
}

SDL_Rect Bee::getHitbox()
{
    return {
        (int)(x * MAP_RENDER_SCALE),
        (int)(y * MAP_RENDER_SCALE),
        m_src.w * SPRITE_RENDER_SCALE,
        m_src.h * SPRITE_RENDER_SCALE
    };
}

void Bee::onDamage(float amount)
{
    if (m_iframeTimer > 0.0f) return;
    health -= amount;
    stateFlags = STATE_PAIN;
    m_iframeTimer = 0.5f;
    m_knockbackX = -50.0f;
    m_knockbackY = 0.0f;
    m_knockbackTimer = 0.15f;

    if (health <= 0)
        onDeath();
}

void Bee::onDeath() 
{
    stateFlags = STATE_DEATH;
    actionState = ACTION_NONE;
    // death animation is 7 frames with each frame lasting 0.07 seconds
    m_deathTimer = 7 * 0.07f;
}

bool Bee::canDeleteEntity()
{
    return stateFlags & STATE_DEATH && m_deathTimer <= 0.0f;
}

void Bee::stateMachine(float px, float py, Player& player)
{
    // sword hit check
    SDL_Rect sword = player.getSwordHitbox();
    if (sword.w > 0 && sword.h > 0) // only check if actually attacking
    {
        SDL_Rect self = getHitbox();
        if (SDL_HasIntersection(&sword, &self))
        {
            onDamage(player.current_damage_amount);
        }
    }

    float dx = px - x;
    float dy = py - y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < AGGRO_RAD)
    {
        actionState = ACTION_CHASE;
        m_targetX = px;
        m_targetY = py;
    }
    else
    {
        actionState = ACTION_PATROL;
    }
}

void Bee::matchAnimation()
{
    Animation* next = nullptr;
    bool nextFlip = false;

    if (actionState & ACTION_NONE) return;

    if (actionState & ACTION_PATROL || actionState & ACTION_CHASE)
    {
        switch (stat)
        {
            case DIR_STATE::UP:    next = &uWalk; nextFlip = false; break;
            case DIR_STATE::DOWN:  next = &dWalk; nextFlip = false; break;
            case DIR_STATE::LEFT:  next = &sWalk; nextFlip = false; break;
            case DIR_STATE::RIGHT: next = &sWalk; nextFlip = true;  break;
        }
    }
    else if (actionState & ACTION_IDLE)
    {
        switch (stat)
        {
            case DIR_STATE::UP:    next = &uIdle; nextFlip = false; break;
            case DIR_STATE::DOWN:  next = &dIdle; nextFlip = false; break;
            case DIR_STATE::LEFT:  next = &sIdle; nextFlip = false; break;
            case DIR_STATE::RIGHT: next = &sIdle; nextFlip = true;  break;
        }
    }

    if (stateFlags & STATE_PAIN)
    {
        m_isRedTimer = 0.5f; // sprite is red while invincible
        requestTexTint();
    }
    else if (stateFlags & STATE_DEATH)
    {
        switch (stat)
        {
            case DIR_STATE::UP:    next = &uDeath; nextFlip = false; break;
            case DIR_STATE::DOWN:  next = &dDeath; nextFlip = false; break;
            case DIR_STATE::LEFT:  next = &sDeath; nextFlip = false; break;
            case DIR_STATE::RIGHT: next = &sDeath; nextFlip = true;  break;
        }
    }

    // only switch if actually changing
    if (next && next != currentAnim)
    {
        currentAnim = next;
        currentAnim->reset();
    }
    flipH = nextFlip;
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
