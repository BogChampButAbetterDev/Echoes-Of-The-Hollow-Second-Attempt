#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig TownConfig()
{
    SceneConfig cfg;
    cfg.areaName = "Castle Town";
    return cfg;
}

inline const SceneConfig& _TownConfig()
{
    static const SceneConfig cfg = TownConfig();
    return cfg;
}
