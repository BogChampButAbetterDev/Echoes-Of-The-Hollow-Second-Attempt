#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <sstream>
#include <SDL2/SDL.h>
#include "tinyxml2.h"
#include "texture.h"
#include "camera.h"
#include "renderer/ren_queue.h"
#include "globals/globals.h"
#include "globals/Actor.h"
#include "interactable.h"
#include "InteractableIncludes.h"

using namespace tinyxml2;

struct TileLayer
{
    std::vector<uint32_t> tiles;
    std::string name;
};

struct TilesetInfo
{
    SDL_Texture* tex;
    int firstGid;
    int columns;
    int tileCount;
    std::string name;
};

class TileMap
{
public:
    TileMap() : m_mapWidth(0), m_mapHeight(0), m_tileWidth(0),
                m_tileHeight(0),
                m_firstGid(1), m_columns(0), m_tileCount(0) {}
    TileMap(const char* path, SDL_Renderer* ren);

    void renderGround(SDL_Renderer* ren, Camera& cam);
    void renderObjects(SDL_Renderer* ren, Camera& cam);
    void renderOverhead(SDL_Renderer* ren, Camera& cam);

    void setTile(const std::string& layerName, int tx, int ty, int gid);
    int getFirstGid(const std::string& tilesetName) const;
    int pixelToGid(const std::string& tilesetName, int pixelX, int pixelY) const;

    const std::vector<SDL_Rect>& getSolidRects() const { return m_solidRects; }
    const std::vector<std::unique_ptr<Interactable>>& getInteractables() const { return m_interactables; }

    const std::unordered_map<std::string, SpawnPoint>& getSpawnPoints() const { return m_spawnPoints; }

    const std::vector<EnemySpawnPoint>& getEnemySpawnPoints() const { return m_enemySpawnPoints; }

    int m_mapWidth, m_mapHeight;
    int m_tileWidth, m_tileHeight;

private:
    void loadTMX(const char* path, SDL_Renderer* ren);
    void renderLayer(SDL_Renderer* ren, Camera& cam, TileLayer& layer);
    void renderLayerToQueue(Camera& cam, TileLayer& layer);
    const TilesetInfo* getTilesetForGid(int gid) const;

    std::vector<TilesetInfo> m_tilesets;
    std::vector<TileLayer> m_layers;
    std::vector<SDL_Rect> m_solidRects;
    std::vector<std::unique_ptr<Interactable>> m_interactables;

    std::unordered_map<std::string, SpawnPoint> m_spawnPoints;
    std::vector<EnemySpawnPoint> m_enemySpawnPoints;

    int m_firstGid;
    int m_columns;
    int m_tileCount;
};
