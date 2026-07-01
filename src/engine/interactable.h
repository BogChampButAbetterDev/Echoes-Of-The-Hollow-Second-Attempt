#pragma once
#include <string>
#include <functional>

enum class InteractType
{
    SIGN,
    LOADZONE,
    CHEST,
    BUTTON,
    NPC,
    ITEM
};

struct Interactable
{
    int tileX, tileY;
    InteractType type;
    std::string data;
    std::string name;
    std::string targetName;
    bool activateOnContact = false;

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    std::function<void()> onInteract;

    virtual ~Interactable() = default;

    // return true if interaction callback handled
    // return false if game should handle it.
    virtual bool interact()
    {
        if (onInteract)
        {
            onInteract();
            return true;
        }
        return false;
    }
};
