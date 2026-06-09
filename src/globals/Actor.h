#pragma once
#include <string>
#include <cstdint>

#define COLLISION_SLIDE_THRESHOLD 5

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
};

struct EnemyDef
{
    std::string type = "";
    float respawnDelay = 5.0f;
    bool  unique = false;
};
