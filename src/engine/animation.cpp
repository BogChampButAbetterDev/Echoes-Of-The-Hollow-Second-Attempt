#include "animation.h"

void Animation::load(SDL_Renderer* ren, const char* path)
{
    m_tex = Texture::loadTex(ren, path);
}

void Animation::update(float delta) {
    if (m_frameTime == 0.0f || m_frames.empty()) return;
    if (!m_loops && m_currentFrame >= (int)m_frames.size() - 1) return;

    m_timer += delta;
    if (m_timer >= m_frameTime) {
        m_timer = 0.0f;
        m_currentFrame++;
        
        if (m_currentFrame >= (int)m_frames.size()) {
            if (m_loops) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = (int)m_frames.size() - 1; // Hold last frame safely
            }
        }
    }
}


SDL_Rect Animation::getCurrentFrame()
{
    if (m_frames.empty()) return {0, 0, 0, 0};
    return m_frames[m_currentFrame];
}

void Animation::addFrame(SDL_Rect src)
{
    m_frames.push_back(src);
}
