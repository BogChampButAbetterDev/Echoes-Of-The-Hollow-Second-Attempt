#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

#define COLLISION_SLIDE_THRESHOLD 5

struct StoryState;
class EnemySpawner;

using u8 = uint8_t;

typedef struct Vector2
{
    float x;
    float y;
} v2;

typedef struct integerVector2
{
    int x, y;
} v2i;

struct SpawnPoint
{
    std::string name = "";
    float x = 0.0f, y = 0.0f;
};

struct EnemySpawnPoint
{
    std::string id = "";
    std::string enemyType = "";
    float x = 0.0f;
    float y = 0.0f;
    int maxCount = 1;
    std::string roomId = "";
};

struct EnemyDef
{
    std::string type = "";
    float respawnDelay = 5.0f;
    bool  unique = false;
};

struct SceneConfig
{
    std::vector<EnemyDef> enemyDefs;
    std::string areaName;
    std::function<void(StoryState&, EnemySpawner& spawner)> onEnter;
    std::function<void(StoryState&, EnemySpawner& spawner, const std::vector<SDL_Rect>& solids)> onUpdate;
};
