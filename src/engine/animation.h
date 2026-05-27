#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>

#include "texture.h"

class Animation
{
public:
    Animation() : m_loops(true), m_tex(nullptr), m_frameTime(0.1f), m_currentFrame(0), m_timer(0.0f) {}

    void load(SDL_Renderer* ren, const char* path);
    void update(float delta);
    SDL_Rect getCurrentFrame();
    SDL_Texture* getTexture() { return m_tex; }
    void setFrameTime(float frameTime) { m_frameTime = frameTime; }
    void addFrame(SDL_Rect src);

    inline int getFrameCount() {return m_frames.size();}
    inline void reset() {m_currentFrame = 0; m_timer = 0;}
    inline bool isFinished() const {return m_frameTime > 0.0f && m_currentFrame == (int)m_frames.size() - 1;}

    bool m_loops;
    inline void setLooping(bool loops) {m_loops = loops;}

    inline void syncFrame(int frame) { m_currentFrame = std::clamp(frame, 0, (int)m_frames.size()-1); }
    inline int getCurrentFrameIndex() const { return m_currentFrame; }

private:
    SDL_Texture* m_tex;
    std::vector<SDL_Rect> m_frames;
    float m_frameTime;
    int m_currentFrame;
    float m_timer;
};
