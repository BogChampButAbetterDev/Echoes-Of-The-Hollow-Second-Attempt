#pragma once
#include <unordered_map>
#include <functional>
#include "testmap.h"
#include "testCave1.h"

class AllScenes
{
public:
    AllScenes()
    {
        reg["testworld.tmx"] = TestMapConfig();
        reg["test_cave.tmx"] = TestCaveConfig();
    }
 
    // Returns the config for a map, or an empty config if unregistered.
    // An empty config means no enemies will spawn — safe fallback.
    const SceneConfig& getConfig(const std::string& mapId) const
    {
        static const SceneConfig empty{};
        auto it = reg.find(mapId);
        return it != reg.end() ? it->second : empty;
    }
 
private:
    std::unordered_map<std::string, SceneConfig> reg;
};
