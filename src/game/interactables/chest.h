#pragma once
#include <engine/interactable.h>

struct Chest : public Interactable
{
    bool opened = false;
    int openedGid = 0; // opened sprite

    Chest(int tx, int ty, const std::string& text, int openedGid)
    {
        tileX = tx; tileY = ty;
        type = InteractType::CHEST;
        data = text;
        this->openedGid = openedGid;
    }

    bool interact() override
    {
        if (opened)
        {
            data = "This chest is empty. | Stop staring into it like something new will appear in it.";
            return false;
        }
        opened = true;
        if (onInteract) onInteract();
        return false;
    }
};
