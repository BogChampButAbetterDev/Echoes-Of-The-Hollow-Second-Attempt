#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig swordCaveConfig()
{
    SceneConfig cfg;
    // just a loading zone test. no enemies
    cfg.areaName = "";
    return cfg;
}

inline const SceneConfig& _swordCaveConfig()
{
    static const SceneConfig cfg = swordCaveConfig();
    return cfg;
}
