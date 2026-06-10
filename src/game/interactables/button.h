#pragma once
#include <cstdint>
#include "engine/interactable.h"

struct Button : public Interactable
{
    bool pressed = false;
    bool toggleable = false; // can be pressed many times
    int upGid = 0;
    int downGid= 0;
    std::vector<uint32_t>* objectTiles = nullptr;
    std::string layerName = "ground";
    int mapW = 0;

    Button(int tx, int ty)
    {
        tileX = tx; tileY = ty;
        type = InteractType::BUTTON;
        activateOnContact = true;
    }

    bool interact() override
    {
        if (pressed && !toggleable) return true; // already pressed, do nothing
        pressed = !pressed;

        if (objectTiles)
        {
            size_t index = tileY * mapW + tileX;
            if (index >= 0 && index < objectTiles->size())
                (*objectTiles)[index] = pressed ? downGid : upGid;
        }

        if (onInteract) onInteract();
        return true; // game should not give buttons dialogue
    }
};
