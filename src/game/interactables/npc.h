#include "engine/interactable.h"
#include "engine/story_state.h"

struct DialogueLine
{
    std::string flag;
    std::string text;
};  

struct NPC : Interactable
{
    // the first flag the player has or doesn't have will be shown
    std::vector<DialogueLine> lines;
    bool isImportant;

    NPC(int tx, int ty)
    {
        tileX = tx; tileY = ty;
        type = InteractType::NPC;
    }

    // when interacting, this will get called to get the story state
    std::string getText(const StoryState& stat) const
    {
        for (const auto& line : lines)
        {
            if (line.flag.empty() || stat.has(line.flag))
                return line.text;
        }

        return "";
    }

    // interact will return false so game opens a dialogue box
    bool interact() override {return false;}
};
