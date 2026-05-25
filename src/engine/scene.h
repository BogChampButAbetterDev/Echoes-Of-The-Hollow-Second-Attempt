#pragma once
#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <SDL2/SDL.h>
#include "globals/globals.h"
#include "globals/Actor.h"
#include "engine/tilemap.h"

struct Scene
{
    std::string id;
    TileMap map;
    std::set<std::string> triggeredInteractables;
    std::unordered_map<std::string, SpawnPoint> spawnPoints;

    Scene() = default;
    Scene(const std::string& mapId, SDL_Renderer* ren)
    : id(mapId), map((std::string(ASSET_PATH) + "maps/" + mapId).c_str(), ren) {}
};
