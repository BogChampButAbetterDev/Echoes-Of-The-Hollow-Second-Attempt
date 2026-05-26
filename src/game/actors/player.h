#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "engine/texture.h"
#include "engine/input.h"
#include "renderer/ren_queue.h"
#include "globals/globals.h"
#include "globals/Actor.h"
#include "engine/animation.h"
#include "engine/camera.h"

enum class DIR_STATES
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Player
{
public:
    Input input;

    Player(const Player& other)
    : x(other.x), y(other.y), tex(other.tex), m_src(other.m_src),
    moving(other.moving), stat(other.stat),
    idleUp(other.idleUp), idleDown(other.idleDown),
    idleLeft(other.idleLeft), idleRight(other.idleRight),
    walkUp(other.walkUp), walkDown(other.walkDown),
    walkLeft(other.walkLeft), walkRight(other.walkRight),
    attackUp(other.attackUp), attackDown(other.attackDown),
    attackLeft(other.attackLeft), attackRight(other.attackRight),
    ssUp(other.ssUp), ssDown(other.ssDown),
    ssLeft(other.ssLeft), ssRight(other.ssRight),
    input(other.input)
    {
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
        else if (other.currentAnim == &other.ssLeft)currentAnim = &ssLeft;
        else if (other.currentAnim == &other.ssRight)currentAnim = &ssRight;
        else currentAnim = nullptr;
    }
    Player() : x(0), y(0), tex(nullptr), m_src{0,0,0,0},
           moving(false), stat(DIR_STATES::DOWN),
           currentAnim(nullptr) {}
    Player& operator=(const Player& other); 
    Player(SDL_Renderer* ren, v2 pos);

    void update(float delta, Camera& cam, const std::vector<SDL_Rect>& solids);
    void queueForRender(Camera& cam);

    void setPosition(int nx, int ny) {x = nx; y = ny;}

    float getX() { return x; }
    float getY() { return y; }

    DIR_STATES getStat() { return stat; }

private:
    SDL_Texture* tex;
    SDL_Rect m_src;
    float x, y;
    bool moving;
    DIR_STATES stat; 

    void move(float delta, const std::vector<SDL_Rect>& solids);
    void matchAnimation();
    void addFrames(SDL_Renderer* ren);

    Animation idleUp;
    Animation idleDown;
    Animation idleLeft;
    Animation idleRight;
    Animation walkUp;
    Animation walkDown;
    Animation walkLeft;
    Animation walkRight;
    Animation attackUp;
    Animation attackDown;
    Animation attackLeft;
    Animation attackRight;
    Animation ssUp; // sword swing : ss
    Animation ssDown;
    Animation ssLeft;
    Animation ssRight;
    Animation* currentAnim;  // pointer to whichever is active
};
