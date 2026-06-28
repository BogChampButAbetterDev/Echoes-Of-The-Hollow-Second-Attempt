#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig pHouseConfig()
{
    SceneConfig cfg;
    cfg.areaName = "Your House";
    return cfg;
}

inline const SceneConfig& _pHouseConfig()
{
    static const SceneConfig cfg = pHouseConfig();
    return cfg;
}
