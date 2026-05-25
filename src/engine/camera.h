#pragma once
#include <cmath>
#include "globals/globals.h"
#include "globals/Actor.h"

struct Camera
{
    float x, y;
    int w, h;
    int mapW, mapH;

    // cutscene state
    bool inCutscene = false;

    Camera() : x(0), y(0), w(0), h(0), mapW(0), mapH(0) {}

    Camera(int scrW, int scrH, int mapWidth, int mapHeight)
    : x(0), y(0), w(scrW), h(scrH), mapW(mapWidth), mapH(mapHeight) {}

    // trigger a pan to a position, then return back to original position
    void panTo(float worldX, float worldY, float holdTime = 1.5f, float speed = 200.0f)
    {
        m_panTargetX = std::max(0.0f, std::min(worldX - w / 2, (float)(mapW - w)));
        m_panTargetY = std::max(0.0f, std::min(worldY - h / 2, (float)(mapH - h)));
        m_holdTime    = holdTime;
        m_holdTimer   = 0.0f;
        m_panSpeed    = speed;
        m_returnX     = x;
        m_returnY     = y;
        inCutscene    = true;
        m_phase       = Phase::PANNING_TO;
    }

    void update(float delta, float playerX, float playerY)
    {
        if (!inCutscene)
        {
            follow(playerX, playerY);
            return;
        }

        switch (m_phase)
        {
            case Phase::PANNING_TO:
            {
                if (moveToward(m_panTargetX, m_panTargetY, delta))
                {
                    m_phase = Phase::HOLDING;
                    m_holdTimer = 0.0f;
                }
                break;
            }

            case Phase::HOLDING:
            {
                m_holdTimer += delta;
                // keep return target updated to current player pos
                m_returnX = playerX - w / 2;
                m_returnY = playerY - h / w;
                if (m_holdTimer >= m_holdTime)
                    m_phase = Phase::RETURNING;
                break;
            }

            case Phase::RETURNING:
            {
                // snap back to player 
                x = m_returnX;
                y = m_returnY;
                inCutscene = false;
                follow(playerX, playerY);
                break;
            }
        }

        clamp();
    }

    void follow(float targetX, float targetY)
    {
        float deadzoneX = 40.0f;
        float deadzoneY = 35.0f;

        float screenTargetX = targetX - x;
        float screenTargetY = targetY - y;

        if (screenTargetX < w / 2 - deadzoneX) x = targetX - (w / 2 - deadzoneX);
        if (screenTargetX > w / 2 + deadzoneX) x = targetX - (w / 2 + deadzoneX);
        if (screenTargetY < h / 2 - deadzoneY) y = targetY - (h / 2 - deadzoneY);
        if (screenTargetY > h / 2 + deadzoneY) y = targetY - (h / 2 + deadzoneY);

        clamp();
    }

    int toScrX(float wrlX) { return (int)(wrlX - x); }
    int toScrY(float wrlY) { return (int)(wrlY - y); }

private:
    enum class Phase { PANNING_TO, HOLDING, RETURNING };
    Phase m_phase = Phase::PANNING_TO;

    float m_panTargetX = 0, m_panTargetY = 0;
    float m_returnX    = 0, m_returnY    = 0;
    float m_holdTime   = 1.5f;
    float m_holdTimer  = 0.0f;
    float m_panSpeed   = 200.0f;

    // moves camera toward target, returns true when arrived
    bool moveToward(float targetX, float targetY, float delta)
    {
        float dx    = targetX - x;
        float dy    = targetY - y;
        float dist  = std::sqrt(dx * dx + dy * dy);
        float step  = m_panSpeed * delta;

        if (dist <= step || dist < 2.0f) // 2.0 arrival threshold
        {
            x = targetX;
            y = targetY;
            return true;
        }

        x += (dx / dist) * step;
        y += (dy / dist) * step;
        return false;
    }

    void clamp()
    {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + w > mapW) x = mapW - w;
        if (y + h > mapH) y = mapH - h;
    }
};
