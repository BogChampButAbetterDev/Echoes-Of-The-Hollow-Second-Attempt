#pragma once
#include <string>

#define COLLISION_SLIDE_THRESHOLD 5

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
