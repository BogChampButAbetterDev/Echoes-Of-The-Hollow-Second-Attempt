#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig testMapConfig()
{
    SceneConfig cfg;
    cfg.areaName = "Test";
    cfg.enemyDefs.push_back({"bee", 5.0f, false});
    return cfg;
}

inline const SceneConfig& TestMapConfig()
{
    static const SceneConfig cfg = testMapConfig();
    return cfg;
}
