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
    attackUp = other.attackUp;
    attackDown = other.attackDown;
    attackLeft = other.attackLeft;
    attackRight = other.attackRight;
    ssUp = other.ssUp;
    ssDown = other.ssDown;
    ssLeft = other.ssLeft;
    ssRight = other.ssRight;

    if (other.currentAnim == &other.idleUp)         currentAnim = &idleUp;
    else if (other.currentAnim == &other.idleDown)  currentAnim = &idleDown;
    else if (other.currentAnim == &other.idleLeft)  currentAnim = &idleLeft;
    else if (other.currentAnim == &other.idleRight) currentAnim = &idleRight;
    else if (other.currentAnim == &other.walkUp)    currentAnim = &walkUp;
    else if (other.currentAnim == &other.walkDown)  currentAnim = &walkDown;
    else if (other.currentAnim == &other.walkLeft)  currentAnim = &walkLeft;
    else if (other.currentAnim == &other.walkRight) currentAnim = &walkRight;
    else if (other.currentAnim == &other.attackUp)  currentAnim = &attackUp;
    else if (other.currentAnim == &other.attackDown)currentAnim = &attackDown;
    else if (other.currentAnim == &other.attackLeft)currentAnim = &attackLeft;
    else if (other.currentAnim == &other.attackRight)currentAnim = &attackRight;
    else if (other.currentAnim == &other.ssUp)  currentAnim = &ssUp;
    else if (other.currentAnim == &other.ssDown)currentAnim = &ssDown;
    else if (other.currentAnim == &other.ssLeft)currentAnim = &ssLeft;
    else if (other.currentAnim == &other.ssRight)currentAnim = &ssRight;
    else currentAnim = nullptr;

    return *this;
}

