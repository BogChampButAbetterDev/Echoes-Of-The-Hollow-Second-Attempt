#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig swordCaveConfig()
{
    SceneConfig cfg;
    cfg.enemyDefs.push_back({"bee", 5.0f, false});
    cfg.areaName = "";

    cfg.onEnter = [](StoryState& story, EnemySpawner& spawner)
    {
        // I will just pass in the coords of the spawner in the scene 
        //  x="157.25" y="224.5"
        spawner.registerRoomBurst("sword_cave", "bee", 157.25f, 224.5f, 5.0f, 5);
        spawner.disable("bee_sp");
    };

    cfg.onUpdate = [](StoryState& story, EnemySpawner& spawner, const std::vector<SDL_Rect>& solids)
    {   
        if (story.has("has_sword"))
        {
            if (!story.has("cleared_sword_cave"))
            {
                spawner.triggerRoomBurst("sword_cave", solids);
                if (spawner.isRoomCleared("sword_cave"))
                {
                    story.set("cleared_sword_cave");
                }
            }
        }
    };
    return cfg;
}

inline const SceneConfig& _swordCaveConfig()
{
    static const SceneConfig cfg = swordCaveConfig();
    return cfg;
}
