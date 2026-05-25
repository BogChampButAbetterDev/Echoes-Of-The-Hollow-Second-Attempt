#include "player.h"

Player& Player::operator=(const Player& other)
{
    if (this == &other) return *this;
    x = other.x; y = other.y;
    tex = other.tex;
    m_src = other.m_src;
    moving = other.moving;
    stat = other.stat;
    input = other.input;

    idleUp = other.idleUp;
    idleDown = other.idleDown;
    idleLeft = other.idleLeft;
    idleRight = other.idleRight;
    walkUp = other.walkUp; 
    walkDown = other.walkDown;
    walkLeft = other.walkLeft; 
    walkRight = other.walkRight;

    if (other.currentAnim == &other.idleUp)         currentAnim = &idleUp;
    else if (other.currentAnim == &other.idleDown)  currentAnim = &idleDown;
    else if (other.currentAnim == &other.idleLeft)  currentAnim = &idleLeft;
    else if (other.currentAnim == &other.idleRight) currentAnim = &idleRight;
    else if (other.currentAnim == &other.walkUp)    currentAnim = &walkUp;
    else if (other.currentAnim == &other.walkDown)  currentAnim = &walkDown;
    else if (other.currentAnim == &other.walkLeft)  currentAnim = &walkLeft;
    else if (other.currentAnim == &other.walkRight) currentAnim = &walkRight;
    else currentAnim = nullptr;

    return *this;
}

Player::Player(SDL_Renderer* ren, v2 pos)
: x(pos.x), y(pos.y),
  tex(Texture().loadTex(ren, PLAYER_PATH("idle.png"))),
  stat(DIR_STATES::DOWN),
  currentAnim(nullptr)
{

    addFrames(ren);
    currentAnim = &idleDown;
}

void Player::update(float delta, Camera& cam, const std::vector<SDL_Rect>& solids)
{
    move(delta, solids);
    matchAnimation();
    currentAnim->update(delta);
    m_src = currentAnim->getCurrentFrame();
}

void Player::queueForRender(Camera &cam)
{
    submit(currentAnim->getTexture(), m_src, 
       cam.toScrX(x * MAP_RENDER_SCALE), 
       cam.toScrY(y * MAP_RENDER_SCALE), 
       m_src.w * SPRITE_RENDER_SCALE, 
       m_src.h * SPRITE_RENDER_SCALE);
}

void Player::move(float delta, const std::vector<SDL_Rect>& solids)
{
    float speed = 30.0f * MAP_RENDER_SCALE;
    float dx = 0, dy = 0;

    if      (input.w) { dy = -speed * delta; moving = true; stat = DIR_STATES::UP; }
    else if (input.s) { dy =  speed * delta; moving = true; stat = DIR_STATES::DOWN; }
    else if (input.a) { dx = -speed * delta; moving = true; stat = DIR_STATES::LEFT; }
    else if (input.d) { dx =  speed * delta; moving = true; stat = DIR_STATES::RIGHT; }
    else              { moving = false; }

    int pw = m_src.w * SPRITE_RENDER_SCALE;
    int ph = m_src.h * SPRITE_RENDER_SCALE;
    const int footHeight = 15;
    const int footOffsetY = ph - footHeight;
    const int shrink = 4;

    auto getFootRect = [&]() -> SDL_Rect {
        return {
            (int)(x * MAP_RENDER_SCALE) + shrink,
            (int)(y * MAP_RENDER_SCALE) + footOffsetY,
            pw - shrink * 2,
            footHeight
        };
    };

    // resolve X
    float oldX = x;
    x += dx;
    SDL_Rect rx = getFootRect();
    for (const SDL_Rect& tile : solids)
    {
        if (SDL_HasIntersection(&rx, &tile))
        {
            int overlapY = std::min(rx.y + rx.h, tile.y + tile.h) - std::max(rx.y, tile.y);
            if (overlapY > COLLISION_SLIDE_THRESHOLD)
            {
                x = oldX;
                break;
            }
        }
    }

    // resolve Y
    float oldY = y;
    y += dy;
    SDL_Rect ry = getFootRect();
    for (const SDL_Rect& tile : solids)
    {
        if (SDL_HasIntersection(&ry, &tile))
        {
            int overlapX = std::min(ry.x + ry.w, tile.x + tile.w) - std::max(ry.x, tile.x);
            if (overlapX > COLLISION_SLIDE_THRESHOLD)
            {
                y = oldY;
                break;
            }
        }
    }
}

