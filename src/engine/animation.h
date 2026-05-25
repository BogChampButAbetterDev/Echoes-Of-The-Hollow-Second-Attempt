#pragma once
#include <SDL2/SDL.h>
#include <vector>

#include "texture.h"

class Animation
{
public:
    Animation() : m_tex(nullptr), m_frameTime(0.1f), m_currentFrame(0), m_timer(0.0f) {}

    void load(SDL_Renderer* ren, const char* path);
    void update(float delta);
    SDL_Rect getCurrentFrame();
    SDL_Texture* getTexture() { return m_tex; }
    void setFrameTime(float frameTime) { m_frameTime = frameTime; }
    void addFrame(SDL_Rect src);

private:
    SDL_Texture* m_tex;
    std::vector<SDL_Rect> m_frames;
    float m_frameTime;
    int m_currentFrame;
    float m_timer;
};
