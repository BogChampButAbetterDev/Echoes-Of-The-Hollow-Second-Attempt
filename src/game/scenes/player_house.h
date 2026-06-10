#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig pHouseConfig()
{
    SceneConfig cfg;
    return cfg;
}

inline const SceneConfig& _pHouseConfig()
{
    static const SceneConfig cfg = pHouseConfig();
    return cfg;
}
