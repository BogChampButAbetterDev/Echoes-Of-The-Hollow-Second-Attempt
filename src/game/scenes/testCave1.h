#pragma once
#include "globals/Actor.h"   // EnemyDef, SceneConfig

inline SceneConfig testCaveConfig()
{
    SceneConfig cfg;
    // just a loading zone test. no enemies
    return cfg;
}

inline const SceneConfig& TestCaveConfig()
{
    static const SceneConfig cfg = testCaveConfig();
    return cfg;
}
