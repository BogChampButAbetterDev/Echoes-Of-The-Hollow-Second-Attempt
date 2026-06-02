#include "player.h"

// Hand offsets (in sprite pixels, pre-scale) per direction per frame
// Format: {sword_dx, sword_dy} from the player's top-left corner
static const v2i swordOffsets[4][5] = {
    // UP    (5 frames)
    {{5, -2}, {5, -4}, {4, -6}, {3, -4}, {5, -1}},
    // DOWN
    {{-1, 10}, {-1, 12},  {-2, 12},  {-3, 10},  {-1, 10}},
    // LEFT
    {{-7, 4},{-9, 4}, {-9, 3}, {-7, 3},  {-7, 4}},
    // RIGHT
    {{5, 7}, {7, 7},  {7, 6},  {5, 6},   {5, 7}},
};

Player& Player::operator=(const Player& other)
{
    if (this == &other) return *this;
    x = other.x; y = other.y;
    tex = other.tex;
    m_src = other.m_src;
    moving = other.moving;
    stat = other.stat;
    input = other.input;
    m_attacking = other.m_attacking;
    m_attackTimer = other.m_attackTimer;

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
    else {currentAnim = nullptr; m_swordAnim = nullptr;}

    return *this;
}

Player::Player(SDL_Renderer* ren, v2 pos)
: x(pos.x), y(pos.y),
  tex(Texture().loadTex(ren, PLAYER_PATH("player.png"))),
  stat(DIR_STATES::DOWN),
  currentAnim(nullptr), m_swordAnim(nullptr)
{

    addFrames(ren);
    currentAnim = &idleDown;
}

void Player::update(float delta, Camera& cam, const std::vector<SDL_Rect>& solids)
{
    if (!m_attacking)
        move(delta, solids);

    if (input.attack && !m_attacking)
    {
        m_attacking = true;
        attackUp.reset();   attackDown.reset();
        attackLeft.reset(); attackRight.reset();
        ssUp.reset();       ssDown.reset();
        ssLeft.reset();     ssRight.reset();
    }

    if (m_attacking)
    {
        Animation* bodyAnim  = nullptr; // actual swing motion
        Animation* swordAnim = nullptr; // sword moving
        switch (stat)
        {
            case DIR_STATES::UP:     bodyAnim = &attackUp;    swordAnim = &ssUp;     break;
            case DIR_STATES::DOWN:   bodyAnim = &attackDown;  swordAnim = &ssDown;   break;
            case DIR_STATES::LEFT:   bodyAnim = &attackLeft;  swordAnim = &ssLeft;   break;
            case DIR_STATES::RIGHT:  bodyAnim = &attackRight; swordAnim = &ssRight;  break;
        }

        // run both swing animations at the same time by using the same timer
        bodyAnim->update(delta);
        swordAnim->syncFrame(bodyAnim->getCurrentFrameIndex());

        currentAnim = bodyAnim;
        m_swordAnim = swordAnim;
        m_src       = currentAnim->getCurrentFrame();

        if (bodyAnim->isFinished())
            m_attacking = false;
        
        return;  // skip second update if slash occurs
    }

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
    
    if (m_attacking && m_swordAnim)
    {
        SDL_Rect ssrc = m_swordAnim->getCurrentFrame();
        int dirIdx = (int)stat; // UP=0 DOWN=1 LEFT=2 RIGHT=3
        int frameIdx = std::min(currentAnim->getCurrentFrameIndex(), 4);
        v2i off = swordOffsets[dirIdx][frameIdx];

        // add sword sprite to renderer queue
        submit(m_swordAnim->getTexture(), ssrc,
            cam.toScrX(x * MAP_RENDER_SCALE) + off.x * SPRITE_RENDER_SCALE,
            cam.toScrY(y * MAP_RENDER_SCALE) + off.y * SPRITE_RENDER_SCALE,
            ssrc.w * SPRITE_RENDER_SCALE,
            ssrc.h * SPRITE_RENDER_SCALE);
    }
}

SDL_Rect Player::getSwordHitbox()
{
    if (!m_attacking) return {0, 0, 0, 0};

    int px = (int)(x * MAP_RENDER_SCALE);
    int py = (int)(y * MAP_RENDER_SCALE);
    int reach = 12 * SPRITE_RENDER_SCALE; // how far the sword can reach

    switch (stat)
    {
        case DIR_STATES::UP:    return {px,          py - reach, m_src.w * SPRITE_RENDER_SCALE, reach};
        case DIR_STATES::DOWN:  return {px,          py + m_src.h * SPRITE_RENDER_SCALE, m_src.w * SPRITE_RENDER_SCALE, reach};
        case DIR_STATES::LEFT:  return {px - reach,  py, reach, m_src.h * SPRITE_RENDER_SCALE};
        case DIR_STATES::RIGHT: return {px + m_src.w * SPRITE_RENDER_SCALE, py, reach, m_src.h * SPRITE_RENDER_SCALE};
        default: return {0, 0, 0, 0}; 
    }
}

