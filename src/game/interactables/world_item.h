#pragma once
#include "engine/interactable.h"
#include "engine/texture.h"
#include "engine/camera.h"
#include "renderer/ren_queue.h"
#include "globals/globals.h"

struct WorldItem : public Interactable
{
    std::string id; // name of the item
    SDL_Rect rect = {0, 0, 0, 0};
    SDL_Texture* tex = nullptr;
    bool collected = false;

    // in world space
    float wx = 0.0f;
    float wy = 0.0f;

    WorldItem(float wrlX, float wrlY, const std::string& itmId, SDL_Rect r, SDL_Texture* texture)
    {
        wx = wrlX; wy = wrlY;
        id = itmId;
        tex = texture;
        type = InteractType::ITEM;
        activateOnContact = true;
        // unused for items, set world coords directly
        tileX = 0; tileY = 0;
        this->rect = r;
        x = r.x;
        y = r.y;
        w = r.w;
        h = r.h;
    }

    // return false to allow game to handle setting the story flag, 
    // showing collection dialogue
    bool interact() override
    {
        if (collected) return true;
        collected = true;
        if (onInteract) onInteract();
        return false;
    }

    void queueForRender(Camera& cam)
    {
        if (collected || !tex) return;

        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);

        int screenX = cam.toScrX(wx * MAP_RENDER_SCALE);
        int screenY = cam.toScrY(wy * MAP_RENDER_SCALE);

        submit(tex, {0, 0, w, h},
               screenX, screenY,
               w * SPRITE_RENDER_SCALE,
               h * SPRITE_RENDER_SCALE);
    }
};
