#pragma once
#include "engine/interactable.h"

struct Sign : public Interactable
{
    Sign(int tx, int ty, const std::string& text)
    {
        tileX = tx; tileY = ty;
        type = InteractType::SIGN;
        data = text;
    }

    // sign is only dialogue. Interact() should let game handle dialogue box
    bool interact() override {return false;}
};