void Player::move(float delta, const std::vector<SDL_Rect>& solids)
{
    float speed = 30.0f * MAP_RENDER_SCALE;
    float dx = 0, dy = 0;

    // --- analog stick: free 8-dir movement ---
    if (std::abs(input.axisX) > 0.0f || std::abs(input.axisY) > 0.0f)
    {
        dx = input.axisX * speed * delta;
        dy = input.axisY * speed * delta;
        moving = true;

        // facing: pick whichever axis is dominant
        if (std::abs(input.axisX) >= std::abs(input.axisY))
            stat = (input.axisX > 0) ? DIR_STATES::RIGHT : DIR_STATES::LEFT;
        else
            stat = (input.axisY > 0) ? DIR_STATES::DOWN  : DIR_STATES::UP;
    }
    // --- digital fallback (keyboard or d-pad) ---
    else if (input.w || input.a || input.s || input.d)
    {
        if (input.w) { dy = -speed * delta; stat = DIR_STATES::UP; }
        if (input.s) { dy =  speed * delta; stat = DIR_STATES::DOWN; }
        if (input.a) { dx = -speed * delta; stat = DIR_STATES::LEFT; }
        if (input.d) { dx =  speed * delta; stat = DIR_STATES::RIGHT; }
        moving = true;

        // normalize diagonals
        if (dx != 0 && dy != 0)
        {
            dx *= 0.7071f;
            dy *= 0.7071f;
        }
    }
    else
    {
        moving = false;
    }

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
    if (m_attacking) return;
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
    walkLeft.setFrameTime(0.04f);
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
    walkRight.setFrameTime(0.04f);
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

    attackUp.setLooping(false);
    attackDown.setLooping(false);
    attackLeft.setLooping(false);
    attackRight.setLooping(false);
    ssUp.setLooping(false);
    ssDown.setLooping(false);
    ssLeft.setLooping(false);
    ssRight.setLooping(false);

    attackUp.load(ren, PLAYER_PATH("player.png"));
    attackUp.setFrameTime(0.04f);
    attackUp.addFrame({5, 100, 14, 16});
    attackUp.addFrame({30, 100, 13, 16});
    attackUp.addFrame({55, 101, 11, 15});
    attackUp.addFrame({79, 101, 10, 17});
    attackUp.addFrame({102, 100, 11, 17});

    attackDown.load(ren, PLAYER_PATH("player.png"));
    attackDown.setFrameTime(0.04f);
    attackDown.addFrame({5, 124, 14, 16});
    attackDown.addFrame({30, 124, 13, 16});
    attackDown.addFrame({54, 125, 13, 17});
    attackDown.addFrame({79, 126, 12, 15});
    attackDown.addFrame({103, 125, 13, 16});

    attackLeft.load(ren, PLAYER_PATH("player.png"));
    attackLeft.setFrameTime(0.04f);
    attackLeft.addFrame({7, 172, 10, 16});
    attackLeft.addFrame({31, 172, 10, 16});
    attackLeft.addFrame({56, 172, 10, 16});
    attackLeft.addFrame({79, 172, 11, 16});
    attackLeft.addFrame({102, 172, 12, 15});

    attackRight.load(ren, PLAYER_PATH("player.png"));
    attackRight.setFrameTime(0.04f);
    attackRight.addFrame({7, 148, 10, 16});
    attackRight.addFrame({31, 148, 10, 16});
    attackRight.addFrame({56, 148, 10, 16});
    attackRight.addFrame({79, 148, 11, 16});
    attackRight.addFrame({102, 148, 12, 15});

    ssUp.load(ren, PLAYER_PATH("player.png"));
    ssUp.setFrameTime(0.07f);
    ssUp.addFrame({8, 222, 11, 11});
    ssUp.addFrame({31, 219, 13, 13});
    ssUp.addFrame({54, 220, 11, 13});
    ssUp.addFrame({77, 221, 11, 12});
    ssUp.addFrame({101, 222, 11, 11});

    ssDown.load(ren, PLAYER_PATH("player.png"));
    ssDown.setFrameTime(0.07f);
    ssDown.addFrame({5, 199, 11, 11});
    ssDown.addFrame({28, 200, 13, 13});
    ssDown.addFrame({55, 199, 11, 13});
    ssDown.addFrame({80, 199, 11, 12});
    ssDown.addFrame({104, 199, 11, 11});

    ssLeft.load(ren, PLAYER_PATH("player.png"));
    ssLeft.setFrameTime(0.07f);
    ssLeft.addFrame({6, 272, 11, 11});
    ssLeft.addFrame({27, 271, 13, 13});
    ssLeft.addFrame({52, 270, 13, 11});
    ssLeft.addFrame({77, 269, 12, 11});
    ssLeft.addFrame({102, 269, 11, 11});

    ssRight.load(ren, PLAYER_PATH("player.png"));
    ssRight.setFrameTime(0.07f);
    ssRight.addFrame({7, 248, 11, 11});
    ssRight.addFrame({32, 247, 13, 13});
    ssRight.addFrame({55, 246, 13, 11});
    ssRight.addFrame({79, 245, 12, 11});
    ssRight.addFrame({103, 245, 11, 11});
}