void Player::matchAnimation()
{
    if (moving)
    {
        switch (stat)
        {
        case DIR_STATES::UP:
            currentAnim = &walkUp;
            break;

        case DIR_STATES::DOWN:
            currentAnim = &walkDown;
            break;
        
        case DIR_STATES::LEFT:
            currentAnim = &walkLeft;
            break;
        
        case DIR_STATES::RIGHT:
            currentAnim = &walkRight;
            break;
        
        default:
            currentAnim = &walkDown;
            std::cout << "You're a dumbass, stat is bad. " << (int)stat << "\n";
            break;
        }
    }
    else
    {
        switch (stat)
        {
        case DIR_STATES::UP:
            currentAnim = &idleUp;
            break;

        case DIR_STATES::DOWN:
            currentAnim = &idleDown;
            break;
        
        case DIR_STATES::LEFT:
            currentAnim = &idleLeft;
            break;
        
        case DIR_STATES::RIGHT:
            currentAnim = &idleRight;
            break;
        
        default:
            currentAnim = &idleDown;
            std::cout << "You're a dumbass, stat is bad. " << (int)stat << "\n";
            break;
        }
    }
}

void Player::addFrames(SDL_Renderer* ren)
{
    idleUp.load(ren, PLAYER_PATH("idle.png"));
    idleUp.setFrameTime(0.0f);
    idleUp.addFrame({24, 212, 19, 27});

    idleDown.load(ren, PLAYER_PATH("idle.png"));
    idleDown.setFrameTime(0.0f);
    idleDown.addFrame({22, 22, 18, 26});

    idleLeft.load(ren, PLAYER_PATH("idle.png"));
    idleLeft.setFrameTime(0.0f);
    idleLeft.addFrame({22, 83, 17, 28});

    idleRight.load(ren, PLAYER_PATH("idle.png"));
    idleRight.setFrameTime(0.0f);
    idleRight.addFrame({24, 147, 17, 28});

    walkDown.load(ren, PLAYER_PATH("walk.png"));
    walkDown.setFrameTime(0.1f);
    walkDown.addFrame({20, 21, 19, 26});
    walkDown.addFrame({84, 19, 19, 28});
    walkDown.addFrame({148, 19, 19, 28});
    walkDown.addFrame({213, 21, 18, 26});
    walkDown.addFrame({278, 19, 17, 28});
    walkDown.addFrame({342, 20, 17, 27});

    walkLeft.load(ren, PLAYER_PATH("walk.png"));
    walkLeft.setFrameTime(0.1f);
    walkLeft.addFrame({20, 81, 19, 26});
    walkLeft.addFrame({84, 81, 19, 28});
    walkLeft.addFrame({148, 81, 19, 28});
    walkLeft.addFrame({213, 81, 18, 26});
    walkLeft.addFrame({278, 81, 17, 28});
    walkLeft.addFrame({342, 81, 17, 27});

    walkRight.load(ren, PLAYER_PATH("walk.png"));
    walkRight.setFrameTime(0.1f);
    walkRight.addFrame({20, 144, 19, 26});
    walkRight.addFrame({84, 144, 19, 28});
    walkRight.addFrame({148, 144, 19, 28});
    walkRight.addFrame({213, 144, 18, 26});
    walkRight.addFrame({278, 144, 17, 28});
    walkRight.addFrame({342, 144, 17, 27});

    walkUp.load(ren, PLAYER_PATH("walk.png"));
    walkUp.setFrameTime(0.1f);
    walkUp.addFrame({20, 211, 19, 26});
    walkUp.addFrame({84, 211, 19, 28});
    walkUp.addFrame({148, 211, 19, 28});
    walkUp.addFrame({213, 211, 18, 26});
    walkUp.addFrame({278, 211, 17, 28});
    walkUp.addFrame({342, 211, 17, 27});
}
