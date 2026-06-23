#include "ui_cursor.h"

void Cursor::update(float dt)
{
    move(dt);

    if (input.interact)
    {
        if (currentAnim != &click)
        {
            click.reset();
            currentAnim = &click;
        }
    }
    if (currentAnim == &click && click.isFinished())
    {
        currentAnim = &def;
    } 

    currentAnim->update(dt);
    m_rect = currentAnim->getCurrentFrame();
}

void Cursor::queueForRender()
{
    submit(currentAnim->getTexture(), m_rect, (int)x, (int)y,
        m_rect.w * SPRITE_RENDER_SCALE, m_rect.h * SPRITE_RENDER_SCALE);
}

void Cursor::move(float dt) 
{
    float speed = 200.0f * MAP_RENDER_SCALE;
    
    float dx = input.axisX;
    float dy = input.axisY;
    
    // Normalize diagonal input vector so diagonal speed isn't too fast
    if (dx != 0.f && dy != 0.f) {
        dx *= 0.7071f; // 1 / sqrt(2)
        dy *= 0.7071f;
    }
    
    x += dx * speed * dt;
    y += dy * speed * dt;
    
    x = std::clamp<float>(x, 0.f, SCREEN_WIDTH);
    y = std::clamp<float>(y, 0.f, SCREEN_HEIGHT);
}

void Cursor::addFrames(SDL_Renderer* ren)
{
    def.load(ren, PLAYER_PATH("player.png"));
    def.setFrameTime(0.07f);
    def.addFrame({8, 222, 11, 11});

    click.load(ren, PLAYER_PATH("player.png"));
    click.setLooping(false);
    click.setFrameTime(0.07f);
    click.addFrame({8, 222, 11, 11});
    click.addFrame({31, 219, 13, 13});
    click.addFrame({54, 220, 11, 13});
    click.addFrame({77, 221, 11, 12});
    click.addFrame({101, 222, 11, 11});
}
