#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <SDL2/SDL.h>
#include "globals/globals.h"
#include "globals/Actor.h"
#include "engine/tilemap.h"
#include "engine/enemy_spawner.h"

struct Scene
{
    std::string id;
    TileMap     map;
    std::set<std::string>                      triggeredInteractables;
    std::unordered_map<std::string, SpawnPoint> spawnPoints; // player spawn points (unchanged)
    EnemySpawner                               spawner;

    Scene() = default;

    // Pass a SceneConfig so the spawner knows per-type tuning (respawn delay,
    // unique flag). The config comes from the scene's header file (testmap.h etc.)
    Scene(const std::string& mapId, SDL_Renderer* ren, const SceneConfig& config)
    : id(mapId), 
    map((std::string(ASSET_PATH) + "maps/" + mapId).c_str(), ren)
    {
        spawner.init(map.getEnemySpawnPoints(), config);
    }
};