Player::Player(SDL_Renderer* ren, v2 pos)
: x(pos.x), y(pos.y),
  tex(Texture().loadTex(ren, PLAYER_PATH("player.png"))),
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
    idleUp.load(ren, PLAYER_PATH("player.png"));
    idleUp.setFrameTime(0.0f);
    idleUp.addFrame({5, 28, 14, 16});

    idleDown.load(ren, PLAYER_PATH("player.png"));
    idleDown.setFrameTime(0.0f);
    idleDown.addFrame({5, 4, 14, 16});

    idleLeft.load(ren, PLAYER_PATH("player.png"));
    idleLeft.setFrameTime(0.0f);
    idleLeft.addFrame({7, 76, 10, 16});

    idleRight.load(ren, PLAYER_PATH("player.png"));
    idleRight.setFrameTime(0.0f);
    idleRight.addFrame({7, 52, 10, 16});

    walkDown.load(ren, PLAYER_PATH("player.png"));
    walkDown.setFrameTime(0.07f);
    walkDown.addFrame({5, 4, 14, 16});
    walkDown.addFrame({30, 4, 13, 17});
    walkDown.addFrame({55, 5, 11, 17});
    walkDown.addFrame({79, 6, 11, 16});
    walkDown.addFrame({103, 5, 11, 17});
    walkDown.addFrame({126, 4, 13, 17});
    walkDown.addFrame({149, 4, 14, 16});
    walkDown.addFrame({173, 4, 13, 17});
    walkDown.addFrame({198, 5, 11, 17});
    walkDown.addFrame({222, 6, 11, 17});
    walkDown.addFrame({246, 5, 11, 17});
    walkDown.addFrame({269, 4, 13, 17});

    walkLeft.load(ren, PLAYER_PATH("player.png"));
    walkLeft.setFrameTime(0.07f);
    walkLeft.addFrame({5, 76, 14, 16});
    walkLeft.addFrame({30, 76, 13, 17});
    walkLeft.addFrame({55, 76, 11, 17});
    walkLeft.addFrame({79, 76, 11, 16});
    walkLeft.addFrame({103, 76, 11, 17});
    walkLeft.addFrame({126, 76, 13, 17});
    walkLeft.addFrame({149, 76, 14, 16});
    walkLeft.addFrame({173, 76, 13, 17});
    walkLeft.addFrame({198, 76, 11, 17});
    walkLeft.addFrame({222, 76, 11, 17});
    walkLeft.addFrame({246, 76, 11, 17});
    walkLeft.addFrame({269, 76, 13, 17});

    walkRight.load(ren, PLAYER_PATH("player.png"));
    walkRight.setFrameTime(0.07f);
    walkRight.addFrame({5, 52, 14, 16});
    walkRight.addFrame({30, 52, 13, 17});
    walkRight.addFrame({55, 52, 11, 17});
    walkRight.addFrame({79, 52, 11, 16});
    walkRight.addFrame({103, 52, 11, 17});
    walkRight.addFrame({126, 52, 13, 17});
    walkRight.addFrame({149, 52, 14, 16});
    walkRight.addFrame({173, 52, 13, 17});
    walkRight.addFrame({198, 52, 11, 17});
    walkRight.addFrame({222, 52, 11, 17});
    walkRight.addFrame({246, 52, 11, 17});
    walkRight.addFrame({269, 52, 13, 17});

    walkUp.load(ren, PLAYER_PATH("player.png"));
    walkUp.setFrameTime(0.07f);
    walkUp.addFrame({5, 28, 14, 16});
    walkUp.addFrame({30, 28, 13, 17});
    walkUp.addFrame({55, 28, 11, 17});
    walkUp.addFrame({79, 28, 11, 16});
    walkUp.addFrame({103, 28, 11, 17});
    walkUp.addFrame({126, 28, 13, 17});
    walkUp.addFrame({149, 28, 14, 16});
    walkUp.addFrame({173, 28, 13, 17});
    walkUp.addFrame({198, 28, 11, 17});
    walkUp.addFrame({222, 28, 11, 17});
    walkUp.addFrame({246, 28, 11, 17});
    walkUp.addFrame({269, 28, 13, 17});

    attackUp.load(ren, PLAYER_PATH("player.png"));
    attackUp.setFrameTime(0.07f);
    attackUp.addFrame({5, 100, 14, 16});
    attackUp.addFrame({30, 100, 13, 16});
    attackUp.addFrame({55, 101, 11, 15});
    attackUp.addFrame({79, 101, 10, 17});
    attackUp.addFrame({102, 100, 11, 17});

    attackDown.load(ren, PLAYER_PATH("player.png"));
    attackDown.setFrameTime(0.07f);
    attackDown.addFrame({5, 124, 14, 16});
    attackDown.addFrame({30, 124, 13, 16});
    attackDown.addFrame({54, 125, 13, 17});
    attackDown.addFrame({79, 126, 12, 15});
    attackDown.addFrame({103, 125, 13, 16});

    attackLeft.load(ren, PLAYER_PATH("player.png"));
    attackLeft.setFrameTime(0.07f);
    attackLeft.addFrame({7, 148, 10, 16});
    attackLeft.addFrame({31, 148, 10, 16});
    attackLeft.addFrame({56, 148, 10, 16});
    attackLeft.addFrame({79, 148, 11, 16});
    attackLeft.addFrame({102, 148, 12, 15});

    ssUp.load(ren, PLAYER_PATH("player.png"));
    ssUp.setFrameTime(0.07f);
    ssUp.addFrame({8, 222, 11, 11});
    ssUp.addFrame({31, 219, 13, 13});
    ssUp.addFrame({54, 220, 11, 13});
    ssUp.addFrame({77, 221, 12, 12});
    ssUp.addFrame({101, 222, 11, 11});

    ssDown.load(ren, PLAYER_PATH("player.png"));
    ssDown.setFrameTime(0.07f);
    ssDown.addFrame({8, 199, 11, 11});
    ssDown.addFrame({31, 199, 13, 13});
    ssDown.addFrame({54, 199, 11, 13});
    ssDown.addFrame({77, 199, 12, 12});
    ssDown.addFrame({101, 199, 11, 11});

    ssLeft.load(ren, PLAYER_PATH("player.png"));
    ssLeft.setFrameTime(0.07f);
    ssLeft.addFrame({8, 272, 11, 11});
    ssLeft.addFrame({31, 272, 13, 13});
    ssLeft.addFrame({54, 272, 11, 13});
    ssLeft.addFrame({77, 272, 12, 12});
    ssLeft.addFrame({101, 272, 11, 11});

    ssRight.load(ren, PLAYER_PATH("player.png"));
    ssRight.setFrameTime(0.07f);
    ssRight.addFrame({8, 248, 11, 11});
    ssRight.addFrame({31, 248, 13, 13});
    ssRight.addFrame({54, 248, 11, 13});
    ssRight.addFrame({77, 248, 12, 12});
    ssRight.addFrame({101, 248, 11, 11});
}
