#pragma once
#include "engine/interactable.h"

struct Door : public Interactable
{
    bool open = false;
    int openedGid = 0;
    int closedGid = 0;

    std::string toMap;
    std::string toSpawn;

    // flag that opens the door
    std::string storyFlag;

    bool isTransitionDoor() const {return !toMap.empty();}

    Door(int tx, int ty, int openedGid, int closedGid)
    {
        tileX = tx; tileY = ty;
        type = InteractType::LOADZONE;
        this->openedGid = openedGid;
        this->closedGid = closedGid;
        activateOnContact = true;
    }

    void setOpen(bool state, std::vector<uint32_t>* tiles, int mapW)
    {
        open = state;
        if (!tiles) return;
        size_t index = tileY * mapW + tileX;
        if(index >= 0 && index < tiles->size())
            (*tiles)[index] = open ? openedGid : closedGid;
    }

    bool interact() override 
    { 
        if (onInteract) onInteract();
        return true;
    }
};
